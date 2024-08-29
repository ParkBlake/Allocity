#pragma once

#include <cstddef>
#include <vector>
#include <mutex>

namespace allocity {

class MemoryPool {
public:

    MemoryPool(std::size_t blockSize, std::size_t blockCount);
    ~MemoryPool();

    void* Allocate();
    void Deallocate(void* ptr);
    void Clear();

    std::size_t GetBlockSize() const { return m_blockSize; }
    std::size_t GetCapacity() const { return m_capacity; }
    std::size_t GetUsedBlocks() const { return m_usedBlocks; }

private:
    std::size_t m_blockSize;
    std::size_t m_capacity;
    std::size_t m_usedBlocks;
    char* m_memory;
    void* m_freeList;
    std::mutex m_mutex;

    void InitializeFreeList();
};

} 