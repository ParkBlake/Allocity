#pragma once

#include <functional>
#include <memory>
#include <atomic>
#include <cstdint>
#include <array>
#include <mutex>
#include <unordered_set>

namespace allocity {

class DefaultAllocator {

public:
    static constexpr std::size_t SMALL_OBJECT_THRESHOLD = 256;

    DefaultAllocator();
    DefaultAllocator(const DefaultAllocator& other);
    DefaultAllocator(DefaultAllocator&& other) noexcept;
    DefaultAllocator& operator=(const DefaultAllocator& other);
    DefaultAllocator& operator=(DefaultAllocator&& other) noexcept;
    ~DefaultAllocator();

    void* Allocate(std::size_t size);
    void Deallocate(void* ptr, std::size_t size);
    void* Assign(void* ptr);
    void Deassign(void* ptr);
    void* AlignedAllocate(std::size_t size, std::size_t alignment);
    void AlignedDeallocate(void* ptr, std::size_t size);

    void Initialize();
    void ClearSmallObjectFreeLists();
    void SetEnableDoubleFreeCheck(bool enable);
    void SetOutOfMemoryHandler(std::function<void(std::size_t)> handler);
    void SetMemoryUsageReporter(std::function<void(const DefaultAllocator&)> reporter);

    std::size_t GetTotalAllocated() const;
    std::size_t GetTotalFreed() const;
    std::size_t GetPeakMemoryUsage() const;
    void ReportMemoryUsage() const;

private:
    void* allocateSmall(std::size_t size);
    void deallocateSmall(void* ptr, std::size_t size);
    void UpdatePeakMemoryUsage();

    std::unique_ptr<DefaultAllocator> Next;
    std::atomic<std::size_t> TotalAllocated;
    std::atomic<std::size_t> TotalFreed;
    std::atomic<std::size_t> PeakMemoryUsage;
    std::function<void(std::size_t)> OutOfMemoryHandler;
    std::function<void(const DefaultAllocator&)> MemoryUsageReporter;
    std::array<std::atomic<void*>, SMALL_OBJECT_THRESHOLD> smallObjectFreeLists;
    bool m_EnableDoubleFreeCheck;
    std::unordered_set<void*> m_FreedPointers;
    std::unordered_set<void*> m_AllocatedPointers;
    mutable std::mutex m_AllocationMutex;
};

} 
