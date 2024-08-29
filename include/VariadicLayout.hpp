#pragma once

#include <string>
#include <cstddef>

namespace allocity {

struct VariadicLayout {

    std::string Name;
    std::size_t indice;
    std::size_t MaxAllocations;
    std::size_t MinAllocations;

    VariadicLayout() : Name(""), indice(0), MaxAllocations(0), MinAllocations(0) {}

    VariadicLayout(std::string name, 
                   std::size_t indice, 
                   std::size_t maxAllocations, 
                   std::size_t minAllocations)
        : Name(std::move(name)), 
          indice(indice), 
          MaxAllocations(maxAllocations), 
          MinAllocations(minAllocations) {}

    VariadicLayout(const VariadicLayout&) = default;
    VariadicLayout(VariadicLayout&&) = default;
    VariadicLayout& operator=(const VariadicLayout&) = default;
    VariadicLayout& operator=(VariadicLayout&&) = default;

};

} 
