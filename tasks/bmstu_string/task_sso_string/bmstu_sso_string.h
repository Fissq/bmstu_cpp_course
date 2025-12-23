#pragma once

#include <exception>
#include <iostream>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <sstream>

namespace bmstu
{
template <typename T>
class basic_string;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template <typename T>
class basic_string
{
   private:
    static constexpr size_t SSO_CAPACITY =
        (sizeof(T*) + sizeof(size_t) + sizeof(size_t)) / sizeof(T) - 1;

    struct LongString
    {
        T* ptr;
        size_t size;
        size_t capacity;
    };

    struct ShortString
    {
        T buffer[SSO_CAPACITY + 1];
        unsigned char size;
    };

    union Data
    {
        LongString long_str;
        ShortString short_str;
    };

    Data data_;
    bool is_long_;

    bool is_long() const { return is_long_; }

    T* get_ptr() {
        if (is_long()) {
            return data_.long_str.ptr;
        }
        return data_.short_str.buffer;
    }

    const T* get_ptr() const {
        if (is_long()) {
            return data_.long_str.ptr;
        }
        return data_.short_str.buffer;
    }

    size_t get_size() const {
        if (is_long()) {
            return data_.long_str.size;
        }
        return static_cast<size_t>(data_.short_str.size);
    }

    size_t get_capacity() const {
        if (is_long()) {
            return data_.long_str.capacity;
        }
        return SSO_CAPACITY;
    }

   public:
    basic_string() {
        is_long_ = false;
        data_.short_str.size = 0;
        data_.short_str.buffer[0] = T(0);
    }

    basic_string(size_t size) {
        if (size <= SSO_CAPACITY) {
            is_long_ = false;
            data_.short_str.size = static_cast<unsigned char>(size);
            for (size_t i = 0; i < size; ++i) {
                data_.short_str.buffer[i] = T(' ');
            }
            data_.short_str.buffer[size] = T(0);
        } else {
            is_long_ = true;
            data_.long_str.size = size;
            data_.long_str.capacity = size;
            data_.long_str.ptr = new T[size + 1];
            for (size_t i = 0; i < size; ++i) {
                data_.long_str.ptr[i] = T(' ');
            }
            data_.long_str.ptr[size] = T(0);
        }
    }

    basic_string(std::initializer_list<T> il) {
        size_t size = il.size();
        if (size <= SSO_CAPACITY) {
            is_long_ = false;
            data_.short_str.size = static_cast<unsigned char>(size);
            size_t i = 0;
            for (auto c : il) data_.short_str.buffer[i++] = c;
            data_.short_str.buffer[size] = T(0);
        } else {
            is_long_ = true;
            data_.long_str.size = size;
            data_.long_str.capacity = size;
            data_.long_str.ptr = new T[size + 1];
            size_t i = 0;
            for (auto c : il) data_.long_str.ptr[i++] = c;
            data_.long_str.ptr[size] = T(0);
        }
    }

    basic_string(const T* c_str) {
        size_t len = strlen_(c_str);
        if (len <= SSO_CAPACITY) {
            is_long_ = false;
            std::memcpy(data_.short_str.buffer, c_str, len * sizeof(T));
            data_.short_str.buffer[len] = T(0);
            data_.short_str.size = static_cast<unsigned char>(len);
        } else {
            is_long_ = true;
            data_.long_str.size = len;
            data_.long_str.capacity = len;
            data_.long_str.ptr = new T[len + 1];
            std::memcpy(data_.long_str.ptr, c_str, (len + 1) * sizeof(T));
        }
    }

    basic_string(const basic_string& other) {
        is_long_ = other.is_long_;
        if (is_long_) {
            data_.long_str.size = other.data_.long_str.size;
            data_.long_str.capacity = other.data_.long_str.capacity;
            data_.long_str.ptr = new T[data_.long_str.capacity + 1];
            std::memcpy(data_.long_str.ptr, other.data_.long_str.ptr, (data_.long_str.size + 1) * sizeof(T));
        } else {
            data_.short_str = other.data_.short_str;
        }
    }

    basic_string(basic_string&& dying) noexcept {
        is_long_ = dying.is_long_;
        if (is_long_) {
            data_.long_str = dying.data_.long_str;
            dying.data_.long_str.ptr = nullptr;
            dying.data_.long_str.size = 0;
            
            dying.is_long_ = false;
            dying.data_.short_str.size = 0;
            dying.data_.short_str.buffer[0] = T(0);
        } else {
            data_.short_str = dying.data_.short_str;
            dying.data_.short_str.size = 0;
            dying.data_.short_str.buffer[0] = T(0);
        }
    }

    ~basic_string() {
        clean_();
    }

    const T* c_str() const {
        return get_ptr();
    }

    size_t size() const {
        return get_size();
    }

    bool is_using_sso() const {
        return !is_long_;
    }

    size_t capacity() const {
        return get_capacity();
    }

    basic_string& operator=(basic_string&& other) noexcept {
        if (this != &other) {
            clean_();
            is_long_ = other.is_long_;
            if (is_long_) {
                data_.long_str = other.data_.long_str;
                other.data_.long_str.ptr = nullptr;
                other.data_.long_str.size = 0;
                
                other.is_long_ = false;
                other.data_.short_str.size = 0;
                other.data_.short_str.buffer[0] = T(0);
            } else {
                data_.short_str = other.data_.short_str;
                other.data_.short_str.size = 0;
                other.data_.short_str.buffer[0] = T(0);
            }
        }
        return *this;
    }

