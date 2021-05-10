#include <unistd.h>
#include <sys/mman.h>

#include <cassert>
#include <cstring>
#include <cerrno>
#include <climits>
#include <atomic>
#include <array>

// ATTENTION: don't change these values
// may break whole logic
inline constexpr uint64_t pageSize = 1 << 12;  // 4KB
inline constexpr uint64_t metadataLimit = pageSize / sizeof(uint64_t);  // 512 elements

inline constexpr uint64_t smallObjectsSizeStart = 1 << 3;   // 8
inline constexpr uint64_t smallObjectsSizeLimit = 1 << 11;  // 2KB

inline constexpr uint64_t largeObjectsSizeStart = 1 << 12;              // 4KB
inline constexpr uint64_t largeObjectsSizeLimit = 1 << 20;              // 1MB

inline constexpr uint64_t smallObjectMask = static_cast<uint64_t>(1) << 63;
inline constexpr uint64_t highestVirtualSpaceBit = 48;  // without PAE in 64-bit mode

// 0000000000000000 -> 00007fffffffffff => canonical low address space half
// 00007fffffffffff -> ffff800000000000 => illegal addresses, unused
// ffff800000000000 -> ffffffffffffffff => canonical high address space half
template<typename T>
T *getWorkingAddress(T *value)
{
    static constexpr uint64_t workingAddressMask =
        (static_cast<uint64_t>(1) << highestVirtualSpaceBit) - 1;  // 0xffffffffffff
    uint64_t address = reinterpret_cast<uint64_t>(value) & workingAddressMask;

    if ((reinterpret_cast<uint64_t>(value) &
            (static_cast<uint64_t>(1) << (highestVirtualSpaceBit - 1))) != 0)
    {
        static constexpr uint64_t highHalf = static_cast<uint64_t>(0x1ffff)  // 0b11111111111111111
                                             << (highestVirtualSpaceBit - 1);
        address |= highHalf;
    }

    return reinterpret_cast<T *>(address);
}

class SpinLock
{
public:
    SpinLock() noexcept = default;
    SpinLock(const SpinLock &other) : lock_(other.lock_.load()) {}
    SpinLock &operator=(const SpinLock &other)
    {
        lock_.store(other.lock_.load());
        return *this;
    }

    void lock() noexcept
    {
        for (;;)
        {
            if (!lock_.exchange(true, std::memory_order_acquire))
            {
                return;
            }
            while (lock_.load(std::memory_order_relaxed))
            {
                __builtin_ia32_pause();
            }
        }
    }

    bool tryLock() noexcept
    {
        return !lock_.load(std::memory_order_relaxed) &&
               !lock_.exchange(true, std::memory_order_acquire);
    }

    void unlock() noexcept
    {
        lock_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> lock_{false};
};

template<typename Lock>
class LockGuard
{
public:
    LockGuard(Lock &lock) noexcept : lock_(lock)
    {
        lock_.lock();
    }

    ~LockGuard() noexcept
    {
        if (locked)
        {
            lock_.unlock();
        }
    }

    void unlock() noexcept
    {
        locked = false;
        lock_.unlock();
    }

private:
    bool locked{true};
    Lock &lock_;
};

template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr T roundUpToNextPowerOf2(T value, uint64_t maxb = sizeof(T) * CHAR_BIT, uint64_t curb = 1)
{
    return maxb <= curb
               ? value
               : roundUpToNextPowerOf2(((value - 1) | ((value - 1) >> curb)) + 1, maxb, curb << 1);
}

template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr bool isPowerOf2(T value)
{
    if ((value & (value - 1)))
    {
        return false;
    }
    return true;
}

template<uint64_t size>
constexpr bool isDivisibleBy(uint64_t value)
{
    return value % size == 0;
}

constexpr uint64_t getNextNearestDivisibleByPageSize(uint64_t value)
{
    return ((value / pageSize) + 1) * pageSize;
}

void *mmapImpl(uint64_t size)
{
    void *mmapedAddress =
        mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(mmapedAddress != MAP_FAILED && std::strerror(errno));
    return mmapedAddress;
}

void munmapImpl(void *mmapedAddress, uint64_t size)
{
    assert(munmap(mmapedAddress, size) != -1 && std::strerror(errno));
}

// XXX: allocator for small objects
// which size is of power of 2 and up to 2048 bytes
class SmallObjectsRegistry
{
public:
    SmallObjectsRegistry() noexcept = default;
    SmallObjectsRegistry(uint64_t size) noexcept : sizeClass(size) {}

