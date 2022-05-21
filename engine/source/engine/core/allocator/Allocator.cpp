#include "engine-precompiled-header.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "Allocator.h"

#ifndef ALIGN
#define ALIGN(x, a)         (((x) + ((a) - 1)) & ~((a) - 1))
#endif

void longmarch::Allocator::Reset(size_t data_size, size_t page_size, size_t alignment) noexcept
{
    FreeAll();

    m_szDataSize = data_size;
    m_szPageSize = page_size;

    size_t minimal_size = (sizeof(BlockHeader) > m_szDataSize) ? sizeof(BlockHeader) : m_szDataSize;
    // this magic only works when alignment is 2^n, which should general be the case
    // because most CPU/GPU also requires the aligment be in 2^n
    // but still we use a assert to guarantee it
    assert(alignment > 0 && ((alignment & (alignment - 1))) == 0);

    m_szBlockSize = ALIGN(minimal_size, alignment);

    // Storing m_szDataSize as blockSize_t right before each block
    assert(m_szBlockSize == (size_t)blockSize_t(m_szBlockSize));

    m_szAlignmentSize = m_szBlockSize - minimal_size;
    m_nBlocksPerPage = (m_szPageSize) / (m_szBlockSize + sizeof(BlockHeader));
}

void longmarch::Allocator::AllocateNewPage() noexcept
{
    // allocate a new page
    auto alloc = this;
    if (PageHeader* pNewPage = reinterpret_cast<PageHeader*>(std::calloc(1, alloc->m_szPageSize));
        !pNewPage)
    [[unlikely]]
    {
        throw std::bad_alloc();
    }
    else
    [[likely]]
    {
#if defined(_DEBUG)
		++alloc->m_nPages;
		alloc->m_nBlocks += alloc->m_nBlocksPerPage;
		alloc->m_nFreeBlocks += alloc->m_nBlocksPerPage;
		alloc->FillFreePage(pNewPage);
#endif
        alloc->m_pPageList.push_back(pNewPage);

        BlockHeader* pBlock = pNewPage->Blocks();
        blockSize_t size = alloc->m_szBlockSize;
        // link each block in the page
        for (auto i(0u); i < alloc->m_nBlocksPerPage - 1; ++i)
        {
            pBlock->pNext.size = size;
            pBlock->pNext.free = true;
            pBlock->pNext = BlockHeader::NextBlock(pBlock, alloc->m_szBlockSize);
            pBlock = pBlock->pNext;
        }
        //link the last block
        pBlock->pNext.size = size;
        pBlock->pNext.free = true;
        pBlock->pNext = nullptr;

        alloc->m_pFreeList = pNewPage->Blocks();
    }
}

void* longmarch::Allocator::Allocate() noexcept
{
    BlockHeader* freeBlock;
    {
        LOCK_GUARD_NC();
        while (!m_pFreeList)
        {
            AllocateNewPage();
        }
        freeBlock = m_pFreeList;
        m_pFreeList = freeBlock->pNext;
    }
    freeBlock->pNext.free = false;
#if defined(_DEBUG)
	--m_nFreeBlocks;
	FillAllocatedBlock(freeBlock);
#endif
    return BlockHeader::GetPtr(freeBlock);
}

void longmarch::Allocator::Free(void* p)
{
    if (BlockHeader* block = BlockHeader::GetBlock(p);
        block->pNext.free)
    [[unlikely]]
    {
        throw std::runtime_error(std::string("Double free!"));
    }
    else if (block->pNext.size != static_cast<blockSize_t>(m_szBlockSize))
    [[unlikely]]
    {
        throw std::runtime_error(std::string("Segementation fault!"));
    }
    else
    [[likely]]
    {
        block->pNext.free = true;
        {
            LOCK_GUARD_NC();
            block->pNext = m_pFreeList;
            m_pFreeList = block;
        }
#if defined(_DEBUG)
		++m_nFreeBlocks;
		FillFreeBlock(block);
#endif
    }
}

void longmarch::Allocator::FreeAll() noexcept
{
    LOCK_GUARD_NC();
    while (!m_pPageList.empty())
    {
        std::free(m_pPageList.back());
        m_pPageList.pop_back();
    }
    m_pPageList.clear();
    m_pFreeList = nullptr;
#if defined(_DEBUG)
	m_nPages = 0;
	m_nBlocks = 0;
	m_nFreeBlocks = 0;
#endif
}

#if defined(_DEBUG)
void longmarch::Allocator::FillFreePage(PageHeader* pPage) noexcept
{
	// blocks
	BlockHeader* pBlock = pPage->Blocks();
	for (uint32_t i = 0; i < m_nBlocksPerPage; i++)
	{
		FillFreeBlock(pBlock);
		pBlock = BlockHeader::NextBlock(pBlock, m_szBlockSize);
	}
}

void longmarch::Allocator::FillFreeBlock(BlockHeader* pBlock) noexcept
{
	// block header + data
	memset(pBlock + 1, PATTERN_FREE, m_szBlockSize - m_szAlignmentSize);

	// alignment
	memset(reinterpret_cast<uint8_t*>(pBlock + 1) + m_szBlockSize - m_szAlignmentSize,
		PATTERN_ALIGN, m_szAlignmentSize);
}

void longmarch::Allocator::FillAllocatedBlock(BlockHeader* pBlock) noexcept
{
	// block header + data
	memset(pBlock + 1, PATTERN_ALLOC, m_szBlockSize - m_szAlignmentSize);

	// alignment
	memset(reinterpret_cast<uint8_t*>(pBlock + 1) + m_szBlockSize - m_szAlignmentSize,
		PATTERN_ALIGN, m_szAlignmentSize);
}
#endif
