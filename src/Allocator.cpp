#include "../include/Allocator.hpp"
#include "../include/AllocityThread.hpp"
#include <iostream>
#include <mutex>
#include <cstring>
#include <thread>

namespace allocity {

Allocator::Allocator() 
    : m_DefaultAllocator(), 
      m_AllocationMap(), 
      m_AllocationMutex(), 
      m_DeallocatedPointers(), 
      m_debugMode(false),
      m_MemoryPools(NUM_MEMORY_POOLS),
      m_StopThreads(false) {
    InitializeMemoryPools();
    InitializeThreadPool(std::thread::hardware_concurrency());
}

Allocator::~Allocator() {
    FinalCleanup();
}

void Allocator::InitializeMemoryPools() {
    m_MemoryPools.clear();
    m_MemoryPools.reserve(NUM_MEMORY_POOLS);
    for (size_t i = 0; i < NUM_MEMORY_POOLS; ++i) {
        m_MemoryPools.push_back(std::make_unique<MemoryPool>((i + 1) * 8, 1024));
    }
}

bool Allocator::IsPoolAllocation(std::size_t size) const {
    return size <= MAX_SMALL_OBJECT_SIZE;
}

void Allocator::InitializeThreadPool(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        m_ThreadPool.emplace_back(&Allocator::ThreadWorker, this);
    }
}

void Allocator::AddWorkToQueue(std::function<void()> work) {
    {
        std::lock_guard<std::mutex> lock(m_ThreadPoolMutex);
        m_WorkQueue.push(std::move(work));
    }
    m_ThreadPoolCondition.notify_one();
}

void Allocator::ThreadWorker() {
    while (!m_StopThreads) {
        std::unique_lock<std::mutex> lock(m_ThreadPoolMutex);
        m_ThreadPoolCondition.wait(lock, [this] { return m_StopThreads || !m_WorkQueue.empty(); });

        if (!m_WorkQueue.empty()) {
            auto work = m_WorkQueue.front();
            m_WorkQueue.pop();
            lock.unlock();
            work();
        }
    }
}

const DefaultAllocator& Allocator::GetDefaultAllocator() const {
    return m_DefaultAllocator;
}

void Allocator::SetDefaultAllocator(const DefaultAllocator& allocator) {
    m_DefaultAllocator = allocator;
}

void* Allocator::Allocate(std::size_t size) {
    if (size == 0) {
        std::cout << "Allocating 0 bytes, returning nullptr\n";
        return nullptr;
    }

    void* ptr = nullptr;
    bool isPoolAllocation = IsPoolAllocation(size);

    if (isPoolAllocation) {
        ptr = AllocateFromPool(size);
    } else {
        ptr = m_DefaultAllocator.Allocate(size);
    }

    if (ptr) {
        std::lock_guard<std::mutex> lock(m_AllocationMutex);
        TrackAllocation(ptr, size, isPoolAllocation);

        if (m_debugMode) {
            for (std::size_t i = 0; i < size; ++i) {
                if (static_cast<unsigned char*>(ptr)[i] == DEBUG_PATTERN) {
                    std::cerr << "Warning: Possible use-after-free detected at " << ptr << std::endl;
                    break;
                }
            }
        }
    }
    return ptr;
}

void* Allocator::AllocateFromPool(std::size_t size) {
    size_t poolIndex = (size - 1) / 8;
    return m_MemoryPools[poolIndex]->Allocate();
}

void Allocator::Deallocate(void* ptr) {
    if (ptr == nullptr) {
        std::cout << "Attempting to deallocate nullptr, ignoring\n";
        return;
    }

    std::size_t size;
    bool isPoolAllocation;
    {
        std::lock_guard<std::mutex> lock(m_AllocationMutex);
        auto it = m_AllocationTracker.find(ptr);
        if (it == m_AllocationTracker.end()) {
            throw std::runtime_error("Attempting to deallocate unknown pointer");
        }
        if (m_DeallocatedPointers.find(ptr) != m_DeallocatedPointers.end()) {
            throw std::runtime_error("Double free detected");
        }
        size = it->second.size;
        isPoolAllocation = it->second.isPoolAllocation;
    }

    if (isPoolAllocation) {
        DeallocateToPool(ptr, size);
    } else {
        std::cout << "Deallocating known pointer: " << ptr << " of size " << size << std::endl;
        if (m_debugMode) {
            std::memset(ptr, DEBUG_PATTERN, size);
        }
        m_DefaultAllocator.Deallocate(ptr, size);
    }

    std::lock_guard<std::mutex> lock(m_AllocationMutex);
    UntrackAllocation(ptr);
}

void Allocator::DeallocateToPool(void* ptr, std::size_t size) {
    size_t poolIndex = (size - 1) / 8;
    m_MemoryPools[poolIndex]->Deallocate(ptr);
}

void* Allocator::Assign(void* ptr) {
    return m_DefaultAllocator.Assign(ptr);
}

void Allocator::Deassign(void* ptr) {
    m_DefaultAllocator.Deassign(ptr);
}