    ~SmallObjectsRegistry()
    {
        if (metadata != nullptr)
        {
            for (uint64_t cursor{0}; cursor < metadataLimit; ++cursor)
            {
                void *ptr =
                    reinterpret_cast<void *>(reinterpret_cast<uint64_t *>(metadata)[cursor]);
                if (ptr != nullptr)
                {
                    munmap(ptr, pageSize);
                }
            }
            munmap(metadata, pageSize);
        }
    }

    void push(void *dataPointer) noexcept
    {
        init();

        std::memset(dataPointer, 0, sizeClass);
    }

    bool pop(void *&dataPointer) noexcept
    {
        init();

        auto &&[cell, index] = findFreeCell(pageIndex);
        // XXX: no free objects
        // maybe add another metadata page here
        // for queue like structure?
        if (index == metadataLimit)
        {
            return false;
        }

        pageIndex = index;

        if (cell == nullptr)
        {
            dataPointer = mmapImpl(pageSize);
            metadata[pageIndex] = reinterpret_cast<uint64_t>(dataPointer);
        }
        else
        {
            dataPointer = cell;
        }

        std::memset(dataPointer, 0xff, sizeClass);
        return true;
    }

private:
    inline void init() noexcept
    {
        assert(sizeClass != 0);
        if (!initialized)
        {
            metadata = static_cast<uint64_t *>(mmapImpl(pageSize));
            std::memset(metadata, 0, pageSize);
            initialized = true;
        }
    }

    std::pair<void *, uint64_t> findFreeCell(uint64_t hint) noexcept
    {
        uint64_t limit{hint};
        for (; limit < metadataLimit && metadata[limit] != 0; ++limit)
        {
            if (void *cell = findFreeCellInPage(limit); cell != nullptr)
            {
                return {cell, limit};
            }
        }

        for (uint64_t index{0}; index < metadataLimit && metadata[index] != 0; ++index)
        {
            if (void *cell = findFreeCellInPage(index); cell != nullptr)
            {
                return {cell, index};
            }
        }

        return {nullptr, limit};
    }

    void *findFreeCellInPage(uint64_t index) noexcept
    {
        char null[sizeClass];
        std::memset(null, 0, sizeClass);

        char *begin = reinterpret_cast<char *>(metadata[index]);
        char *end = begin + pageSize;

        for (; begin != end; begin += sizeClass)
        {
            if (!std::memcmp(null, begin, sizeClass))
            {
                return begin;
            }
        }

        return nullptr;
    }

private:
    bool initialized{false};
    uint64_t sizeClass{0};

    uint64_t *metadata{nullptr};
    uint64_t pageIndex{0};
};

// XXX: can be up to 268431360 bytes to encode in 16 bits
// multiplied by page size
class LargeObjectsRegistry
{
public:
    LargeObjectsRegistry() noexcept = default;
    LargeObjectsRegistry(uint64_t size) noexcept : sizeClass(size)
    {
        assert(sizeClass >= pageSize && isDivisibleBy<pageSize>(sizeClass));
    }

    void push(void *dataPointer) noexcept
    {
        LockGuard guard{lock_};
        init();

        if (pushIndex != metadataLimit)
        {
            metadata[pushIndex] = reinterpret_cast<uint64_t>(dataPointer);
            popIndex = pushIndex++;
        }
        else
        {
            guard.unlock();
            munmapImpl(dataPointer, sizeClass);
        }
    }

