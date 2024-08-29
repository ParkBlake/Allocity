#include "../include/DefaultAllocator.hpp"
#include <cstdlib>
#include <new>
#include <iostream>
#include <cstring>

#if defined(_MSC_VER)
    #include <malloc.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
    #include <sys/mman.h>
#else
    #include <cstdint>
    #include <memory>
#endif

namespace allocity {

DefaultAllocator::DefaultAllocator()
    : TotalAllocated(0), TotalFreed(0), PeakMemoryUsage(0), m_EnableDoubleFreeCheck(false) {
    Initialize();
}

DefaultAllocator::DefaultAllocator(const DefaultAllocator& other)
    : TotalAllocated(other.TotalAllocated.load()),
      TotalFreed(other.TotalFreed.load()),
      PeakMemoryUsage(other.PeakMemoryUsage.load()),
      OutOfMemoryHandler(other.OutOfMemoryHandler),
      MemoryUsageReporter(other.MemoryUsageReporter),
      m_EnableDoubleFreeCheck(other.m_EnableDoubleFreeCheck),
      m_FreedPointers(other.m_FreedPointers),
      m_AllocatedPointers(other.m_AllocatedPointers) {
    for (size_t i = 0; i < SMALL_OBJECT_THRESHOLD; ++i) {
        smallObjectFreeLists[i].store(other.smallObjectFreeLists[i].load());
    }
    if (other.Next) {
        Next = std::make_unique<DefaultAllocator>(*other.Next);
    }
}

DefaultAllocator::DefaultAllocator(DefaultAllocator&& other) noexcept
    : Next(std::move(other.Next)),
      TotalAllocated(other.TotalAllocated.load()),
      TotalFreed(other.TotalFreed.load()),
      PeakMemoryUsage(other.PeakMemoryUsage.load()),
      OutOfMemoryHandler(std::move(other.OutOfMemoryHandler)),
      MemoryUsageReporter(std::move(other.MemoryUsageReporter)),
      m_EnableDoubleFreeCheck(other.m_EnableDoubleFreeCheck),
      m_FreedPointers(std::move(other.m_FreedPointers)),
      m_AllocatedPointers(std::move(other.m_AllocatedPointers)) {
    for (size_t i = 0; i < SMALL_OBJECT_THRESHOLD; ++i) {
        smallObjectFreeLists[i].store(other.smallObjectFreeLists[i].load());
        other.smallObjectFreeLists[i].store(nullptr);
    }
}

DefaultAllocator& DefaultAllocator::operator=(const DefaultAllocator& other) {
    if (this != &other) {
        TotalAllocated.store(other.TotalAllocated.load());
        TotalFreed.store(other.TotalFreed.load());
        PeakMemoryUsage.store(other.PeakMemoryUsage.load());
        OutOfMemoryHandler = other.OutOfMemoryHandler;
        MemoryUsageReporter = other.MemoryUsageReporter;
        m_EnableDoubleFreeCheck = other.m_EnableDoubleFreeCheck;
        m_FreedPointers = other.m_FreedPointers;
        m_AllocatedPointers = other.m_AllocatedPointers;
        for (size_t i = 0; i < SMALL_OBJECT_THRESHOLD; ++i) {
            smallObjectFreeLists[i].store(other.smallObjectFreeLists[i].load());
        }
        if (other.Next) {
            Next = std::make_unique<DefaultAllocator>(*other.Next);
        } else {
            Next.reset();
        }
    }
    return *this;
}

DefaultAllocator& DefaultAllocator::operator=(DefaultAllocator&& other) noexcept {
    if (this != &other) {
        Next = std::move(other.Next);
        TotalAllocated.store(other.TotalAllocated.load());
        TotalFreed.store(other.TotalFreed.load());
        PeakMemoryUsage.store(other.PeakMemoryUsage.load());
        OutOfMemoryHandler = std::move(other.OutOfMemoryHandler);
        MemoryUsageReporter = std::move(other.MemoryUsageReporter);
        m_EnableDoubleFreeCheck = other.m_EnableDoubleFreeCheck;
        m_FreedPointers = std::move(other.m_FreedPointers);
        m_AllocatedPointers = std::move(other.m_AllocatedPointers);
        for (size_t i = 0; i < SMALL_OBJECT_THRESHOLD; ++i) {
            smallObjectFreeLists[i].store(other.smallObjectFreeLists[i].load());
            other.smallObjectFreeLists[i].store(nullptr);
        }
    }
    return *this;
}

DefaultAllocator::~DefaultAllocator() = default;

void DefaultAllocator::Initialize() {
    for (auto& freeList : smallObjectFreeLists) {
        freeList.store(nullptr, std::memory_order_relaxed);
    }

    OutOfMemoryHandler = [](std::size_t size) {
        std::cerr << "Out of memory! Failed to allocate " << size << " bytes." << std::endl;
    };

    MemoryUsageReporter = [](const DefaultAllocator& allocator) {
        std::cout << "Total Allocated: " << allocator.GetTotalAllocated() << " bytes" << std::endl;
        std::cout << "Total Freed: " << allocator.GetTotalFreed() << " bytes" << std::endl;
        std::cout << "Current Usage: " << (allocator.GetTotalAllocated() - allocator.GetTotalFreed()) << " bytes" << std::endl;
        std::cout << "Peak Usage: " << allocator.GetPeakMemoryUsage() << " bytes" << std::endl;
    };
}

void DefaultAllocator::Deallocate(void* ptr, std::size_t size) {
    if (ptr == nullptr) return;
    
    if (m_EnableDoubleFreeCheck) {
        std::lock_guard<std::mutex> lock(m_AllocationMutex);
        auto it = m_AllocatedPointers.find(ptr);
        if (it == m_AllocatedPointers.end()) {
            throw std::runtime_error("Double free or invalid free detected");
        }
        m_AllocatedPointers.erase(it);
    }
    
    if (size <= SMALL_OBJECT_THRESHOLD) {
        deallocateSmall(ptr, size);
    } else {
        std::free(ptr);
    }
    
    TotalFreed.fetch_add(size, std::memory_order_relaxed);
}

void* DefaultAllocator::Allocate(std::size_t size) {
    if (size == 0) size = 1;  
    void* ptr = nullptr;
    
    try {
        if (size <= SMALL_OBJECT_THRESHOLD) {
            ptr = allocateSmall(size);
        } else {
            ptr = std::malloc(size);
        }
        
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        
        TotalAllocated.fetch_add(size, std::memory_order_relaxed);
        UpdatePeakMemoryUsage();
        
        if (m_EnableDoubleFreeCheck) {
            std::lock_guard<std::mutex> lock(m_AllocationMutex);
            m_AllocatedPointers.insert(ptr);
        }
        
        return ptr;
    } catch (const std::bad_alloc&) {
        if (OutOfMemoryHandler) {
            OutOfMemoryHandler(size);
        }
        throw;
    }
}

void* DefaultAllocator::Assign(void* ptr) {
    return ptr;
}

void DefaultAllocator::Deassign(void* ptr) {
    (void)ptr; 
    
}

void* DefaultAllocator::AlignedAllocate(std::size_t size, std::size_t alignment) {
    void* ptr = nullptr;
    #if defined(_MSC_VER)
        ptr = _aligned_malloc(size, alignment);
    #elif defined(__APPLE__) || defined(__linux__)
        if (posix_memalign(&ptr, alignment, size) != 0) {
            ptr = nullptr;
        }
    #else
        std::size_t space = size + alignment - 1 + sizeof(void*);
        void* unaligned = std::malloc(space);
        if (unaligned != nullptr) {
            ptr = reinterpret_cast<void*>(
                (reinterpret_cast<std::uintptr_t>(unaligned) + sizeof(void*) + alignment - 1) & ~(alignment - 1)
            );
            reinterpret_cast<void**>(ptr)[-1] = unaligned;
        }
    #endif
    if (ptr == nullptr) {
        if (OutOfMemoryHandler) {
            OutOfMemoryHandler(size);
        }
        throw std::bad_alloc();
    }
    TotalAllocated.fetch_add(size, std::memory_order_relaxed);
    UpdatePeakMemoryUsage();
    return ptr;
}

void DefaultAllocator::AlignedDeallocate(void* ptr, std::size_t size) {
    if (ptr == nullptr) return;
    #if defined(_MSC_VER)
        _aligned_free(ptr);
    #elif defined(__APPLE__) || defined(__linux__)
        free(ptr);
    #else
        std::free(reinterpret_cast<void**>(ptr)[-1]);
    #endif
    TotalFreed.fetch_add(size, std::memory_order_relaxed);
}

void DefaultAllocator::ClearSmallObjectFreeLists() {
    for (auto& freeList : smallObjectFreeLists) {
        while (freeList != nullptr) {
            void* ptr = freeList;
            freeList = *reinterpret_cast<void**>(ptr);
            std::free(ptr);
        }
    }
    TotalFreed.store(TotalAllocated.load(std::memory_order_relaxed), std::memory_order_relaxed);
    if (m_EnableDoubleFreeCheck) {
        m_FreedPointers.clear();
    }
}

void DefaultAllocator::SetEnableDoubleFreeCheck(bool enable) {
    m_EnableDoubleFreeCheck = enable;
    if (!enable) {
        m_FreedPointers.clear();
    }
}

void DefaultAllocator::SetOutOfMemoryHandler(std::function<void(std::size_t)> handler) {
    OutOfMemoryHandler = std::move(handler);
}

void DefaultAllocator::SetMemoryUsageReporter(std::function<void(const DefaultAllocator&)> reporter) {
    MemoryUsageReporter = std::move(reporter);
}

std::size_t DefaultAllocator::GetTotalAllocated() const {
    return TotalAllocated.load(std::memory_order_relaxed);
}

std::size_t DefaultAllocator::GetTotalFreed() const {
    return TotalFreed.load(std::memory_order_relaxed);
}

std::size_t DefaultAllocator::GetPeakMemoryUsage() const {
    return PeakMemoryUsage.load(std::memory_order_relaxed);
}

void DefaultAllocator::ReportMemoryUsage() const {
    if (MemoryUsageReporter) {
        MemoryUsageReporter(*this);
    }
}

void* DefaultAllocator::allocateSmall(std::size_t size) {
    if (size == 0 || size > SMALL_OBJECT_THRESHOLD) return nullptr;
    
    void* ptr = smallObjectFreeLists[size].load(std::memory_order_relaxed);
    while (ptr != nullptr) {
        if (smallObjectFreeLists[size].compare_exchange_weak(ptr, *reinterpret_cast<void**>(ptr), std::memory_order_release,std::memory_order_relaxed)) {
            return ptr;
        }
    }

    return std::malloc(size);
}

void DefaultAllocator::deallocateSmall(void* ptr, std::size_t size) {
    if (size == 0 || size > SMALL_OBJECT_THRESHOLD) return;

    void* expected = smallObjectFreeLists[size].load(std::memory_order_relaxed);
    do {
        *reinterpret_cast<void**>(ptr) = expected;
    } while (!smallObjectFreeLists[size].compare_exchange_weak(expected, ptr ,std::memory_order_release,std::memory_order_relaxed));
}

void DefaultAllocator::UpdatePeakMemoryUsage() {
    std::size_t currentUsage = TotalAllocated.load(std::memory_order_relaxed) - TotalFreed.load(std::memory_order_relaxed);
    std::size_t peakUsage = PeakMemoryUsage.load(std::memory_order_relaxed);
    while (currentUsage > peakUsage) {
        if (PeakMemoryUsage.compare_exchange_weak(peakUsage, currentUsage, std::memory_order_relaxed)) {
            break;
        }
    }
}

} 
