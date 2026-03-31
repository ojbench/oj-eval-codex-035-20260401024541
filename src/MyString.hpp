#ifndef SIMPLE_STRING_SIMPLESTRING_HPP
#define SIMPLE_STRING_SIMPLESTRING_HPP

#include <stdexcept>
#include <cstring>

class MyString {
private:
    union {
        char* heap_ptr;
        char small_buffer[16];
    };
    size_t len{};
    size_t cap{}; // 0 => SSO, otherwise heap capacity excluding null terminator

    bool is_sso() const { return cap == 0; }

    char* data_ptr() { return is_sso() ? const_cast<char*>(small_buffer) : heap_ptr; }
    const char* data_ptr() const { return is_sso() ? small_buffer : heap_ptr; }

    void allocate_heap(size_t new_cap) {
        // allocate new_cap + 1 bytes
        char* p = new char[new_cap + 1];
        p[0] = '\0';
        heap_ptr = p;
        cap = new_cap;
    }

    void ensure_capacity(size_t need) {
        if (need <= 15 && is_sso()) return; // SSO suffices
        if (!is_sso() && need <= cap) return; // already sufficient on heap
        // grow strategy: max(need, max(16, cap*2))
        size_t new_cap = need;
        if (!is_sso()) {
            size_t grow = cap ? cap * 2 : 16;
            if (grow > new_cap) new_cap = grow;
        } else {
            if (new_cap < 16) new_cap = 16;
        }
        char* new_mem = new char[new_cap + 1];
        // copy existing data
        std::memcpy(new_mem, data_ptr(), len + 1);
        // release old if heap
        if (!is_sso()) delete[] heap_ptr;
        heap_ptr = new_mem;
        cap = new_cap; // switch to heap mode
        // After moving to heap, ensure flag behavior: len remains same.
    }

public:
    MyString() {
        len = 0;
        cap = 0;
        small_buffer[0] = '\0';
    }

    MyString(const char* s) {
        if (!s) {
            len = 0; cap = 0; small_buffer[0] = '\0';
            return;
        }
        size_t l = std::strlen(s);
        if (l <= 15) {
            len = l; cap = 0;
            std::memcpy(small_buffer, s, l + 1);
        } else {
            len = l;
            allocate_heap(l);
            std::memcpy(heap_ptr, s, l + 1);
        }
    }

    MyString(const MyString& other) {
        len = other.len;
        if (other.is_sso()) {
            cap = 0;
            std::memcpy(small_buffer, other.small_buffer, other.len + 1);
        } else {
            cap = other.cap;
            allocate_heap(cap);
            std::memcpy(heap_ptr, other.heap_ptr, other.len + 1);
        }
    }

    MyString(MyString&& other) noexcept {
        len = other.len;
        bool other_sso = other.is_sso();
        if (other_sso) {
            cap = 0;
            std::memcpy(small_buffer, other.small_buffer, other.len + 1);
            other.small_buffer[0] = '\0';
        } else {
            heap_ptr = other.heap_ptr;
            cap = other.cap;
            other.heap_ptr = nullptr;
        }
        other.len = 0;
        other.cap = 0;
    }

    MyString& operator=(MyString&& other) noexcept {
        if (this == &other) return *this;
        // clean up current
        if (!is_sso() && heap_ptr) delete[] heap_ptr;
        len = other.len;
        bool other_sso = other.is_sso();
        if (other_sso) {
            cap = 0;
            std::memcpy(small_buffer, other.small_buffer, other.len + 1);
            other.small_buffer[0] = '\0';
        } else {
            heap_ptr = other.heap_ptr;
            cap = other.cap;
            other.heap_ptr = nullptr;
        }
        other.len = 0;
        other.cap = 0;
        return *this;
    }

    MyString& operator=(const MyString& other) {
        if (this == &other) return *this;
        if (!is_sso()) {
            if (!other.is_sso() && other.len <= cap) {
                // reuse
                len = other.len;
                std::memcpy(heap_ptr, other.data_ptr(), len + 1);
                return *this;
            }
            delete[] heap_ptr;
        }
        len = other.len;
        if (other.is_sso()) {
            cap = 0;
            std::memcpy(small_buffer, other.small_buffer, other.len + 1);
        } else {
            allocate_heap(other.cap);
            std::memcpy(heap_ptr, other.heap_ptr, other.len + 1);
        }
        return *this;
    }

