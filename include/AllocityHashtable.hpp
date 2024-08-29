#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace allocity {

class AllocityHashtable {
public:
    AllocityHashtable(std::size_t initialCapacity = 16);
    ~AllocityHashtable();

    void insert(void* key, std::size_t value);
    bool remove(void* key);
    std::size_t* find(void* key);
    void clear();

    std::size_t size() const { return m_size; }
    bool empty() const { return m_size == 0; }

private:
    struct Entry {
        void* key;
        std::size_t value;
        bool occupied;
    };

    std::vector<Entry> m_entries;
    std::size_t m_size;
    std::size_t m_capacity;

    static std::size_t hash(void* key);
    void rehash();
};

} 