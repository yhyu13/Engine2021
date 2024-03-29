#include "engine-precompiled-header.h"
#include "MemoryManager.h"

#ifndef ALIGN
#define ALIGN(x, a)         (((x) + ((a) - 1)) & ~((a) - 1))
#endif

void longmarch::MemoryManager::Init()
{
    // initialize block size lookup table
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
        (s_pAllocators + s_pBlockSizeLookup[size])->Free(p);
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
