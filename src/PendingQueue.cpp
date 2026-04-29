#include "PendingQueue.h"

#include <cstdio>

PendingQueue::~PendingQueue() {
    std::printf("%ld pending items dropped\n", static_cast<long>(size()));
}

void PendingQueue::run_one() {
    if (items_.empty()) {
        return;
    }

    items_.front()->run();
    items_.pop();
}

void PendingQueue::run_all() {
    while (!items_.empty()) {
        run_one();
    }
}

std::size_t PendingQueue::size() const {
    return items_.size();
}