    void pop(void *&dataPointer) noexcept
    {
        LockGuard guard{lock_};
        init();

        if (popIndex == 0 && metadata[popIndex] == 0)
        {
            guard.unlock();
            dataPointer = mmapImpl(sizeClass);
        }
        else
        {
            pushIndex = popIndex;
            dataPointer = reinterpret_cast<void *>(metadata[popIndex]);
            metadata[popIndex] = 0;

            if (popIndex != 0)
            {
                --popIndex;
            }
        }
    }

private:
    inline void init() noexcept
    {
        assert(sizeClass != 0);
        if (!initialized)
        {
            metadata = static_cast<uint64_t *>(mmapImpl(pageSize));
            std::memset(metadata, 0, pageSize);
            initialized = true;
        }
    }

private:
    bool initialized{false};
    uint64_t sizeClass{0};
    SpinLock lock_;

    uint64_t *metadata{nullptr};
    uint64_t pushIndex{0};
    uint64_t popIndex{0};
};

static auto createSmallObjectsRegistries()
{
    std::array<SmallObjectsRegistry, 9> registries;
    for (uint64_t index{0}, size{smallObjectsSizeStart}; size <= smallObjectsSizeLimit;
         ++index, size <<= 1)
    {
        registries[index] = SmallObjectsRegistry{size};
    }
    return registries;
}

static SmallObjectsRegistry &getSmallObjectsRegistry(size_t size)
{
    thread_local auto registries = createSmallObjectsRegistries();
    uint64_t index{0};
    for (; size > smallObjectsSizeStart; size >>= 1, ++index)
        ;
    return registries[index];
}

static auto createLargeObjectsRegistries()
{
    static constexpr uint64_t count = largeObjectsSizeLimit / largeObjectsSizeStart;
    std::array<LargeObjectsRegistry, count> registries;
    for (uint64_t index{0}, size{largeObjectsSizeStart}; index < count; ++index, size += pageSize)
    {
        registries[index] = std::move(LargeObjectsRegistry{size});
    }
    return registries;
}

static LargeObjectsRegistry &getLargeObjectsRegistry(size_t size)
{
    assert(isDivisibleBy<pageSize>(size));
    static auto registries = createLargeObjectsRegistries();
    return registries[size / pageSize - 1];
}

void *myMalloc(size_t size)
{
    void *ptr{nullptr};
    if (size <= smallObjectsSizeLimit)
    {
        size = roundUpToNextPowerOf2(size);
        auto &registry = getSmallObjectsRegistry(size);
        assert(registry.pop(ptr));

        uint64_t controlBits = (size << highestVirtualSpaceBit) | smallObjectMask;
        ptr = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(ptr) | controlBits);
    }
    else
    {
        size = getNextNearestDivisibleByPageSize(size);

        if (size <= largeObjectsSizeLimit)
        {
            auto &registry = getLargeObjectsRegistry(size);
            registry.pop(ptr);

            uint64_t controlBits = (size / pageSize) << highestVirtualSpaceBit;
            ptr = reinterpret_cast<void *>(reinterpret_cast<uint64_t>(ptr) | controlBits);
        }
        else
        {
            ptr = mmapImpl(pageSize + size);
            *static_cast<uint64_t *>(ptr) = size;
            ptr = static_cast<std::byte *>(ptr) + pageSize;
        }
    }
    return ptr;
}

void myFree(void *ptr)
{
    uint64_t address = reinterpret_cast<uint64_t>(ptr);
    size_t sizeClass = (address >> highestVirtualSpaceBit) & 0x7fff;

    ptr = getWorkingAddress(ptr);

    if (sizeClass <= smallObjectsSizeLimit)
    {
        auto &registry = getSmallObjectsRegistry(sizeClass);
        registry.push(ptr);
    }
    else if (sizeClass != 0)
    {
        auto &registry = getLargeObjectsRegistry(sizeClass * pageSize);
        registry.push(ptr);
    }
    else
    {
        ptr = static_cast<std::byte *>(ptr) - pageSize;
        munmapImpl(ptr, *static_cast<uint64_t *>(ptr));
    }
}
