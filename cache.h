#pragma once
#include <list>
#include <memory>
#include <unordered_map>

// range LRU cache implementation
// We need to implement it ourselves since it's not a standard template.
// N.B.: The type of the map value is the iterator for the keys list so
// that we can efficiently make it the _most_ recently used in the keys list
// via splice() rather than searching the list for the key. That then 
// requires the map value type to have both the key and the actual value
// because we're using it in both structures.
        
template <typename Key, typename Value>
class Cache 
{
    public:
        explicit Cache(size_t maxSize = 16 * 1024 * 1000, size_t elasticity = 10)
            : _maxSize(maxSize), _elasticity(elasticity) {}
        virtual ~Cache() = default;
        
        bool tryGet(const Key& k, Value& out)
        {
            const auto iter = range_map.find(k);
            if (iter == range_map.end()) {
                return false;
            }
            range_list.splice(range_list.begin(), range_list, iter->second);
            out = iter->second->second;
            return true;
        }
        void put(const Key& k, const Value& v)
        {
            const auto iter = range_map.find(k);
            if (iter != range_map.end()) {
                iter->second->second = v;
                range_list.splice(range_list.begin(), range_list, iter->second);
                return;
            }
            range_list.emplace_front(k, v);
            range_map[k] = range_list.begin();
            evict();
        }
        void evict(void)
        {
            size_t maxAllowed = _maxSize + _elasticity;
            if (_maxSize == 0 || range_map.size() <= maxAllowed) {
                return;
            }
            while (range_map.size() > _maxSize) {
                range_map.erase(range_list.back().first);
                range_list.pop_back();
            }
        }
    private:
        Cache(const Cache&) = delete;
        Cache& operator=(const Cache&) = delete;

        using list_type = std::list<std::pair<Key, Value>>;
        list_type range_list;
        std::unordered_map<Key, typename list_type::iterator> range_map;
        size_t _maxSize;
        size_t _elasticity;
};