    basic_string& operator=(const T* c_str) {
        clean_();
        size_t len = strlen_(c_str);
        if (len <= SSO_CAPACITY) {
            is_long_ = false;
            std::memcpy(data_.short_str.buffer, c_str, len * sizeof(T));
            data_.short_str.buffer[len] = T(0);
            data_.short_str.size = static_cast<unsigned char>(len);
        } else {
            is_long_ = true;
            data_.long_str.size = len;
            data_.long_str.capacity = len;
            data_.long_str.ptr = new T[len + 1];
            std::memcpy(data_.long_str.ptr, c_str, (len + 1) * sizeof(T));
        }
        return *this;
    }

    basic_string& operator=(const basic_string& other) {
        if (this != &other) {
            clean_();
            is_long_ = other.is_long_;
            if (is_long_) {
                data_.long_str.size = other.data_.long_str.size;
                data_.long_str.capacity = other.data_.long_str.capacity;
                data_.long_str.ptr = new T[data_.long_str.capacity + 1];
                std::memcpy(data_.long_str.ptr, other.data_.long_str.ptr, (data_.long_str.size + 1) * sizeof(T));
            } else {
                data_.short_str = other.data_.short_str;
            }
        }
        return *this;
    }

    friend basic_string<T> operator+(const basic_string<T>& left,
                                     const basic_string<T>& right)
    {
        basic_string<T> result;
        size_t new_len = left.size() + right.size();
        
        if (new_len <= SSO_CAPACITY) {
            result.is_long_ = false;
            result.data_.short_str.size = static_cast<unsigned char>(new_len);
            std::memcpy(result.data_.short_str.buffer, left.c_str(), left.size() * sizeof(T));
            std::memcpy(result.data_.short_str.buffer + left.size(), right.c_str(), right.size() * sizeof(T));
            result.data_.short_str.buffer[new_len] = T(0);
        } else {
            result.is_long_ = true;
            result.data_.long_str.size = new_len;
            result.data_.long_str.capacity = new_len;
            result.data_.long_str.ptr = new T[new_len + 1];
            std::memcpy(result.data_.long_str.ptr, left.c_str(), left.size() * sizeof(T));
            std::memcpy(result.data_.long_str.ptr + left.size(), right.c_str(), right.size() * sizeof(T));
            result.data_.long_str.ptr[new_len] = T(0);
        }
        return result;
    }

    friend bool operator==(const basic_string<T>& lhs, const basic_string<T>& rhs) {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        const T* l_ptr = lhs.c_str();
        const T* r_ptr = rhs.c_str();
        for (size_t i = 0; i < lhs.size(); ++i) {
            if (l_ptr[i] != r_ptr[i]) {
                return false;
            }
        }
        return true;
    }

    friend bool operator!=(const basic_string<T>& lhs, const basic_string<T>& rhs) {
        return !(lhs == rhs);
    }

    template <typename S>
    friend S& operator<<(S& os, const basic_string& obj)
    {
        os << obj.c_str();
        return os;
    }

    template <typename S>
    friend S& operator>>(S& is, basic_string& obj)
    {
        std::basic_ostringstream<T> oss;
        oss << is.rdbuf(); 
        auto s = oss.str();
        obj = s.c_str();
        return is;
    }

    basic_string& operator+=(const basic_string& other) {
        size_t current_size = size();
        size_t other_size = other.size();
        size_t new_size = current_size + other_size;

        if (is_long() && new_size <= data_.long_str.capacity) {
            std::memcpy(data_.long_str.ptr + current_size, other.c_str(), other_size * sizeof(T));
            data_.long_str.size = new_size;
            data_.long_str.ptr[new_size] = T(0);
        }
        else if (!is_long() && new_size <= SSO_CAPACITY) {
            std::memcpy(data_.short_str.buffer + current_size, other.c_str(), other_size * sizeof(T));
            data_.short_str.size = static_cast<unsigned char>(new_size);
            data_.short_str.buffer[new_size] = T(0);
        }
        else {
            size_t new_capacity = new_size; 
            T* new_ptr = new T[new_capacity + 1];
            std::memcpy(new_ptr, c_str(), current_size * sizeof(T));
            std::memcpy(new_ptr + current_size, other.c_str(), other_size * sizeof(T));
            new_ptr[new_size] = T(0);
            
            clean_();
            is_long_ = true;
            data_.long_str.ptr = new_ptr;
            data_.long_str.size = new_size;
            data_.long_str.capacity = new_capacity;
        }
        return *this;
    }

    basic_string& operator+=(T symbol) {
        T temp[2] = {symbol, T(0)};
        return *this += temp; 
    }

    T& operator[](size_t index) noexcept
    {
        return get_ptr()[index];
    }

    const T& operator[](size_t index) const noexcept
    {
        return get_ptr()[index];
    }

    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("Wrong index");
        }
        return get_ptr()[index];
    }

    T* data() { return get_ptr(); }

   private:
    static size_t strlen_(const T* str) {
        size_t len = 0;
        while (str[len] != T(0)) {
            len++;
        }
        return len;
    }

    void clean_() {
        if (is_long_) {
            delete[] data_.long_str.ptr;
            data_.long_str.ptr = nullptr;
        }
    }
};
}