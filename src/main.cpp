#include "../include/Allocator.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <thread>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include <string>

void printMemoryUsage(const allocity::Allocator& allocator) {
    std::cout << "Attempting to print memory usage...\n";
    try {
        allocator.ReportMemoryUsage();
    } catch (const std::exception& e) {
        std::cout << "Exception caught while reporting memory usage: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception caught while reporting memory usage\n";
    }
    std::cout << "Memory usage report completed.\n";
}

void testSimpleAllocation(allocity::Allocator& allocator) {
    std::cout << "Testing simple allocation...\n";
    try {
        void* ptr = allocator.Allocate(10);
        std::cout << "Allocated 10 bytes successfully.\n";
        allocator.Deallocate(ptr);
        std::cout << "Deallocated 10 bytes successfully.\n";
    } catch (const std::exception& e) {
        std::cout << "Exception caught during simple allocation test: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception caught during simple allocation test\n";
    }
    std::cout << "Simple allocation test completed.\n";
}

void edgeCaseTests(allocity::Allocator& allocator) {
    std::cout << "\n+------------------------------------+";
    std::cout << "\n|           Edge Case Tests          |";
    std::cout << "\n+------------------------------------+\n";
    
    try {
        std::cout << "\n* Allocating 0 bytes:\n";
        void* zeroPtr = allocator.Allocate(0);
        std::cout << "  Result: " << (zeroPtr ? "Unexpected non-null pointer" : "Null pointer (as expected)") << "\n";

        std::cout << "\n* Deallocating 0-byte allocation:\n";
        allocator.Deallocate(zeroPtr);
        std::cout << "  Result: Completed without error\n";

        std::cout << "\n* Allocating 1 byte:\n";
        void* oneBytePtr = allocator.Allocate(1);
        std::cout << "  Result: " << (oneBytePtr ? "Success" : "Failure") << "\n";
        allocator.Deallocate(oneBytePtr);
        std::cout << "  Deallocated successfully\n";

        std::cout << "\n* Allocating max size_t bytes:\n";
        try {
            void* maxPtr = allocator.Allocate(std::numeric_limits<size_t>::max());
            std::cout << "  Result: Unexpected success (this should not happen)\n";
            allocator.Deallocate(maxPtr);
        } catch (const std::exception& e) {
            std::cout << "  Result: Expected exception caught - " << e.what() << "\n";
        }

    } catch (const std::exception& e) {
        std::cout << "Unexpected exception in edge case tests: " << e.what() << "\n";
    } catch (...) {
        std::cout << "Unknown exception caught in edge case tests\n";
    }

    std::cout << "\nEdge case tests completed.\n";
}

void smallAllocationTest(allocity::Allocator& allocator) {
    std::cout << "\n+------------------------------------+";
    std::cout << "\n|        Small Allocation Test       |";
    std::cout << "\n+------------------------------------+\n";

    std::cout << std::setw(15) << "Size" 
              << std::setw(25) << "Custom Alloc (ns)" 
              << std::setw(25) << "Custom Dealloc (ns)"
              << std::setw(25) << "Standard Alloc (ns)" 
              << std::setw(25) << "Standard Dealloc (ns)" << std::endl;
    std::cout << std::string(115, '-') << std::endl;

    std::vector<size_t> sizes = {8, 16, 32, 64, 128, 256, 512, 1024, 4096, 16384, 65536, 262144, 1048576}; 

    for (size_t size : sizes) {
        
        auto start = std::chrono::high_resolution_clock::now();
        void* customPtr = allocator.Allocate(size);
        auto end = std::chrono::high_resolution_clock::now();
        auto customAllocTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        allocator.Deallocate(customPtr);
        end = std::chrono::high_resolution_clock::now();
        auto customDeallocTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        
        start = std::chrono::high_resolution_clock::now();
        void* standardPtr = malloc(size);
        end = std::chrono::high_resolution_clock::now();
        auto standardAllocTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        free(standardPtr);
        end = std::chrono::high_resolution_clock::now();
        auto standardDeallocTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        std::string sizeStr;
        if (size < 1024) {
            sizeStr = std::to_string(size) + " B";
        } else if (size < 1048576) {
            sizeStr = std::to_string(size / 1024) + " KB";
        } else {
            sizeStr = std::to_string(size / 1048576) + " MB";
        }

        std::cout << std::setw(15) << sizeStr
                  << std::setw(25) << customAllocTime 
                  << std::setw(25) << customDeallocTime
                  << std::setw(25) << standardAllocTime 
                  << std::setw(25) << standardDeallocTime << std::endl;
    }
}

