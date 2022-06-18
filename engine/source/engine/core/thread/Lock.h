#pragma once
#include "../EngineCore.h"
#include <queue>
#include <atomic>

namespace longmarch
{
    // used in stbi image parallel read/write
    extern std::atomic_flag stbi_hdr_write_lock;
    // used in stbi image parallel read/write
    extern std::atomic_flag stbi_png_write_lock;

    struct atomic_flag_guard
    {
        NONCOPYABLE(atomic_flag_guard);
        atomic_flag_guard() = delete;
        explicit atomic_flag_guard(std::atomic_flag& flag) noexcept;
        ~atomic_flag_guard() noexcept;
    private:
        std::atomic_flag* m_lock;
    };

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

    /*
        Base atomic class for class static method
    */
    struct BaseAtomicClassStatic
    {
    public:
        virtual ~BaseAtomicClassStatic() = default;
#define LOCK_GUARD_S() atomic_flag_guard __lock_s(sc_flag)
        static void LockS() noexcept;
        static void UnlockS() noexcept;
    protected:
        inline static std::atomic_flag sc_flag; // c++ 20 default initialization to false
    };

    /*
        Base atomic class for NON-INSTANTIABLE classes
    */
    struct BaseAtomicClassNI
    {
    public:
        NONINSTANTIABLE(BaseAtomicClassNI);
#define LOCK_GUARD_NI() atomic_flag_guard __lock_ni(ni_flag)
        static void LockNI() noexcept;
        static void UnlockNI() noexcept;
    protected:
        inline static std::atomic_flag ni_flag; // c++ 20 default initialization to false
    };

    /*
        Base atomic class for NON-COPYABLE classes
    */
    struct BaseAtomicClassNC
    {
    public:
        NONCOPYABLE(BaseAtomicClassNC);

#define LOCK_GUARD_NC() atomic_flag_guard __lock_nc(nc_flag)

        BaseAtomicClassNC() noexcept = default;
        virtual ~BaseAtomicClassNC() noexcept = default;

        void LockNC() const noexcept;
        void UnlockNC() const noexcept;

    protected:
        mutable std::atomic_flag nc_flag;
    };

    /*
        Base atomic class for COPYABLE classes (size is 16 bytes)
        Note, that the state of the lock is not copied over on assignment or copy construction
    */
    struct BaseAtomicClass
    {
    public:
#define LOCK_GUARD() atomic_flag_guard __lock(m_flag)

        BaseAtomicClass() noexcept = default;
        virtual ~BaseAtomicClass() noexcept = default;

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
        mutable std::atomic_flag m_flag;
    };

    /*
        Base adaptive atomic lock class for NON-COPYABLE classes (size is 96 bytes)
    */
    struct AdaptiveAtomicClassNC
    {
    public:
        NONCOPYABLE(AdaptiveAtomicClassNC);

#define LOCK_GUARD_ADAPTIVE_NC() adaptive_atomic_guard __lock_adaptive_nc(nc_mutex, nc_period_nano)

        AdaptiveAtomicClassNC() noexcept = default;
        virtual ~AdaptiveAtomicClassNC() noexcept = default;

        void LockAdaptiveNC() const noexcept;
        void UnlockAdaptiveNC() const noexcept;

    protected:
        mutable std::mutex nc_mutex;
        mutable int64_t nc_period_nano{2000ull};
    };
}
