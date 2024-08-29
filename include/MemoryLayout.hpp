#pragma once

#include "VariadicLayout.hpp"
#include <string>

namespace allocity {

class MemoryLayout {
    
private:

    VariadicLayout m_VariadicLayout;

public:

    MemoryLayout() = default;

    explicit MemoryLayout(const VariadicLayout& layout) : m_VariadicLayout(layout) {}

    const VariadicLayout& GetVariadicLayout() const {

        return m_VariadicLayout;

    }

    void SetVariadicLayout(const VariadicLayout& variadic) {

        m_VariadicLayout = variadic;

    }

    std::string GetName() const { 

        return m_VariadicLayout.Name; 

    }

    void SetName(std::string name) { 
        m_VariadicLayout.Name = std::move(name); 

    }

    std::size_t GetIndice() const { 

        return m_VariadicLayout.indice; 

    }

    void SetIndice(std::size_t indice) { 

        m_VariadicLayout.indice = indice; 

    }

    std::size_t GetMaxAllocations() const { 

        return m_VariadicLayout.MaxAllocations; 

    }

    void SetMaxAllocations(std::size_t maxAllocations) {

        m_VariadicLayout.MaxAllocations = maxAllocations; 

    }

    std::size_t GetMinAllocations() const { 

        return m_VariadicLayout.MinAllocations; 

    }

    void SetMinAllocations(std::size_t minAllocations) {
        
        m_VariadicLayout.MinAllocations = minAllocations; 

    }

};

} 
