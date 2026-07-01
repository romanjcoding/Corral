#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <algorithm> 
#include <stdexcept>

namespace util {

template <class T, size_t N>
struct dynamic_array {
private:
    std::array<T, N> m_arr{};
    size_t m_size{};

public:
    static constexpr size_t capacity() { return N; }
    constexpr bool full()  const { return m_size == capacity(); }
    constexpr bool empty() const { return m_size == 0; }
    constexpr size_t size() const { return m_size; }

    constexpr dynamic_array() = default;

    constexpr dynamic_array(std::initializer_list<T> il) : m_size(il.size()) {
        assert(il.size() <= N);
        std::ranges::copy(il, m_arr.begin());
    }

    constexpr T& at(const size_t i) {
        if (i >= m_size) { throw std::out_of_range("Error, .at() access out of bounds."); }
        return m_arr[i];
    }

    constexpr T& at(const size_t i) const {
        if (i >= m_size) { throw std::out_of_range("Error, .at() access out of bounds."); }
        return m_arr[i];
    }

    constexpr bool contains(const T& m) const {
        for (size_t i{}; i < m_size; i++) {
            if (m == m_arr[i]) { return true; }
        }
        return false;
    }

    constexpr size_t find (const T& m) const {
        for (size_t i{}; i < m_size; i++) {
            if (m == m_arr[i]) { return i; }
        }
        return m_size;
    }

    constexpr void push_back(const T& m) {
        assert(m_size < capacity());
        m_arr[m_size++] = m;
    }

    constexpr auto begin()       { return m_arr.begin(); }
    constexpr auto end()         { return m_arr.begin() + m_size; }
    constexpr auto begin() const { return m_arr.begin(); }
    constexpr auto end()   const { return m_arr.begin() + m_size; }
};

} // namespace util