    ~MyString() {
        if (!is_sso() && heap_ptr) delete[] heap_ptr;
    }

    const char* c_str() const { return data_ptr(); }

    size_t size() const { return len; }

    size_t capacity() const { return is_sso() ? 15 : cap; }

    void reserve(size_t new_capacity) {
        // Per spec: when size <= 15 (SSO), capacity() should report 15.
        // So do nothing in SSO mode; grow only after exceeding SSO.
        if (is_sso()) return;
        if (new_capacity <= cap) return;
        ensure_capacity(new_capacity);
    }

    void resize(size_t new_size) {
        if (new_size <= 15) {
            if (!is_sso()) {
                // move to SSO
                size_t copy_n = new_size < len ? new_size : len;
                char tmp[16];
                if (copy_n) std::memcpy(tmp, heap_ptr, copy_n);
                delete[] heap_ptr;
                cap = 0;
                len = new_size;
                if (copy_n) std::memcpy(small_buffer, tmp, copy_n);
                small_buffer[len] = '\0';
            } else {
                if (new_size > len) {
                    std::memset(small_buffer + len, '\0', new_size - len);
                }
                len = new_size;
                small_buffer[len] = '\0';
            }
        } else {
            ensure_capacity(new_size);
            if (new_size > len) {
                std::memset(data_ptr() + len, '\0', new_size - len);
            }
            len = new_size;
            data_ptr()[len] = '\0';
        }
    }

    char& operator[](size_t index) {
        if (index >= len) throw std::out_of_range("index out of range");
        return data_ptr()[index];
    }

    MyString operator+(const MyString& rhs) const {
        MyString res;
        size_t total = len + rhs.len;
        if (total <= 15) {
            res.len = total;
            res.cap = 0;
            std::memcpy(res.small_buffer, data_ptr(), len);
            std::memcpy(res.small_buffer + len, rhs.data_ptr(), rhs.len);
            res.small_buffer[total] = '\0';
        } else {
            res.len = total;
            res.cap = total;
            res.allocate_heap(res.cap);
            std::memcpy(res.heap_ptr, data_ptr(), len);
            std::memcpy(res.heap_ptr + len, rhs.data_ptr(), rhs.len);
            res.heap_ptr[total] = '\0';
        }
        return res;
    }

    void append(const char* str) {
        if (!str) return;
        size_t add = std::strlen(str);
        size_t new_len = len + add;
        ensure_capacity(new_len);
        // after ensure_capacity, storage is correct based on need
        std::memcpy(data_ptr() + len, str, add + 1);
        len = new_len;
    }

    const char& at(size_t pos) const {
        if (pos >= len) throw std::out_of_range("index out of range");
        return data_ptr()[pos];
    }

    class const_iterator;

    class iterator {
    private:
        char* ptr{};
    public:
        explicit iterator(char* p=nullptr): ptr(p) {}
        iterator& operator++() { ++ptr; return *this; }
        iterator operator++(int) { iterator tmp=*this; ++ptr; return tmp; }
        iterator& operator--() { --ptr; return *this; }
        iterator operator--(int) { iterator tmp=*this; --ptr; return tmp; }
        char& operator*() const { return *ptr; }
        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
        bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
        friend class MyString;
    };

    class const_iterator {
    private:
        const char* ptr{};
    public:
        explicit const_iterator(const char* p=nullptr): ptr(p) {}
        const_iterator& operator++() { ++ptr; return *this; }
        const_iterator operator++(int) { const_iterator tmp=*this; ++ptr; return tmp; }
        const_iterator& operator--() { --ptr; return *this; }
        const_iterator operator--(int) { const_iterator tmp=*this; --ptr; return tmp; }
        const char& operator*() const { return *ptr; }
        bool operator==(const const_iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const const_iterator& other) const { return ptr != other.ptr; }
        friend class MyString;
        friend class iterator;
    };

public:
    iterator begin() { return iterator(data_ptr()); }
    iterator end() { return iterator(data_ptr() + len); }
    const_iterator cbegin() const { return const_iterator(data_ptr()); }
    const_iterator cend() const { return const_iterator(data_ptr() + len); }
};

#endif
