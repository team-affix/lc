#ifndef RC_POOL_HPP
#define RC_POOL_HPP

#include "debug_assert.hpp"
#include <deque>
#include <memory>
#include <utility>
#include <vector>

template <typename T> struct rc_pool {
    rc_pool();
    std::shared_ptr<T> make(T value);
    void release(T* p);

  private:
    T* take_slot();

    std::deque<T> storage_;
    std::vector<T*> free_;
    std::vector<T*> to_release_;
    bool releasing_;
};

template <typename T> struct rc_pool_deleter {
    rc_pool<T>* pool;

    void operator()(T* p) const {
        DEBUG_ASSERT(pool != nullptr);
        DEBUG_ASSERT(p != nullptr);
        pool->release(p);
    }
};

template <typename T>
rc_pool<T>::rc_pool() : storage_(), free_(), to_release_(), releasing_(false) {
}

template <typename T> std::shared_ptr<T> rc_pool<T>::make(T value) {
    T* p = take_slot();
    *p = std::move(value);
    return std::shared_ptr<T>(p, rc_pool_deleter<T>{this});
}

template <typename T> void rc_pool<T>::release(T* p) {
    DEBUG_ASSERT(p != nullptr);
    to_release_.push_back(p);
    if(releasing_)
        return;
    releasing_ = true;
    while(!to_release_.empty()) {
        T* q = to_release_.back();
        to_release_.pop_back();
        *q = T{};
        free_.push_back(q);
    }
    releasing_ = false;
}

template <typename T> T* rc_pool<T>::take_slot() {
    if(free_.empty()) {
        storage_.push_back(T{});
        return &storage_.back();
    }
    T* p = free_.back();
    free_.pop_back();
    return p;
}

#endif
