#include "../include/MemoryPool.hpp"
#include <cstring>
#include <stdexcept>

namespace allocity {

MemoryPool::MemoryPool(std::size_t blockSize, std::size_t blockCount)
    : m_blockSize(blockSize), m_capacity(blockCount), m_usedBlocks(0), m_memory(nullptr), m_freeList(nullptr) {
    if (blockSize < sizeof(void*)) {
        throw std::invalid_argument("Block size must be at least the size of a pointer");
    }
    m_memory = new char[m_blockSize * m_capacity];
    InitializeFreeList();
}

MemoryPool::~MemoryPool() {
    delete[] m_memory;
}

void MemoryPool::InitializeFreeList() {
    m_freeList = m_memory;
    char* current = m_memory;
    for (std::size_t i = 0; i < m_capacity - 1; ++i) {
        *reinterpret_cast<char**>(current) = current + m_blockSize;
        current += m_blockSize;
    }
    *reinterpret_cast<char**>(current) = nullptr; 
}

void* MemoryPool::Allocate() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_freeList == nullptr) {
        return nullptr; 
    }
    void* result = m_freeList;
    m_freeList = *reinterpret_cast<void**>(m_freeList);
    ++m_usedBlocks;
    return result;
}

void MemoryPool::Deallocate(void* ptr) {
    if (ptr == nullptr) return;
    if (ptr < m_memory || ptr >= m_memory + m_blockSize * m_capacity) {
        throw std::invalid_argument("Pointer does not belong to this memory pool");
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    *reinterpret_cast<void**>(ptr) = m_freeList;
    m_freeList = ptr;
    --m_usedBlocks;
}

void MemoryPool::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    InitializeFreeList();
    m_usedBlocks = 0;
}

} 
