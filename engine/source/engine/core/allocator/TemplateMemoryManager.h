#pragma once

#include "MemoryManager.h"

namespace longmarch
{
    /**
     *  @brief A specialized memory manager for a given template class
     *
     *  @detail Support maximum class size of 4096 byte, consider using malloc for class bigger than that.
     */
    template <class T>
    class ENGINE_API TemplateMemoryManager
    {
    public:
        NONINSTANTIABLE(TemplateMemoryManager);

        static void Init();
        static void Shutdown();

        inline static size_t GetCurrentManagedMemory() noexcept
        {
            return s_AllocatedSize;
        }

        // Replacement for make_shared
        template <typename... Arguments>
        [[nodiscard]] inline static std::shared_ptr<T> Make_shared(Arguments&&... args) noexcept
        {
#if CUSTOM_ALLOCATOR == 1
            return std::shared_ptr<T>(New<T>(std::forward<Arguments>(args)...), Delete);
#else
			return std::make_shared<T>(std::forward<Arguments>(args)...);
#endif // CUSTOM_ALLOCATOR
        }

        // Replacement for make_unique
        template <typename... Arguments>
        [[nodiscard]] inline static std::unique_ptr<T> Make_unique(Arguments&&... args) noexcept
        {
            return std::make_unique<T>(std::forward<Arguments>(args)...);
        }

        // Replacement for new
        template <typename... Arguments>
        [[nodiscard]] inline static T* New(Arguments&&... args) noexcept
        {
            s_AllocatedSize += sizeof(T);
            //DEBUG_PRINT("New : " + Str(typeid(T).name()) + " " + Str(sizeof(T)) + " " + Str(s_AllocatedSize));
#if CUSTOM_ALLOCATOR == 1
            return new(Allocate()) T(std::forward<Arguments>(args)...);
#else
			return new T(std::forward<Arguments>(args)...);
#endif // CUSTOM_ALLOCATOR
        }

        // Replacement for delete
        inline static void Delete(T* p) noexcept
        {
            s_AllocatedSize -= sizeof(T);
            //DEBUG_PRINT("Delete : " + Str(typeid(T).name()) + " " + Str(sizeof(T)) + " " + Str(BlockHeader::GetSize(p)) + " " + Str(s_AllocatedSize));
#if CUSTOM_ALLOCATOR == 1
            p->~T();
            Free(p);
#else
			delete p;
#endif // CUSTOM_ALLOCATOR
            p = nullptr;
        }

        // Replacement for malloc()
        [[nodiscard]] static void* Allocate() noexcept;

        // Replacement for free()
        static void Free(void* p) noexcept;

    private:
        constexpr inline static const bool bUseLargeBlock = {sizeof(T) > MemoryManager::kMaxBlockSize};
        constexpr inline static const uint32_t kMaxElementSize = {1u << 12};
        constexpr inline static const uint32_t kPageSize = {1u << 12};
        constexpr inline static const uint32_t kPageSizeLarge = {1u << 21};
        constexpr inline static const uint32_t kAlignment = {1u << 3};

        inline static CACHE_ALIGN64 std::atomic_size_t s_AllocatedSize = {0u};

        inline static Allocator s_Allocator;
        inline static std::once_flag s_flag_init;
    };
}

namespace longmarch
{
    template <class T>
    void TemplateMemoryManager<T>::Init()
    {
        static_assert(sizeof(T) >= kMaxElementSize,
            "Class has size greater than allowed! Consider using std::malloc instead");
        // initialize the allocator
        s_Allocator.Reset(sizeof(T),
                          (!bUseLargeBlock) ? kPageSize : kPageSizeLarge,
                          kAlignment);
    }

    template <class T>
    void TemplateMemoryManager<T>::Shutdown()
    {
        /* We found that shared pointers might call deleter in the very end of the exit
           even after Shutdown() while s_pAllocators and s_pBlockSizeLookup
           are already deleted. To avoid that error, simply leave these memory
           for the operating system to clean up.
           */
        //delete s_Allocator;
    }

    template <class T>
    void* TemplateMemoryManager<T>::Allocate() noexcept
    {
#if CUSTOM_ALLOCATOR
        // There might be cases where allocation happens before we call Init, so we do init here
        std::call_once(s_flag_init, TemplateMemoryManager::Init);
        return s_Allocator.Allocate();
#else
        return malloc(size);
#endif
    }

    template <class T>
    void TemplateMemoryManager<T>::Free(void* p) noexcept
    {
#if CUSTOM_ALLOCATOR
        s_Allocator.Free(p);
#else
        free(p);
#endif
    }
}
