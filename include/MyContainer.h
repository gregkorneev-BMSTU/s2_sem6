#pragma once

#include <cstddef>
#include <iterator>
#include <new>
#include <type_traits>
#include <utility>

template<typename T>
class MyContainer {
private:
    template<typename ValueType>
    class BasicIterator {
        template<typename>
        friend class BasicIterator;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = typename std::remove_const<ValueType>::type;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        BasicIterator() : ptr_(nullptr) {}
        explicit BasicIterator(pointer ptr) : ptr_(ptr) {}

        reference operator*() const {
            return *ptr_;
        }

        pointer operator->() const {
            return ptr_;
        }

        BasicIterator& operator++() {
            ++ptr_;
            return *this;
        }

        BasicIterator operator++(int) {
            BasicIterator temp(*this);
            ++(*this);
            return temp;
        }

        BasicIterator& operator--() {
            --ptr_;
            return *this;
        }

        BasicIterator operator--(int) {
            BasicIterator temp(*this);
            --(*this);
            return temp;
        }

        BasicIterator& operator+=(difference_type offset) {
            ptr_ += offset;
            return *this;
        }

        BasicIterator& operator-=(difference_type offset) {
            ptr_ -= offset;
            return *this;
        }

        BasicIterator operator+(difference_type offset) const {
            return BasicIterator(ptr_ + offset);
        }

        BasicIterator operator-(difference_type offset) const {
            return BasicIterator(ptr_ - offset);
        }

        difference_type operator-(const BasicIterator& other) const {
            return ptr_ - other.ptr_;
        }

        reference operator[](difference_type offset) const {
            return *(ptr_ + offset);
        }

        bool operator==(const BasicIterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const BasicIterator& other) const {
            return ptr_ != other.ptr_;
        }

        bool operator<(const BasicIterator& other) const {
            return ptr_ < other.ptr_;
        }

        bool operator>(const BasicIterator& other) const {
            return ptr_ > other.ptr_;
        }

        bool operator<=(const BasicIterator& other) const {
            return ptr_ <= other.ptr_;
        }

        bool operator>=(const BasicIterator& other) const {
            return ptr_ >= other.ptr_;
        }

        friend BasicIterator operator+(difference_type offset, const BasicIterator& it) {
            return BasicIterator(it.ptr_ + offset);
        }

    private:
        pointer ptr_;
    };

public:
    using Iterator = BasicIterator<T>;
    using ConstIterator = BasicIterator<const T>;

    MyContainer() : data_(nullptr), size_(0), capacity_(0) {}

    MyContainer(const MyContainer& other) : data_(nullptr), size_(0), capacity_(0) {
        reserve(other.size_);
        for (std::size_t i = 0; i < other.size_; ++i) {
            push_back(other.data_[i]);
        }
    }

    MyContainer(MyContainer&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    MyContainer& operator=(const MyContainer& other) {
        if (this == &other) {
            return *this;
        }

        clear();
        reserve(other.size_);
        for (std::size_t i = 0; i < other.size_; ++i) {
            push_back(other.data_[i]);
        }
        return *this;
    }

    MyContainer& operator=(MyContainer&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        clear();
        ::operator delete(data_);

        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    ~MyContainer() {
        clear();
        ::operator delete(data_);
    }

    void push_back(const T& value) {
        ensureCapacity();
        new (data_ + size_) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        ensureCapacity();
        new (data_ + size_) T(std::move(value));
        ++size_;
    }

    template<typename... Args>
    T& emplace_back(Args&&... args) {
        ensureCapacity();
        new (data_ + size_) T(std::forward<Args>(args)...);
        ++size_;
        return data_[size_ - 1];
    }

    void pop_back() {
        if (size_ == 0) {
            return;
        }

        data_[size_ - 1].~T();
        --size_;
    }

    bool removeAt(std::size_t index) {
        if (index >= size_) {
            return false;
        }

        for (std::size_t i = index; i + 1 < size_; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }

        pop_back();
        return true;
    }

    Iterator erase(Iterator position) {
        std::size_t index = static_cast<std::size_t>(position - begin());
        if (index >= size_) {
            return end();
        }

        removeAt(index);
        if (index >= size_) {
            return end();
        }
        return Iterator(data_ + index);
    }

    void clear() {
        while (size_ > 0) {
            pop_back();
        }
    }

    void reserve(std::size_t newCapacity) {
        if (newCapacity <= capacity_) {
            return;
        }

        T* newData = static_cast<T*>(::operator new(sizeof(T) * newCapacity));

        for (std::size_t i = 0; i < size_; ++i) {
            new (newData + i) T(std::move(data_[i]));
            data_[i].~T();
        }

        ::operator delete(data_);
        data_ = newData;
        capacity_ = newCapacity;
    }

    std::size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    T& operator[](std::size_t index) {
        return data_[index];
    }

    const T& operator[](std::size_t index) const {
        return data_[index];
    }

    Iterator find(const T& value) {
        for (Iterator it = begin(); it != end(); ++it) {
            if (*it == value) {
                return it;
            }
        }
        return end();
    }

    ConstIterator find(const T& value) const {
        for (ConstIterator it = begin(); it != end(); ++it) {
            if (*it == value) {
                return it;
            }
        }
        return end();
    }

    Iterator begin() {
        return Iterator(data_);
    }

    Iterator end() {
        return Iterator(data_ == nullptr ? nullptr : data_ + size_);
    }

    ConstIterator begin() const {
        return ConstIterator(data_);
    }

    ConstIterator end() const {
        return ConstIterator(data_ == nullptr ? nullptr : data_ + size_);
    }

    ConstIterator cbegin() const {
        return begin();
    }

    ConstIterator cend() const {
        return end();
    }

    void swap(MyContainer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

private:
    void ensureCapacity() {
        if (size_ < capacity_) {
            return;
        }

        std::size_t newCapacity = capacity_ == 0 ? 4 : capacity_ * 2;
        reserve(newCapacity);
    }

    T* data_;
    std::size_t size_;
    std::size_t capacity_;
};
