#include <unistd.h>
#include <sys/mman.h>

#include <cassert>
#include <cstring>
#include <climits>
#include <string>
#include <string_view>
#include <atomic>

inline constexpr uint64_t pageSize = 1 << 12;  // 4KB

inline constexpr uint64_t registryRangeBegin = 1 << 12;  // 4KB
inline constexpr uint64_t registryRangeEnd = 1 << 21;    // 2MB

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
    LockGuard(Lock &lock) : lock_(lock)
    {
        lock_.lock();
    }

    ~LockGuard()
    {
        lock_.unlock();
    }

private:
    Lock &lock_;
};

std::string systemError(std::string_view message)
{
    return std::string(message) + " failed : " + std::strerror(errno);
}

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

template<uint64_t sizeClass>
class MemoryRegistry
{
    static_assert(sizeClass >= pageSize && isPowerOf2(sizeClass),
        "Size class must be greater or equal to OS page size and be power of 2");

public:
    MemoryRegistry() noexcept
        : metadataPages(static_cast<uintptr_t *>(mmapImpl(pageSize))),
          currentMetadataPage(metadataPages),
          currentFreeCell(metadataPages),
          metadataLimit(pageSize / sizeof(uintptr_t))
    {
    }

    void push(void *dataPointer) noexcept
    {
        assert(dataPointer != nullptr);
        LockGuard guard{lock};

        if (currentFreeCell + 1 == currentMetadataPage + metadataLimit)
        {
            currentFreeCell = static_cast<uintptr_t *>(mmapImpl(pageSize));
            currentMetadataPage[metadataLimit - 1] = reinterpret_cast<uintptr_t>(currentFreeCell);
            *currentFreeCell = reinterpret_cast<uintptr_t>(currentMetadataPage);
            currentMetadataPage = ++currentFreeCell;
        }

        *(currentFreeCell++) = reinterpret_cast<uintptr_t>(dataPointer);
    }

    void pop(void *&dataPointer) noexcept
    {
        LockGuard guard{lock};

        if (currentFreeCell == metadataPages)
        {
            dataPointer = mmapImpl(sizeClass);
        }
        else
        {
            dataPointer = static_cast<void *>(--currentFreeCell);
            if (currentFreeCell - 1 == currentMetadataPage && currentFreeCell != metadataPages)
            {
                uintptr_t *previousMetadataPage =
                    reinterpret_cast<uintptr_t *>(*currentMetadataPage);
                currentFreeCell = previousMetadataPage + metadataLimit - 1;
                munmapImpl(currentMetadataPage, pageSize);
                currentMetadataPage = previousMetadataPage;
            }
        }
    }

private:
    void *mmapImpl(uint64_t size)
    {
        void *mmapedAddress =
            mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        assert(mmapedAddress != MAP_FAILED && systemError("mmap").c_str());
        return mmapedAddress;
    }

    void munmapImpl(uintptr_t *mmapedAddress, uint64_t size)
    {
        assert(munmap(static_cast<void *>(mmapedAddress), size) != -1 &&
               systemError("munmap").c_str());
    }

private:
    SpinLock lock;

    uintptr_t *metadataPages;
    uintptr_t *currentMetadataPage;
    uintptr_t *currentFreeCell;
    size_t metadataLimit;
};
