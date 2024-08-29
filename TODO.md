# TODO List for Allocity.

## Adding CPP Files for Existing HPP Files

1. Allocator.cpp
   - Priority: High
   - Task: Create Allocator.cpp to implement Allocator.hpp
   - Rationale: Separate interface from implementation
   - Details:
     - Implement all methods declared in Allocator.hpp
     - Include proper error handling and memory management
     - Ensure thread-safety for multi-threaded environments
     - Implement any performance optimisations specific to allocation strategies

2. Blocks.cpp
   - Priority: High
   - Task: Create Blocks.cpp to implement Blocks.hpp
   - Rationale: Implement methods for managing memory blocks
   - Details:
     - Develop efficient algorithms for block allocation and deallocation
     - Implement block splitting and merging functionalities
     - Ensure proper handling of block metadata
     - Optimise for both speed and memory usage

3. DefaultAllocator.cpp
   - Priority: High
   - Task: Create DefaultAllocator.cpp to implement DefaultAllocator.hpp
   - Rationale: Provide default allocation strategies
   - Details:
     - Implement standard memory allocation algorithms (e.g., first-fit, best-fit)
     - Ensure compatibility with various memory layouts
     - Implement fallback strategies for when primary allocation fails
     - Add debugging features such as memory leak detection

4. MemoryLayout.cpp
   - Priority: Medium
   - Task: Create MemoryLayout.cpp to implement MemoryLayout.hpp
   - Rationale: Implement memory layout management functions
   - Details:
     - Develop functions for creating and modifying memory layouts
     - Implement algorithms for optimising memory layout based on usage patterns
     - Ensure proper alignment and padding of memory structures
     - Add serialisation and deserialisation of memory layouts for persistence

5. MemoryManager.cpp
   - Priority: High
   - Task: Create MemoryManager.cpp to implement MemoryManager.hpp
   - Rationale: Implement overall memory management logic
   - Details:
     - Develop the core logic for managing multiple allocators and memory layouts
     - Implement strategies for balancing memory usage across different allocators
     - Add functionality for memory defragmentation and compaction
     - Implement logging and reporting features for memory usage statistics

6. StandardBlock.cpp
   - Priority: Medium
   - Task: Create StandardBlock.cpp to implement StandardBlock.hpp
   - Rationale: Implement standard block operations
   - Details:
     - Develop functions for initialising, accessing, and modifying standard blocks
     - Implement memory pooling for frequently used block sizes
     - Add support for custom block attributes and metadata
     - Ensure proper cleanup and deallocation of blocks

7. VariadicLayout.cpp
   - Priority: Medium
   - Task: Create VariadicLayout.cpp to implement VariadicLayout.hpp
   - Rationale: Implement variadic layout functionality
   - Details:
     - Develop template metaprogramming techniques for handling variadic layouts
     - Implement compile-time layout calculations where possible
     - Add runtime support for dynamic layout modifications
     - Ensure type safety and proper alignment for variadic structures

## StandardBlock Enhancements

1. Creation Time Tracking
   - Feature: Add CreationTime member
   - Type: std::chrono::steady_clock::time_point
   - Priority: Medium
   - Rationale: Track when each block was created for debugging and optimisation

2. Last Access Time Tracking
   - Feature: Add LastAccessTime member
   - Type: std::chrono::steady_clock::time_point
   - Priority: Medium
   - Rationale: Monitor usage patterns and identify rarely used blocks

3. Access Count Tracking
   - Feature: Add AccessCount member
   - Type: std::size_t
   - Priority: Low
   - Rationale: Measure frequency of block access for optimisation

4. Custom Destructor
   - Feature: Add CustomDestructor member
   - Type: std::function<void(void*)>
   - Priority: High
   - Rationale: Allow custom cleanup operations for complex data structures

5. Thread-safe Locking
   - Feature: Add IsLocked member
   - Type: std::atomic<bool>
   - Priority: High
   - Rationale: Enable thread-safe operations on blocks

