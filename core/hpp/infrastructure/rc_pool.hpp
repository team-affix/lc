#ifndef RC_POOL_HPP
#define RC_POOL_HPP

#include <cstddef>
#include <deque>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

template <typename T> struct rc_pool {
    rc_pool();
    std::shared_ptr<T> alloc(T value);
    void release(std::optional<T>& slot);
    std::size_t space_usage() const;

  private:
    void try_collect_loop();
    std::optional<T>* take_slot();

    std::vector<std::optional<T>*> pending_collection_;
    std::vector<std::optional<T>*> free_;
    std::deque<std::optional<T>> storage_;
    bool collecting_;
};

template <typename T> struct rc_pool_deleter {
    rc_pool<T>& pool;
    std::optional<T>& slot;

    void operator()(T*) const {
        pool.release(slot);
    }
};

template <typename T>
rc_pool<T>::rc_pool()
    : pending_collection_(), free_(), storage_(), collecting_(false) {
}

template <typename T> std::shared_ptr<T> rc_pool<T>::alloc(T value) {
    std::optional<T>* slot = take_slot();
    slot->emplace(std::move(value));
    return std::shared_ptr<T>(&slot->value(), rc_pool_deleter<T>{*this, *slot});
}

template <typename T> void rc_pool<T>::release(std::optional<T>& slot) {
    pending_collection_.push_back(&slot);
    try_collect_loop();
}

template <typename T> void rc_pool<T>::try_collect_loop() {
    if(collecting_)
        return;
    collecting_ = true;
    while(!pending_collection_.empty()) {
        std::optional<T>* slot = pending_collection_.back();
        pending_collection_.pop_back();
        slot->reset();
        free_.push_back(slot);
    }
    collecting_ = false;
}

template <typename T> std::size_t rc_pool<T>::space_usage() const {
    return storage_.size() * sizeof(std::optional<T>);
}

template <typename T> std::optional<T>* rc_pool<T>::take_slot() {
    if(free_.empty()) {
        storage_.emplace_back(std::nullopt);
        return &storage_.back();
    }
    std::optional<T>* slot = free_.back();
    free_.pop_back();
    return slot;
}

#endif
