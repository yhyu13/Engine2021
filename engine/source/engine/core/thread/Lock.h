#pragma once
#include "../EngineCore.h"
#include <atomic>

#define PADDING 0

#define ATOMIC_THREAD_FENCE std::atomic_thread_fence(std::memory_order_acq_rel)

namespace longmarch
{
    // used in stbi image parallel read/write
    extern std::atomic_flag stbi_hdr_write_lock;
    // used in stbi image parallel read/write
    extern std::atomic_flag stbi_png_write_lock;

    // A RAII class that locks the atomic flag in the constructor and unlocks it in the destructor.
    struct atomic_flag_guard
    {
        NONCOPYABLE(atomic_flag_guard);
        atomic_flag_guard() = delete;
        explicit atomic_flag_guard(std::atomic_flag& flag) noexcept;
        ~atomic_flag_guard() noexcept;
    private:
        std::atomic_flag* m_lock;
    };

    // A RAII wrapper for a mutex.
    struct adaptive_atomic_guard
    {
        NONCOPYABLE(adaptive_atomic_guard);
        adaptive_atomic_guard() = delete;
        explicit adaptive_atomic_guard(std::mutex& flag, int64_t& period_nano) noexcept;
        ~adaptive_atomic_guard() noexcept;
    private:
        std::mutex* m_lock;
        int64_t* m_period_nano;
    };


    //  1. It is a base class for all atomic static classes.
    //  2. It provides a static lock for all atomic classes.
    //  3. It provides a static lock guard for all atomic classes.
    //  4. It provides a protected non-virtual destructor that prevents delete by base pointer.
    //  5. It provides a static atomic flag for all atomic classes.
    struct BaseAtomicClassStatic
    {
    public:
#define LOCK_GUARD_S() atomic_flag_guard __lock_s(sc_flag);ATOMIC_THREAD_FENCE
        static void LockS() noexcept;
        static void UnLockS() noexcept;
    protected:
        // yuhang : protected non-virtual destructor that prevents delete by base pointer
        ~BaseAtomicClassStatic() noexcept = default;

    protected:
        CACHE_ALIGN inline static std::atomic_flag sc_flag; // c++ 20 default initialization to false
    };


    //  1. A base class for all atomic non-instantiable classes.
    //  2. It provides a static lock for all atomic classes.
    //  3. It provides a static lock guard for all atomic classes.
    //  4. It provides a protected non-virtual destructor that prevents delete by base pointer.
    //  5. It provides a static atomic flag for all atomic classes.
    struct BaseAtomicClassNI
    {
    public:
        NONINSTANTIABLE(BaseAtomicClassNI);
#define LOCK_GUARD_NI() atomic_flag_guard __lock_ni(ni_flag);ATOMIC_THREAD_FENCE
        static void LockNI() noexcept;
        static void UnLockNI() noexcept;
    protected:
        CACHE_ALIGN inline static std::atomic_flag ni_flag; // c++ 20 default initialization to false
    };


    // 1. It is a base class for all atomic non-copyable classes.
    // 2. It provides a lock for all atomic classes.
    // 3. It provides a lock guard for all atomic classes.
    // 4. It provides a protected non-virtual destructor that prevents delete by base pointer.
    // 5. It provides a atomic flag for all atomic classes.
    // 6. The atomic flag is padded to be cache aligned to avoid false sharing
    struct MS_ALIGN8 BaseAtomicClassNC
    {
    public:
        NONCOPYABLE(BaseAtomicClassNC);

#define LOCK_GUARD_NC() atomic_flag_guard __lock_nc(nc_flag);ATOMIC_THREAD_FENCE

        BaseAtomicClassNC() = default;

        void LockNC() const noexcept;
        void UnLockNC() const noexcept;
    protected:
        // yuhang : protected non-virtual destructor that prevents delete by base pointer
        ~BaseAtomicClassNC() noexcept = default;

    protected:
        mutable std::atomic_flag nc_flag;
    private:
#if PADDING
        // yuhang : use byte padding instead of CACHE_ALIGN specifier so that inherited class is not affected by this alignment
        std::byte COMBINE(__PADDING_, __LINE__)[PLATFORM_CACHE_LINE - sizeof(std::atomic_flag)];
#endif
    };

    // 1. It is a base class for all atomic copyable classes. The lock state is not copied.
    // 2. It provides a lock for all atomic classes.
    // 3. It provides a lock guard for all atomic classes.
    // 4. It provides a protected non-virtual destructor that prevents delete by base pointer.
    // 5. It provides a atomic flag for all atomic classes.
    // 6. The atomic flag is padded to be cache aligned to avoid false sharing
    struct MS_ALIGN8 BaseAtomicClass
    {
    public:
#define LOCK_GUARD() atomic_flag_guard __lock(m_flag);ATOMIC_THREAD_FENCE

        BaseAtomicClass() noexcept = default;

        BaseAtomicClass(const BaseAtomicClass& other) noexcept
        {
            // Leave as empty to not copy the atomic flag
        }

        BaseAtomicClass(BaseAtomicClass&& other) noexcept
        {
            // Leave as empty to not copy the atomic flag
        }

        BaseAtomicClass& operator=(const BaseAtomicClass& rhs) noexcept { return *this; }
        BaseAtomicClass& operator=(BaseAtomicClass&& rhs) noexcept { return *this; }

        void Lock() const noexcept;
        void UnLock() const noexcept;
    protected:
        // yuhang : protected non-virtual destructor that prevents delete by base pointer
        ~BaseAtomicClass() noexcept = default;

    protected:
        mutable std::atomic_flag m_flag;
    private:
#if PADDING
        // yuhang : use byte padding instead of CACHE_ALIGN specifier so that inherited class is not affected by this alignment
        std::byte COMBINE(__PADDING_, __LINE__)[PLATFORM_CACHE_LINE - sizeof(std::atomic_flag)];
#endif
    };

    
    // 1. It is a base class for all atomic non-copyable classes.
    // 2. It provides a spin-mutex lock for all atomic classes.
    // 3. It provides a lock guard for all atomic classes.
    // 4. It provides a protected non-virtual destructor that prevents delete by base pointer.
    // 5. The mutex is padded to be cache aligned to avoid false sharing
    struct MS_ALIGN8 AdaptiveAtomicClassNC
    {
    public:
        NONCOPYABLE(AdaptiveAtomicClassNC);

#define LOCK_GUARD_ADAPTIVE_NC() adaptive_atomic_guard __lock_adaptive_nc(nc_mutex, nc_period_nano);ATOMIC_THREAD_FENCE

        AdaptiveAtomicClassNC() noexcept = default;

        void LockAdaptiveNC() const noexcept;
        void UnLockAdaptiveNC() const noexcept;
    protected:
        // yuhang : protected non-virtual destructor that prevents delete by base pointer
        ~AdaptiveAtomicClassNC() noexcept = default;

    protected:
        mutable std::mutex nc_mutex;
        mutable int64_t nc_period_nano{2000ull};

    private:
#if PADDING
        // yuhang : use byte padding instead of CACHE_ALIGN specifier so that inherited class is not affected by this alignment
        // yuhang : std::mutex is usually larger than 64 bytes. Need double check its size with cache line size
        std::byte COMBINE(__PADDING_, __LINE__)[((sizeof(std::mutex) > PLATFORM_CACHE_LINE) ? 2 * PLATFORM_CACHE_LINE : PLATFORM_CACHE_LINE) - sizeof(std::mutex) - sizeof(std::int64_t)];
#endif
    };
}

#undef PADDING
