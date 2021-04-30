#include <unistd.h>
#include <sys/mman.h>

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <atomic>

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

uint64_t roundUpToNextPowerOf2(uint64_t value)
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;
    return value;
}

template<typename T>
class MemoryQueue
{
public:
    MemoryQueue() noexcept : pageSize(getpagesize())
    {
        sizeClass = roundUpToNextPowerOf2(sizeof(T));

        head = static_cast<Header *>(
            mmap(nullptr, pageSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        assert(head != MAP_FAILED && systemError("mmap").c_str());

        tail = head + pageSize / sizeof(Header) - 1;
    }

private:
    // Header size must be of power of 2
    // to be equally divisible on system page size
    // and can't be more than page size
    struct Header
    {
    };

private:
    SpinLock lock;

    uint64_t pageSize;
    uint64_t sizeClass;

    Header *head{nullptr};
    Header *tail{nullptr};
};
