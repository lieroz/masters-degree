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

constexpr bool isDivisibleByPageSize(uint64_t value)
{
    return value % pageSize == 0;
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

template<uint64_t sizeClass>
class MemoryQueue
{
    static_assert(sizeClass >= pageSize && isDivisibleByPageSize(sizeClass),
        "Size class must be greater or equal to OS page size and be divisible on page size");

public:
    MemoryQueue() noexcept {}

    void push(void *dataPointer) noexcept
    {
        assert(dataPointer != nullptr && isDivisibleByPageSize(dataPointer));
        *(static_cast<uint64_t *>(dataPointer) - 1) = 0;

        LockGuard guard{lock_};

        if (tail == nullptr)
        {
            tail = dataPointer;
            head = tail;
        }
        else
        {
            *(static_cast<uint64_t *>(tail) - 1) = reinterpret_cast<uint64_t>(dataPointer);
            tail = dataPointer;
        }
    }

    void pop(void *&dataPointer) noexcept
    {
        LockGuard guard{lock_};

        if (head == nullptr)
        {
            guard.unlock();

            uint64_t *mmapedAddress = static_cast<uint64_t *>(mmapImpl(pageSize + sizeClass));
            *mmapedAddress = sizeClass;
            dataPointer = static_cast<void *>(mmapedAddress + pageSize / sizeof(uint64_t));
        }
        else
        {
            dataPointer = head;
            if (uint64_t next = *(static_cast<uint64_t *>(head) - 1); next == 0)
            {
                head = nullptr;
                tail = nullptr;
            }
            else
            {
                head = reinterpret_cast<void *>(next);
            }
        }
    }

private:
    SpinLock lock_;

    void *head{nullptr};
    void *tail{nullptr};
};

template<uint16_t sizeClass>
class SmallMemoryRegistry
{
    static_assert(sizeClass < pageSize, "Size class must be less than OS page size");

public:
private:
};
