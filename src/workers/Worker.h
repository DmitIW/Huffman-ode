//
// Created by dmitri on 07.03.2020.
//

#ifndef HUFFMANCODE_WORKER_H
#define HUFFMANCODE_WORKER_H

#include <future>
#include <list>
#include <functional>

#include "utility.hpp"

template <class Callback>
class WorkersPool {
private:
    std::list<std::future<void>> workers_;
    Callback callback_;
    const size_t workers_max_number_;
public:
    explicit WorkersPool(Callback callback, size_t workers_max_number = 4):
        workers_(),
        callback_(std::move(callback)),
        workers_max_number_(workers_max_number) {}
    void Wait() {
        for (const auto& worker: workers_)
            worker.wait();
        workers_.clear();
    }
    bool WaitFor(std::chrono::milliseconds duration) {
        for (auto it = workers_.begin(); it != workers_.end(); it = std::next(it))
            if (it->wait_for(duration) == std::future_status::ready) {
                workers_.erase(it);
                return true;
            }
        return false;
    }
    template <typename Arg>
    void Processing(Arg&& arg) {
        while (workers_.size() >= workers_max_number_)
            WaitFor(std::chrono::milliseconds(2));
        workers_.emplace_back(std::async([&](Arg&& arg){callback_(arg);}, arg));
    }
    const Callback& InnerProcessor() const {
        return callback_;
    }
};

#endif //HUFFMANCODE_WORKER_H
