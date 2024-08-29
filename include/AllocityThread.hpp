#pragma once

#include <unordered_map>
#include <unordered_set>
#include <cstddef>

namespace allocity {

class AllocityThread {
public:
    static std::unordered_map<void*, std::size_t>& GetRecentAllocations();
    static std::unordered_set<void*>& GetRecentDeallocations();
    static void ClearThreadLocalStorage();

private:
    thread_local static std::unordered_map<void*, std::size_t> t_recentAllocations;
    thread_local static std::unordered_set<void*> t_recentDeallocations;
};

} 