void* Allocator::AlignedAllocate(std::size_t size, std::size_t alignment) {
    void* ptr = m_DefaultAllocator.AlignedAllocate(size, alignment);
    if (ptr) {
        std::lock_guard<std::mutex> lock(m_AllocationMutex);
        TrackAllocation(ptr, size, false);

        if (m_debugMode) {
            for (std::size_t i = 0; i < size; ++i) {
                if (static_cast<unsigned char*>(ptr)[i] == DEBUG_PATTERN) {
                    std::cerr << "Warning: Possible use-after-free detected at " << ptr << std::endl;
                    break;
                }
            }
        }
    }
    return ptr;
}

void Allocator::AlignedDeallocate(void* ptr) {
    if (ptr) {
        std::size_t size;
        {
            std::lock_guard<std::mutex> lock(m_AllocationMutex);
            auto it = m_AllocationTracker.find(ptr);
            if (it == m_AllocationTracker.end()) {
                throw std::runtime_error("Attempting to aligned deallocate unknown pointer");
            }
            if (m_DeallocatedPointers.find(ptr) != m_DeallocatedPointers.end()) {
                throw std::runtime_error("Double free detected");
            }
            size = it->second.size;
        }

        std::cout << "Deallocating aligned pointer: " << ptr << " of size " << size << std::endl;
        if (m_debugMode) {
            std::memset(ptr, DEBUG_PATTERN, size);
        }
        m_DefaultAllocator.AlignedDeallocate(ptr, size);

        std::lock_guard<std::mutex> lock(m_AllocationMutex);
        UntrackAllocation(ptr);
    }
}

void Allocator::SetOutOfMemoryHandler(std::function<void(std::size_t)> handler) {
    m_DefaultAllocator.SetOutOfMemoryHandler(std::move(handler));
}

void Allocator::SetMemoryUsageReporter(std::function<void(const DefaultAllocator&)> reporter) {
    m_DefaultAllocator.SetMemoryUsageReporter(std::move(reporter));
}

std::size_t Allocator::GetTotalAllocated() const {
    return m_DefaultAllocator.GetTotalAllocated();
}

std::size_t Allocator::GetTotalFreed() const {
    return m_DefaultAllocator.GetTotalFreed();
}

std::size_t Allocator::GetPeakMemoryUsage() const {
    return m_DefaultAllocator.GetPeakMemoryUsage();
}

void Allocator::ReportMemoryUsage() const {
    m_DefaultAllocator.ReportMemoryUsage();
}

std::size_t* Allocator::FindAllocation(void* ptr) {
    std::lock_guard<std::mutex> lock(m_AllocationMutex);
    auto it = m_AllocationTracker.find(ptr);
    if (it != m_AllocationTracker.end()) {
        return &(it->second.size);
    }
    return nullptr;
}

std::size_t Allocator::GetAllocationCount() const {
    std::lock_guard<std::mutex> lock(m_AllocationMutex);
    return m_AllocationTracker.size();
}

bool Allocator::IsEmpty() const {
    std::lock_guard<std::mutex> lock(m_AllocationMutex);
    return m_AllocationTracker.empty();
}

void Allocator::ClearAllocationMap() {
    std::lock_guard<std::mutex> lock(m_AllocationMutex);
    m_AllocationTracker.clear();
    m_AllocationMap.clear();
    m_DeallocatedPointers.clear();
    AllocityThread::ClearThreadLocalStorage();
}

void Allocator::ClearSmallObjectFreeLists() {
    m_DefaultAllocator.ClearSmallObjectFreeLists();
    std::lock_guard<std::mutex> lock(m_AllocationMutex);
    m_DeallocatedPointers.clear();
    AllocityThread::ClearThreadLocalStorage();
    for (auto& pool : m_MemoryPools) {
        pool->Clear();
    }
}

void Allocator::SetEnableDoubleFreeCheck(bool enable) {
    m_DefaultAllocator.SetEnableDoubleFreeCheck(enable);
}

void Allocator::SetDebugMode(bool enable) {
    m_debugMode = enable;
}

void Allocator::FinalCleanup() {
    m_StopThreads = true;
    m_ThreadPoolCondition.notify_all();
    for (auto& thread : m_ThreadPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    ClearAllocationMap();
    ClearSmallObjectFreeLists();
    m_DefaultAllocator.ClearSmallObjectFreeLists();
}

void Allocator::TrackAllocation(void* ptr, std::size_t size, bool isPoolAllocation) {
    m_AllocationTracker[ptr] = {size, isPoolAllocation};
    m_AllocationMap.insert(ptr, size);
    m_DeallocatedPointers.erase(ptr);
    AllocityThread::GetRecentAllocations()[ptr] = size;
    AllocityThread::GetRecentDeallocations().erase(ptr);
}

void Allocator::UntrackAllocation(void* ptr) {
    m_AllocationTracker.erase(ptr);
    m_AllocationMap.remove(ptr);
    m_DeallocatedPointers.insert(ptr);
    AllocityThread::GetRecentDeallocations().insert(ptr);
    AllocityThread::GetRecentAllocations().erase(ptr);
}

} 
