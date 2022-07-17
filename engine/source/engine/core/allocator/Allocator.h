#pragma once
#include <stdint.h>
#include <vector>
#include "../thread/Lock.h"

namespace longmarch
{
    typedef uint16_t blockSize_t;

    //! Reference : https://stackoverflow.com/questions/16198700/using-the-extra-16-bits-in-64-bit-pointers
    template <typename T>
    struct MS_ALIGN8 LongMarch_64Ptr
    {
        NONINSTANTIABLE(LongMarch_64Ptr);
        signed long long ptr : 48; // as per phuclv's comment, we need the type to be signed to be sign extended
        unsigned long long size : 15;
        // ...and, what's more, as Peter Cordes pointed out, it's better to mark signedness of bit field explicitly (before C++14)
        unsigned long long free : 1;
        // Additionally, as Peter found out, types can differ by sign and it doesn't mean the beginning of another bit field (MSVC is particularly strict about it: other type == new bit field)
        inline void operator=(T* rhs)
        {
            ptr = reinterpret_cast<signed long long>(rhs);
        }

        inline operator T*()
        {
            return reinterpret_cast<T*>(ptr);
        }

        inline T* operator->()
        {
            return reinterpret_cast<T*>(ptr);
        }
    };

    /**
     *  @brief BlockHeader has been optimized with platform of 64 bits addresses that only utilize the lower 48 bits
     *         BlockHeader's pNext pointer has bit utilization of 64 bits in the following order
     *         1 bit : free | 15 bits : block size | 48 bits : pointer to next free block     
     */
    struct MS_ALIGN8 BlockHeader
    {
        // Mark BlockHeader as NONINSTANTIABLE to remind ourself only use it as a pointer
        NONINSTANTIABLE(BlockHeader);
        LongMarch_64Ptr<BlockHeader> pNext;

        inline static bool GetFree(void* ptr) noexcept
        {
            return GetBlock(ptr)->pNext.free;
        }

        inline static blockSize_t GetSize(void* ptr) noexcept
        {
            return GetBlock(ptr)->pNext.size;
        }

        inline static BlockHeader* GetBlock(void* content) noexcept
        {
            return reinterpret_cast<BlockHeader*>(content) - 1;
        }

        inline static void* GetPtr(BlockHeader* block) noexcept
        {
            return reinterpret_cast<void*>(block + 1);
        }

        inline static BlockHeader* NextBlock(BlockHeader* block, size_t blockSize) noexcept
        {
            return reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(block + 1) + blockSize);
        }
    };

    // Stack page
    struct PageHeader
    {
        NONINSTANTIABLE(PageHeader);

        inline BlockHeader* GetBlockHeader()
        {
            return reinterpret_cast<BlockHeader*>(this);
        }
    };

    class Allocator : public BaseAtomicClassNC
    {
    public:
        NONCOPYABLE(Allocator);
        Allocator() = default;
        ~Allocator() { FreeAll(); }

        // resets the allocator to a new configuration
        void Reset(size_t data_size, size_t page_size, size_t alignment) noexcept;
        // alloc and free blocks
        void* Allocate() noexcept;
        void Free(void* p);
        void FreeAll() noexcept;
    private:
        void AllocateNewPage() noexcept;
    private:
        // the page list
        std::vector<PageHeader*> m_pPageList;

        // the free block list
        CACHE_ALIGN BlockHeader * m_pFreeList = {nullptr};

        size_t m_szDataSize = {0};
        size_t m_szPageSize = {0};
        size_t m_szAlignmentSize = {0};
        size_t m_szBlockSize = {0};
        size_t m_nBlocksPerPage = {0};

#if defined(_DEBUG)
        // statistics
        size_t m_nPages = {0};
        size_t m_nBlocks = {0};
        size_t m_nFreeBlocks = {0};

        // debug patterns
        constexpr static const uint8_t PATTERN_ALIGN = 0xFC;
        constexpr static const uint8_t PATTERN_ALLOC = 0xFD;
        constexpr static const uint8_t PATTERN_FREE = 0xFE;

        // fill a free page with debug patterns
        void FillFreePage(PageHeader* pPage) noexcept;

        // fill a block with debug patterns
        void FillFreeBlock(BlockHeader* pBlock) noexcept;

        // fill an allocated block with debug patterns
        void FillAllocatedBlock(BlockHeader* pBlock) noexcept;
#endif
    };
}
