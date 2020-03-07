//
// Created by dmitri on 07.03.2020.
//

#ifndef HUFFMANCODE_WORKER_H
#define HUFFMANCODE_WORKER_H

#include <future>
#include <vector>

template <class Callback>
class WorkersPool {
    inline static const std::size_t MAX_WORKERS_NUMBER = 4;
private:
    std::vector<std::future<void>> workers_;
    Callback callback_;
public:
    WorkersPool(Callback callback):
        workers_(MAX_WORKERS_NUMBER),
        callback_(callback) {}

    template <typename T>
    void ApplyToValue(const T& value) const {
        bool send = false;
        while (!send) {
            for (size_t worker_ind = 0; worker_ind < workers_.size(); worker_ind++) {
            }
        }
    }
};

#endif //HUFFMANCODE_WORKER_H
