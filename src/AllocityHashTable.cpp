#include "../include/AllocityHashtable.hpp"
#include <algorithm>
#include <cstdint>

namespace allocity {

AllocityHashtable::AllocityHashtable(std::size_t initialCapacity)
    : m_entries(initialCapacity), m_size(0), m_capacity(initialCapacity) {}

AllocityHashtable::~AllocityHashtable() = default;

void AllocityHashtable::insert(void* key, std::size_t value) {
    if (m_size >= m_capacity * 0.75) {
        rehash();
    }

    std::size_t index = hash(key) & (m_capacity - 1);
    while (m_entries[index].occupied && m_entries[index].key != key) {
        index = (index + 1) & (m_capacity - 1);
    }

    if (!m_entries[index].occupied) {
        ++m_size;
    }

    m_entries[index] = {key, value, true};
}

bool AllocityHashtable::remove(void* key) {
    std::size_t index = hash(key) & (m_capacity - 1);
    while (m_entries[index].occupied) {
        if (m_entries[index].key == key) {
            m_entries[index].occupied = false;
            --m_size;
            return true;
        }
        index = (index + 1) & (m_capacity - 1);
    }
    return false;
}

std::size_t* AllocityHashtable::find(void* key) {
    std::size_t index = hash(key) & (m_capacity - 1);
    while (m_entries[index].occupied) {
        if (m_entries[index].key == key) {
            return &m_entries[index].value;
        }
        index = (index + 1) & (m_capacity - 1);
    }
    return nullptr;
}

void AllocityHashtable::clear() {
    std::fill(m_entries.begin(), m_entries.end(), Entry{nullptr, 0, false});
    m_size = 0;
}

std::size_t AllocityHashtable::hash(void* key) {
    
    std::uint64_t h = 14695981039346656037ULL;
    std::uintptr_t k = reinterpret_cast<std::uintptr_t>(key);
    for (std::size_t i = 0; i < sizeof(void*); ++i) {
        h ^= (k >> (i * 8)) & 0xFF;
        h *= 1099511628211ULL;
    }
    
    return static_cast<std::size_t>((h >> 32) ^ h);
}


void AllocityHashtable::rehash() {
    m_capacity *= 2;
    std::vector<Entry> oldEntries = std::move(m_entries);
    m_entries.resize(m_capacity);
    m_size = 0;

    for (const auto& entry : oldEntries) {
        if (entry.occupied) {
            insert(entry.key, entry.value);
        }
    }
}

} 