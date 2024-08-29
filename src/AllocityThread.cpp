#include "../include/AllocityThread.hpp"

namespace allocity {

thread_local std::unordered_map<void*, std::size_t> AllocityThread::t_recentAllocations;
thread_local std::unordered_set<void*> AllocityThread::t_recentDeallocations;

std::unordered_map<void*, std::size_t>& AllocityThread::GetRecentAllocations() {
    return t_recentAllocations;
}

std::unordered_set<void*>& AllocityThread::GetRecentDeallocations() {
    return t_recentDeallocations;
}

void AllocityThread::ClearThreadLocalStorage() {
    t_recentAllocations.clear();
    t_recentDeallocations.clear();
}

} 
