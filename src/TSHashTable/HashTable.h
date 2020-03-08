//
// Created by dmitri on 08.03.2020.
//

#ifndef HUFFMANCODE_HASHTABLE_H
#define HUFFMANCODE_HASHTABLE_H

#include <vector>
#include <future>
#include <unordered_map>

template <class Key, class Value, class Hasher = std::hash<Key>>
class ConcurrentHashTable {
    struct ConcurrentPair {
        ConcurrentPair():
                first(),
                second() {}
        mutable std::mutex first;
        std::unordered_map<Key, Value, Hasher> second;
    };
    struct Access {
        Access(const Key& key, ConcurrentPair& bucket):
            locker(bucket.first),
            ref_to_value(bucket.second[key]) {}
        std::lock_guard<std::mutex> locker;
        Value& ref_to_value;
    };
private:
    std::vector<ConcurrentPair> buckets_;
    Hasher hash;

    std::size_t GetIndex(const Key& key) const { return hash(key) % buckets_.size(); }
public:
    explicit ConcurrentHashTable(std::size_t buckets_number = 100):
        buckets_(buckets_number), hash() {}
    Access operator[](const Key& key) { return {key, buckets_[GetIndex(key)]}; }
    std::unordered_map<Key, Value, Hasher> GetDump() {
        std::unordered_map<Key, Value, Hasher> result;
        for (auto& [m, bucket]: buckets_) {
            auto locker = std::lock_guard<std::mutex>(m);
            result.insert(bucket.begin(), bucket.end());
        }
        return result;
    }
    std::vector<std::pair<Key, Value>> GetDumpVec() const {
        std::vector<std::pair<Key, Value>> result;
        for (auto& [m, bucket]: buckets_) {
            auto locker = std::lock_guard<std::mutex>(m);
            result.insert(result.end(), bucket.begin(), bucket.end());
        }
        return result;
    }
};

#endif //HUFFMANCODE_HASHTABLE_H