6. Raw Data Storage
   - Feature: Add RawData member
   - Type: std::vector<std::byte>
   - Priority: Medium
   - Rationale: Store actual data within the block if needed

7. Metadata Storage
   - Feature: Add Metadata member
   - Type: std::any
   - Priority: Low
   - Rationale: Store additional type-erased information for flexibility

8. Custom Comparison
   - Feature: Add ComparisonFunction member
   - Type: std::function<bool(const StandardBlock&)>
   - Priority: Low
   - Rationale: Enable custom sorting and searching of blocks

## VariadicLayout / MemoryLayout Enhancements

1. Current Allocations Tracking
   - Feature: Add CurrentAllocations member
   - Type: std::size_t
   - Priority: High
   - Rationale: Track the current number of allocations for resource management

2. Allocation Size Tracking
   - Feature: Add AllocationSize member
   - Type: std::size_t
   - Priority: Medium
   - Rationale: Store the size of each allocation for better memory management

3. Resizable Flag
   - Feature: Add IsResizable member
   - Type: bool
   - Priority: Medium
   - Rationale: Indicate whether the layout can be resized dynamically

4. Custom Deallocator
   - Feature: Add Deallocator member
   - Type: std::function<void(void*)>
   - Priority: High
   - Rationale: Allow custom deallocation strategies

5. Last Access Time Tracking
   - Feature: Add LastAccessTime member
   - Type: std::chrono::steady_clock::time_point
   - Priority: Low
   - Rationale: Monitor usage patterns of the entire layout

6. Variable-sized Allocations
   - Feature: Add AllocationSizes member
   - Type: std::vector<std::size_t>
   - Priority: Medium
   - Rationale: Support allocations of different sizes within the same layout

7. Reference Counting
   - Feature: Add ReferenceCount member
   - Type: std::atomic<std::size_t>
   - Priority: High
   - Rationale: Enable shared layouts with proper resource management

8. Allocation Strategy
   - Feature: Add Strategy member
   - Type: std::unique_ptr<MemoryStrategy>
   - Priority: High
   - Rationale: Allow customisable allocation strategies (e.g., pool, stack, heap)

## General Allocator Enhancements

1. Total Allocated Memory Tracking
   - Feature: Add TotalAllocated member
   - Type: std::size_t
   - Priority: High
   - Rationale: Monitor overall memory usage

2. Total Freed Memory Tracking
   - Feature: Add TotalFreed member
   - Type: std::size_t
   - Priority: High
   - Rationale: Track memory deallocation for leak detection

3. Peak Memory Usage Tracking
   - Feature: Add PeakMemoryUsage member
   - Type: std::size_t
   - Priority: Medium
   - Rationale: Identify maximum memory consumption for optimisation

4. Custom Out-of-Memory Handler
   - Feature: Add OutOfMemoryHandler member
   - Type: std::function<void(std::size_t)>
   - Priority: High
   - Rationale: Provide custom error handling for out-of-memory situations

5. Aligned Memory Allocation
   - Feature: Add AlignedAllocate member
   - Type: std::function<void*(std::size_t, std::size_t)>
   - Priority: Medium
   - Rationale: Support aligned memory allocation for performance-critical operations

6. Aligned Memory Deallocation
   - Feature: Add AlignedDeallocate member
   - Type: std::function<void(void*)>
   - Priority: Medium
   - Rationale: Properly free aligned memory allocations

7. Allocation Tracking Map
   - Feature: Add AllocationMap member
   - Type: std::unordered_map<void*, std::size_t>
   - Priority: High
   - Rationale: Track individual allocations for debugging and leak detection

8. Memory Usage Reporting
   - Feature: Add MemoryUsageReporter member
   - Type: std::function<void(const DefaultAllocator&)>
   - Priority: Medium
   - Rationale: Provide customisable reporting of memory usage statistics