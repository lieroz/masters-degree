#include <unistd.h>
#include <sys/mman.h>

#include <cassert>
#include <cstring>
#include <climits>
#include <string>
#include <string_view>
#include <atomic>

// ATTENTION: don't change these values
// may break whole logic
inline constexpr uint64_t pageSize = 1 << 12;  // 4KB
inline constexpr uint64_t metadataLimit = pageSize / sizeof(uint64_t);  // 512 elements
inline constexpr uint64_t largeObjectSizeLimit = 1 << 20;               // 1MB

inline constexpr uint64_t smallObjectMask = static_cast<uint64_t>(1) << 63;
inline constexpr uint64_t highestVirtualSpaceBit = 48;  // without PAE in 64-bit mode

template<typename T>
T *getWorkingAddress(T *value)
{
    static constexpr uint64_t workingAddressMask =
        (static_cast<uint64_t>(1) << highestVirtualSpaceBit) - 1;  // 0xffffffffffff
    return reinterpret_cast<T *>(reinterpret_cast<uint64_t>(value) & workingAddressMask);
}

class SpinLock
{
public:
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

std::string systemError(std::string_view message)
{
    return std::string(message) + " failed : " + std::strerror(errno);
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
    assert(mmapedAddress != MAP_FAILED && systemError("mmap").c_str());
    return mmapedAddress;
}

void munmapImpl(void *mmapedAddress, uint64_t size)
{
    assert(munmap(mmapedAddress, size) != -1 && systemError("munmap").c_str());
}

template<uint64_t sizeClass>
class MemoryRegistry
{
    static_assert(sizeClass >= pageSize && isDivisibleBy<pageSize>(sizeClass));

public:
    MemoryRegistry() noexcept : metadata(static_cast<uint64_t *>(mmapImpl(pageSize)))
    {
        std::memset(metadata, 0, pageSize);
    }

    void push(void *dataPointer) noexcept
    {
        dataPointer = getWorkingAddress(dataPointer);
        LockGuard guard{lock_};

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
        uint64_t mmapedAddress;

        {
            LockGuard guard{lock_};

            if (popIndex == 0 && metadata[popIndex] == 0)
            {
                guard.unlock();
                mmapedAddress = reinterpret_cast<uint64_t>(mmapImpl(sizeClass));
            }
            else
            {
                pushIndex = popIndex;
                mmapedAddress = metadata[popIndex];
                metadata[popIndex] = 0;

                if (popIndex != 0)
                {
                    --popIndex;
                }
            }
        }

        uint64_t bitsSizeClass = (sizeClass / pageSize) << highestVirtualSpaceBit;
        dataPointer = reinterpret_cast<void *>(mmapedAddress | bitsSizeClass);
    }

private:
    SpinLock lock_;

    uint64_t *metadata;
    uint64_t pushIndex{0};
    uint64_t popIndex{0};
};
