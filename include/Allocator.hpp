#pragma once

#include "DefaultAllocator.hpp"
#include "AllocityHashtable.hpp"
#include "MemoryPool.hpp"
#include <functional>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <thread>
#include <queue>
#include <atomic>
#include <unordered_map>

namespace allocity {

class Allocator {
private:
    DefaultAllocator m_DefaultAllocator;
    AllocityHashtable m_AllocationMap;
    mutable std::mutex m_AllocationMutex;
    std::unordered_set<void*> m_DeallocatedPointers;
    std::atomic<bool> m_debugMode;
    static constexpr unsigned char DEBUG_PATTERN = 0xFE;

    
    static constexpr size_t MAX_SMALL_OBJECT_SIZE = 256;
    static constexpr size_t NUM_MEMORY_POOLS = MAX_SMALL_OBJECT_SIZE / 8;
    std::vector<std::unique_ptr<MemoryPool>> m_MemoryPools;

    
    thread_local static std::unordered_map<void*, std::size_t> t_recentAllocations;
    thread_local static std::unordered_set<void*> t_recentDeallocations;

    
    std::vector<std::thread> m_ThreadPool;
    std::mutex m_ThreadPoolMutex;
    std::condition_variable m_ThreadPoolCondition;
    std::atomic<bool> m_StopThreads;
    std::queue<std::function<void()>> m_WorkQueue;

    
    struct AllocationInfo {
        std::size_t size;
        bool isPoolAllocation;
    };
    std::unordered_map<void*, AllocationInfo> m_AllocationTracker;
    mutable std::mutex m_AllocationTrackerMutex;

public:
    Allocator();
    ~Allocator();

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;
    Allocator(Allocator&&) = delete;
    Allocator& operator=(Allocator&&) = delete;

    const DefaultAllocator& GetDefaultAllocator() const;
    void SetDefaultAllocator(const DefaultAllocator& allocator);

    void* Allocate(std::size_t size);
    void Deallocate(void* ptr);
    void* Assign(void* ptr);
    void Deassign(void* ptr);

    void* AlignedAllocate(std::size_t size, std::size_t alignment);
    void AlignedDeallocate(void* ptr);

    void SetOutOfMemoryHandler(std::function<void(std::size_t)> handler);
    void SetMemoryUsageReporter(std::function<void(const DefaultAllocator&)> reporter);

    std::size_t GetTotalAllocated() const;
    std::size_t GetTotalFreed() const;
    std::size_t GetPeakMemoryUsage() const;

    void ReportMemoryUsage() const;

    std::size_t* FindAllocation(void* ptr);
    std::size_t GetAllocationCount() const;
    bool IsEmpty() const;
    void ClearAllocationMap();

    void ClearSmallObjectFreeLists();
    
    void SetEnableDoubleFreeCheck(bool enable);
    void SetDebugMode(bool enable);

    void FinalCleanup();

private:
    void InitializeMemoryPools();
    void InitializeThreadPool(size_t numThreads);
    void ThreadWorker();
    void* AllocateFromPool(std::size_t size);
    void DeallocateToPool(void* ptr, std::size_t size);
    void AddWorkToQueue(std::function<void()> work);
    bool IsPoolAllocation(std::size_t size) const;
    void TrackAllocation(void* ptr, std::size_t size, bool isPoolAllocation);
    void UntrackAllocation(void* ptr);
};

} 