void largeAllocationTest(allocity::Allocator& allocator) {
    std::cout << "\n+------------------------------------+";
    std::cout << "\n|        Large Allocation Test       |";
    std::cout << "\n+------------------------------------+\n";
    
    std::cout << std::setw(10) << "Size (GB)" << std::setw(20) << "Alloc Time (ms)" << std::setw(20) << "Dealloc Time (ms)" << std::endl;
    std::cout << std::string(50, '-') << std::endl;

    for (size_t gb = 1; gb <= 32; gb *= 2) {
        size_t size = gb * 1024 * 1024 * 1024ULL;

        auto start = std::chrono::high_resolution_clock::now();
        void* ptr = allocator.Allocate(size);
        auto end = std::chrono::high_resolution_clock::now();
        auto alloc_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        allocator.Deallocate(ptr);
        end = std::chrono::high_resolution_clock::now();
        auto dealloc_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << std::setw(10) << gb << std::setw(20) << alloc_time << std::setw(20) << dealloc_time << std::endl;
    }
}
void compareWithStandardAllocator() {
    std::cout << "\n+------------------------------------------------------------+";
    std::cout << "\n|     Comparison with Standard Allocator (malloc/free)       |";
    std::cout << "\n+------------------------------------------------------------+\n";

    std::cout << std::setw(10) << "Size (GB)" 
              << std::setw(25) << "Custom Alloc (us)" 
              << std::setw(25) << "Custom Dealloc (us)"
              << std::setw(25) << "Standard Alloc (us)" 
              << std::setw(25) << "Standard Dealloc (us)" << std::endl;
    std::cout << std::string(110, '-') << std::endl;

    allocity::Allocator customAllocator;

    for (size_t gb = 1; gb <= 16; gb *= 2) {
        size_t size = gb * 1024 * 1024 * 1024ULL;

        
        auto start = std::chrono::high_resolution_clock::now();
        void* customPtr = customAllocator.Allocate(size);
        auto end = std::chrono::high_resolution_clock::now();
        auto customAllocTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        customAllocator.Deallocate(customPtr);
        end = std::chrono::high_resolution_clock::now();
        auto customDeallocTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        
        start = std::chrono::high_resolution_clock::now();
        void* standardPtr = malloc(size);
        end = std::chrono::high_resolution_clock::now();
        auto standardAllocTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        free(standardPtr);
        end = std::chrono::high_resolution_clock::now();
        auto standardDeallocTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << std::setw(10) << gb 
                  << std::setw(25) << customAllocTime 
                  << std::setw(25) << customDeallocTime
                  << std::setw(25) << standardAllocTime 
                  << std::setw(25) << standardDeallocTime << std::endl;
    }
}

int main() {
    try {
        std::cout << "+------------------------------------+\n";
        std::cout << "|        Allocator Test Suite        |\n";
        std::cout << "+------------------------------------+\n\n";

        std::cout << "Initializing Allocator...\n";
        allocity::Allocator allocator;
        allocator.SetEnableDoubleFreeCheck(true);
        allocator.SetDebugMode(true);
        std::cout << "Allocator initialized with double free checking and debug mode enabled.\n\n";

        std::cout << "Running tests:\n";
        std::cout << "1. Simple Allocation Test\n";
        testSimpleAllocation(allocator);

        std::cout << "\n2. Edge Case Tests\n";
        edgeCaseTests(allocator);

        std::cout << "\n3. Small Allocation Test\n";
        smallAllocationTest(allocator);

        std::cout << "\n4. Large Allocation Test\n";
        largeAllocationTest(allocator);

        std::cout << "\n5. Comparison with Standard Allocator (Large Allocations)\n";
        compareWithStandardAllocator();

        std::cout << "\n+------------------------------------+\n";
        std::cout << "|        All tests completed          |\n";
        std::cout << "+------------------------------------+\n";
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception in main: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception caught in main\n";
    }

    std::cout << "\nProgram finished. Press Enter to exit.\n";
    std::cin.get();

    return 0;
}
