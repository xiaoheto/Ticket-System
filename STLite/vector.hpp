#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>

namespace sjtu {
    template<typename T>
    class vector {
    public:
        class const_iterator;

        class iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            T *ptr;
            vector<T> *container; //指向T类型数组的指针
        public:
            iterator(T *p, vector<T> *con): ptr(p), container(con) {
            }

            iterator operator+(const int &n) const {
                return iterator(ptr + n, container);
            }

            iterator operator-(const int &n) const {
                return iterator(ptr - n, container);
            }

            int operator-(const iterator &rhs) const {
                if (container != rhs.container) {
                    throw invalid_iterator();
                }
                return ptr - rhs.ptr;
            }

            iterator &operator+=(const int &n) {
                ptr += n;
                return *this;
            } //修改现有对象
            iterator &operator-=(const int &n) {
                ptr -= n;
                return *this;
            }

            iterator operator++(int) {
                iterator temp = *this;
                ptr += 1;
                return temp;
            }

            iterator &operator++() {
                ptr += 1;
                return *this;
            }


            iterator operator--(int) {
                iterator temp = *this;
                ptr -= 1;
                return temp;
            }


            iterator &operator--() {
                ptr -= 1;
                return *this;
            }


            T &operator*() const {
                return *ptr;
            }


            bool operator==(const iterator &rhs) const {
                return ptr == rhs.ptr;
            }


            bool operator!=(const iterator &rhs) const {
                return ptr != rhs.ptr;
            }
        };


        class const_iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T *;
            using reference = T &;
            using iterator_category = std::output_iterator_tag;

        private:
            T *ptr;
            const vector<T> *container; //指向T类型数组的指针
        public:
            const_iterator(T *p, const vector<T> *con): ptr(p), container(con) {
            }

            const_iterator operator+(const int &n) const {
                return const_iterator(ptr + n, container);
            }

            const_iterator operator-(const int &n) const {
                return const_iterator(ptr - n, container);
            }

            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw invaild_iterator.
            int operator-(const const_iterator &rhs) const {
                if (container != rhs.container) {
                    throw invalid_iterator();
                }
                return ptr - rhs.ptr;
            }

            const_iterator &operator+=(const int &n) {
                ptr += n;
                return *this;
            } //修改现有对象
            const_iterator &operator-=(const int &n) {
                ptr -= n;
                return *this;
            }

            const_iterator operator++(int) {
                const_iterator temp = *this;
                ptr += 1;
                return temp;
            }

            const_iterator &operator++() {
                ptr += 1;
                return *this;
            }

            const_iterator operator--(int) {
                const_iterator temp = *this;
                ptr -= 1;
                return temp;
            }

            const_iterator &operator--() {
                ptr -= 1;
                return *this;
            }

            T &operator*() const {
                return *ptr;
            }

            bool operator==(const const_iterator &rhs) const {
                return ptr == rhs.ptr;
            }

            bool operator!=(const const_iterator &rhs) const {
                return ptr != rhs.ptr;
            }
        };

    private:
        T *data; //T元素的动态数组
        size_t size_; //数组中元素个数
        size_t capacity; //数组的容量大小
        void resize(size_t new_capacity) {
            T *new_data = static_cast<T *>(::operator new(sizeof(T) * new_capacity));
            for (size_t i = 0; i < size_; ++i) {
                new(&new_data[i]) T(std::move(data[i]));
                data[i].~T();
            }
            ::operator delete(data);
            data = new_data;
            capacity = new_capacity;
        }

    public:
        vector(): data(nullptr), size_(0), capacity(0) {
        }

        vector(const vector &other) : data(nullptr), size_(other.size_), capacity(other.capacity) {
            data = static_cast<T *>(::operator new(sizeof(T) * capacity)); //分配内存
            for (size_t i = 0; i < size_; ++i) {
                new(&data[i]) T(other.data[i]);
            }
        }

        ~vector() {
            clear();
            ::operator delete(data);
        }

        vector &operator=(const vector &other) {
            if (this == &other) {
                return *this;
            }
            clear();
            if (capacity < other.size_) {
                resize(other.size_);
            }
            for (size_t i = 0; i < other.size_; ++i) {
                new(&data[i]) T(other.data[i]);
            }
            size_ = other.size_;
            return *this;
        }

        T &at(const size_t &pos) {
            if (pos > size_) {
                throw index_out_of_bound();
            }
            return data[pos];
        }

        const T &at(const size_t &pos) const {
            if (pos > size_) {
                throw index_out_of_bound();
            }
            return data[pos];
        }

        T &operator[](const size_t &pos) {
            if (pos < 0 || pos >= size_) {
                throw index_out_of_bound();
            }
            return data[pos];
        }

        const T &operator[](const size_t &pos) const {
            if (pos < 0 || pos >= size_) {
                throw index_out_of_bound();
            }
            return data[pos];
        }

        const T &front() const {
            if (empty()) {
                throw container_is_empty();
            }
            return data[0];
        }

        const T &back() const {
            if (empty()) {
                throw container_is_empty();
            }
            return data[size_ - 1];
        }

        iterator begin() {
            return iterator(data, this);
        }

        const_iterator cbegin() const {
            return const_iterator(data, this);
        }

        iterator end() {
            return iterator(data + size_, this);
        }

        const_iterator cend() const {
            return const_iterator(data + size_, this);
        }

        bool empty() const {
            return size_ == 0;
        }

        size_t size() const {
            return size_;
        }

        void clear() {
            for (size_t i = 0; i < size_; ++i) {
                data[i].~T();
            }
            size_ = 0;
        }

        iterator insert(iterator pos, const T &value) {
            size_t index = pos - begin();
            if (size_ == capacity) {
                resize(capacity == 0 ? 1 : capacity * 10);
            }
            for (size_t i = size_; i > index; --i) {
                new(&data[i]) T(std::move(data[i - 1]));
                data[i - 1].~T();
            }
            new(&data[index]) T(value);
            ++size_;
            return iterator(data + index, this);
        }

        iterator insert(const size_t &ind, const T &value) {
            if (ind > size_) {
                throw index_out_of_bound();
            }
            return insert(begin() + ind, value);
        }

        iterator erase(iterator pos) {
            size_t index = pos - begin();
            for (size_t i = index; i < size_ - 1; ++i) {
                data[i] = std::move(data[i + 1]);
            }
            data[size_ - 1].~T();
            --size_;
            return iterator(data + index, this);
        }

        iterator erase(const size_t &ind) {
            if (ind >= size_) {
                throw index_out_of_bound();
            }
            return Erase(begin() + ind);
        }

        void push_back(const T &value) {
            if (size_ == capacity) {
                resize(capacity == 0 ? 1 : capacity * 2);
            }
            new(&data[size_]) T(value);
            ++size_;
        }

        void pop_back() {
            if (empty()) {
                throw container_is_empty(); // 如果容器为空，抛出异常
            }
            data[size_ - 1].~T();
            --size_;
            if (size_ < capacity / 4) {
                resize(capacity / 2);
            }
        }
    };
}

#endif
