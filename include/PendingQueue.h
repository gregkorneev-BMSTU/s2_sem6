#pragma once

#include <cstddef>
#include <memory>
#include <queue>
#include <tuple>
#include <type_traits>
#include <utility>

class IPending {
public:
    virtual ~IPending() = default;
    virtual void run() = 0;
};

template<typename Callable, typename... Args>
class Pending : public IPending {
public:
    Pending(Callable callable, Args... args)
        : callable_(std::move(callable)), args_(std::make_tuple(std::move(args)...)) {}

    void run() override {
        std::apply(callable_, args_);
    }

private:
    Callable callable_;
    std::tuple<Args...> args_;
};

class PendingQueue {
public:
    ~PendingQueue();

    template<typename Callable, typename... Args>
    void enqueue(Callable&& callable, Args&&... args) {
        using PendingType = Pending<std::decay_t<Callable>, std::decay_t<Args>...>;

        items_.emplace(std::make_unique<PendingType>(std::forward<Callable>(callable),
                                                     std::forward<Args>(args)...));
    }

    void run_one();
    void run_all();

    std::size_t size() const;

private:
    std::queue<std::unique_ptr<IPending>> items_;
};
