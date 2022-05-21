#include "engine-precompiled-header.h"
#include "MemoryManager.h"

#ifndef ALIGN
#define ALIGN(x, a)         (((x) + ((a) - 1)) & ~((a) - 1))
#endif

void longmarch::MemoryManager::Init()
{
    // initialize block size lookup table
    s_pBlockSizeLookup = new size_t[kMaxBlockSize + 1];
    size_t j = 0;
    for (size_t i = 0; i <= kMaxBlockSize; i++)
    {
        if (i > kBlockSizes[j]) ++j;
        s_pBlockSizeLookup[i] = j;
    }
    // Sanity check!
    if (j != kNumBlockSizes - 1)
    {
        throw std::runtime_error("Memory manger pool initialization has failed.");
    }
    // initialize the allocators
    s_pAllocators = new Allocator[kNumBlockSizes];
    for (size_t i = 0; i < kNumBlockSizes; i++)
    {
        s_pAllocators[i].Reset(kBlockSizes[i], kPageSize, kAlignment);
    }
}

void longmarch::MemoryManager::Shutdown()
{
    /* We found that shared pointers might call deleter in the very end of the exit
       even after Shutdown() while s_pAllocators and s_pBlockSizeLookup
       are already deleted. To avoid that error, simply leave these memory
       for the operating system to clean up.
       */
    //delete[] s_pAllocators;
    //delete[] s_pBlockSizeLookup;
}

Allocator* longmarch::MemoryManager::LookUpAllocator(const size_t size) noexcept
{
    // check eligibility for lookup
    if (size <= kMaxBlockSize)
    [[likely]]
    {
        return s_pAllocators + s_pBlockSizeLookup[size];
    }
    else
    [[unlikely]]
    {
        return nullptr;
    }
}

[[nodiscard]]
void* longmarch::MemoryManager::Allocate(const size_t size, const size_t alignment) noexcept
{
    // There might be cases where allocation happens before we call Init, so we do init here
    std::call_once(s_flag_init, MemoryManager::Init);

    uint8_t* p;
    const size_t _size = ALIGN(size, alignment);
    if (Allocator* pAlloc = LookUpAllocator(_size))
    [[likely]]
    {
        p = reinterpret_cast<uint8_t*>(pAlloc->Allocate());
    }
    else
    [[unlikely]]
    {
        p = reinterpret_cast<uint8_t*>(malloc(_size));
    }

    p = reinterpret_cast<uint8_t*>(ALIGN(reinterpret_cast<size_t>(p), alignment));

    return reinterpret_cast<void*>(p);
}

// Replacement for malloc()

[[nodiscard]]
void* longmarch::MemoryManager::Allocate(const size_t size) noexcept
{
    // There might be cases where allocation happens before we call Init, so we do init here
    std::call_once(s_flag_init, MemoryManager::Init);

#if CUSTOM_ALLOCATOR
    if (size <= kMaxBlockSize)
    [[likely]]
    {
        return (s_pAllocators + s_pBlockSizeLookup[size])->Allocate();
    }
    else
    [[unlikely]]
    {
        return malloc(size);
    }
#else
	return malloc(size);
#endif // CUSTOM_ALLOCATOR
}

// Replacement for free()

void longmarch::MemoryManager::Free(void* p, const size_t size) noexcept
{
#if CUSTOM_ALLOCATOR
    if (size <= kMaxBlockSize)
    [[likely]]
    {
        /*
        Find the block size that was used to allocate pointer p by querying the block header.
        */
        (s_pAllocators + s_pBlockSizeLookup[BlockHeader::GetSize(p)])->Free(p);
    }
    else
    [[unlikely]]
    {
        free(p);
    }
#else
	free(p);
#endif // CUSTOM_ALLOCATOR
}
