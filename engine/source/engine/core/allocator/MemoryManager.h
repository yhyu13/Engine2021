#pragma once
#include <memory>
#include "Allocator.h"

#ifdef _DEBUG
#define CUSTOM_ALLOCATOR 1
#else
#define CUSTOM_ALLOCATOR 1
#endif // _DEBUG

namespace longmarch
{
    template <class T>
    using LongMarch_Shared_ptr = std::shared_ptr<T>;
    template <class T>
    struct A4Games_Deleter;
    template <class T>
    using LongMarch_Unique_ptr = std::unique_ptr<T>;

    // Hard to work with custom deleter struct
    //template<class T>
    //using LongMarch_Unique_ptr = std::unique_ptr<T, A4Games_Deleter<T>>;

    /**
     * @brief Custom MemoryManager that uses a segregated memory list for 8 , 32, 64 alignments
     *
     * Use it like : MemoryManager::Make_shared<T>(args...);
     *				 MemoryManager::Make_unique<T>(args...);
     *				 Use std::allocator to allocate values with smaller than 8 bytes, it is not going to be efficient
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    class ENGINE_API MemoryManager
    {
    public:
        NONINSTANTIABLE(MemoryManager);

        static void Init();
        static void Shutdown();

        inline static size_t GetCurrentManagedMemory() noexcept
        {
            return s_AllocatedSize;
        }

        // Replacement for make_shared
        template <class T, typename... Arguments>
        [[nodiscard]] inline constexpr static LongMarch_Shared_ptr<T> Make_shared(Arguments&&... args) noexcept
        {
#if CUSTOM_ALLOCATOR == 1
            return std::shared_ptr<T>(New<T>(std::forward<Arguments>(args)...), Delete<T>);
#else
			return std::make_shared<T>(std::forward<Arguments>(args)...);
#endif // CUSTOM_ALLOCATOR
        }

        // Replacement for make_unique
        template <class T, typename... Arguments>
        [[nodiscard]] inline constexpr static LongMarch_Unique_ptr<T> Make_unique(Arguments&&... args) noexcept
        {
            return std::make_unique<T>(std::forward<Arguments>(args)...);
        }

    private:
        // Replacement for new
        template <class T, typename... Arguments>
        [[nodiscard]] inline constexpr static T* New(Arguments&&... args) noexcept
        {
            s_AllocatedSize += sizeof(T);
            //DEBUG_PRINT("New : " + Str(typeid(T).name()) + " " + Str(sizeof(T)) + " " + Str(s_AllocatedSize));
#if CUSTOM_ALLOCATOR == 1
            return new(Allocate(sizeof(T))) T(std::forward<Arguments>(args)...);
#else
			return new T(std::forward<Arguments>(args)...);
#endif // CUSTOM_ALLOCATOR
        }

        // Replacement for delete
        template <class T>
        inline constexpr static void Delete(T* p) noexcept
        {
            s_AllocatedSize -= sizeof(T);
            //DEBUG_PRINT("Delete : " + Str(typeid(T).name()) + " " + Str(sizeof(T)) + " " + Str(BlockHeader::GetSize(p)) + " " + Str(s_AllocatedSize));
#if CUSTOM_ALLOCATOR == 1
            p->~T();
            Free(p, sizeof(T));
#else
			delete p;
#endif // CUSTOM_ALLOCATOR
        }

    public:
        [[nodiscard]] static void* Allocate(const size_t size, const size_t alignment) noexcept;

        // Replacement for malloc()
        [[nodiscard]] static void* Allocate(const size_t size) noexcept;

        // Replacement for free()
        static void Free(void* p, const size_t size) noexcept;

    private:
        [[nodiscard]] static Allocator* LookUpAllocator(const size_t size) noexcept;

    private:
        constexpr inline static const uint32_t kBlockSizes[] =
        {
            8, 16, 24, 32, 40, 48, 56,
            64, 72, 80, 88, 96,

            128, 160, 192, 224, 256,
            320, 384, 448, 512, 576, 640,

            704, 768, 832, 896, 960, 1024
        };

        constexpr inline static const uint32_t kPageSize = {1u << 12};
        constexpr inline static const uint32_t kAlignment = {1u << 3};

        // number of elements in the block size array
        constexpr inline static const uint32_t kNumBlockSizes = {sizeof(kBlockSizes) / sizeof(kBlockSizes[0])};

        // largest valid block size
        constexpr inline static const uint32_t kMaxBlockSize = {kBlockSizes[kNumBlockSizes - 1]};

        inline static size_t* s_pBlockSizeLookup = {nullptr};
        inline static Allocator* s_pAllocators = {nullptr};
        inline static std::atomic_size_t s_AllocatedSize = {0u};
        inline static std::once_flag s_flag_init;

        template <typename T>
        friend struct Mallocator;
        template <typename T>
        friend struct A4Games_Deleter;
    };

    template <class T>
    struct A4Games_Deleter
    {
        // deleter
        void operator()(T* p) const
        {
            MemoryManager::s_AllocatedSize -= sizeof(T);
            //DEBUG_PRINT("Delete : " + Str(typeid(T).name()) + " " + Str(sizeof(T)) + " " + Str(BlockHeader::GetSize(Allocator::GetBlock(p))) + " " + Str(MemoryManager::s_AllocatedSize));
#if CUSTOM_ALLOCATOR == 1
            p->~T();
            MemoryManager::Free(p, sizeof(T));
#else
			delete p;
#endif // CUSTOM_ALLOCATOR
        };
    };

    /*
        Reference https://en.cppreference.com/w/cpp/named_req/Allocator
    */
    template <class T>
    struct Mallocator
    {
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        using propagate_on_container_move_assignment = std::true_type;
        using is_always_equal = std::true_type;

        constexpr Mallocator() noexcept
        {
        }

        constexpr Mallocator(const Mallocator&) noexcept = default;

        template <class U>
        constexpr Mallocator(const Mallocator<U>&) noexcept
        {
        };

        [[nodiscard]] T* allocate(std::size_t n)
        {
            // Using static_cast instead of reinterpret_cast because MemoryManager::Allocate might return nullptr or NULL
            if (auto p = static_cast<T*>(MemoryManager::Allocate(n * sizeof(T))); p)
            [[likely]]
            {
                MemoryManager::s_AllocatedSize += n * sizeof(T);
                //DEBUG_PRINT("Allocate vector : " + Str(typeid(T).name()) + " " + Str(n) + " " + Str(sizeof(T)) + " " + Str(n * sizeof(T)) + " " + Str(MemoryManager::s_AllocatedSize));
                return p;
            }
            else
            [[unlikely]]
            {
                throw std::bad_alloc();
            }
        }

        void deallocate(T* p, std::size_t n) noexcept
        {
            MemoryManager::Free(p, sizeof(T) * n);
            MemoryManager::s_AllocatedSize -= n * sizeof(T);
            //DEBUG_PRINT("Free vector : " + Str(typeid(T).name()) + " " + Str(n) + " " + Str(sizeof(T)) + " " + Str(n * sizeof(T)) + " " + Str(MemoryManager::s_AllocatedSize));
        }
    };

    template <class W, class U>
    inline bool operator==(const longmarch::Mallocator<W>&, const longmarch::Mallocator<U>&) { return true; }

    template <class W, class U>
    inline bool operator!=(const longmarch::Mallocator<W>&, const longmarch::Mallocator<U>&) { return false; }
}
