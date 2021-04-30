#include <unistd.h>
#include <sys/mman.h>

#include <cassert>
#include <cstring>
#include <climits>
#include <string>
#include <string_view>
#include <atomic>

inline constexpr uint64_t pageSize = 4096;

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

template<typename T,
    typename = std::enable_if_t<std::is_integral_v<T>>,
    typename = std::enable_if_t<std::is_unsigned_v<T>>>
constexpr T roundUpToNextPowerOf2(T value, uint64_t maxb = sizeof(T) * CHAR_BIT, uint64_t curb = 1)
{
    return maxb <= curb
               ? value
               : roundUpToNextPowerOf2(((value - 1) | ((value - 1) >> curb)) + 1, maxb, curb << 1);
}

template<uint64_t sizeClass>
class MemoryQueue
{
    static_assert(sizeClass >= pageSize, "Size class must be greater or equal to OS page size");

public:
    MemoryQueue() noexcept
    {
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

    Header *head{nullptr};
    Header *tail{nullptr};
};
