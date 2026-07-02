#pragma once

#include "fast_map.h"
#include "dynamic_array.h"

#include <array>
#include <algorithm>
#include <ranges>
#include <map>
#include <cstdint>
#include <stdexcept>
#include <ranges>
#include <vector>
#include <bitset>
#include <iostream>
#include <cstring>
#include <bit>
#include <numeric>

constexpr std::pair<int, int> NBH4[] = {
    {0, 1}, // row, col
    {0, -1},
    {1, 0},
    {-1, 0},
};

constexpr std::pair<int, int> NBH8[] = {
    {0, 1}, // row, col
    {0, -1},
    {1, 0},
    {-1, 0},
    {-1, 1},
    {-1, -1},
    {1, -1},
    {1, 1}
};

constexpr size_t GRID_SIZE {16};
constexpr size_t MAXIMUM_ORIENTATIONS {8uz};

class bitmap {
    std::array<uint16_t, 16> m_data{};
    uint16_t m_height{GRID_SIZE}; // logical height
    uint16_t m_width {GRID_SIZE}; // logical width
public:

    constexpr bitmap() = default;
    constexpr bitmap(std::array<uint16_t, 16> arr) : m_data{arr} {};

    constexpr void flip() {
        for (auto& row : m_data) {
            row = ~row;
        }
    }

    constexpr bitmap& operator&=(const bitmap& rhs) {
        for (auto i : std::views::iota(0uz, GRID_SIZE)) {
            m_data[i] &= rhs.m_data[i];
        }
        return *this;
    }

    constexpr bitmap& operator|=(const bitmap& rhs) {
        for (auto i : std::views::iota(0uz, GRID_SIZE)) {
            m_data[i] |= rhs.m_data[i];
        }
        return *this;
    }

    constexpr friend bitmap operator& (bitmap lhs, const bitmap& rhs) { return lhs &= rhs; }
    constexpr friend bitmap operator| (bitmap lhs, const bitmap& rhs) { return lhs |= rhs; }
    constexpr friend bitmap operator~ (bitmap m) { m.flip(); return m; }

    constexpr friend auto operator<=>(const bitmap& lhs, const bitmap& rhs) = default;

    constexpr void shift_x(int x) {
        for (auto& row : m_data) {
            if (x <= 0) { row <<= -x; }
            else        { row >>= x; }
        }
    }   
    constexpr void shift_y(int dy) {
        if (dy == 0) { return; }
        else if (dy > 0) {
            std::move_backward(m_data.begin(), m_data.end() - dy, m_data.end());
            std::fill(m_data.begin(), m_data.begin() + dy, 0);
        }
        else {
            std::move(m_data.begin() - dy, m_data.end(), m_data.begin());
            std::fill(m_data.end() + dy, m_data.end(), 0);
        }
    }

    constexpr void move(int dx, int dy) {
        shift_x(dx);
        shift_y(dy);
    }

    constexpr bool any() const { return std::ranges::any_of(m_data, std::identity{}); }
    constexpr void set(size_t col, size_t row) {
        assert(row < m_width);
        assert(col < m_height);
        m_data[row] |= (1ull << (m_width - col - 1));
    }

    constexpr bool test(size_t col, size_t row) const {
        assert(row < m_width);
        assert(col < m_height);
        return (1ull << (m_width - col - 1)) & m_data[row];
    }

    constexpr void flip(size_t col, size_t row) {
        assert(row < m_width);
        assert(col < m_height);
        m_data[row] ^= (1ull << (m_width - col - 1));
    }

    constexpr std::pair<size_t, size_t> min_corner() const;
    constexpr std::pair<size_t, size_t> max_corner() const;

    constexpr void normalize() {
        const auto [min_col, min_row] {min_corner()};
        move(-min_col, -min_row);
    }

    constexpr void canonicalize();

    constexpr void expand(std::span<const std::pair<int, int>> directions) {
        bitmap original {*this};
        for (auto [dr, dc] : directions) {
            bitmap shift {original};
            shift.move(dr, dc);
            *this |= shift;
        }
    }

    constexpr bitmap get_frontier(std::span<const std::pair<int, int>> directions) const {
        bitmap cpy {*this};
        cpy.expand(directions);
        return cpy & ~(*this);
    }

    constexpr size_t count() const {
        return std::accumulate(
            m_data.begin(), m_data.end(),
            0uz, 
            [](size_t acc, auto value) { return acc + std::popcount(value); }
        );
    }

    constexpr util::dynamic_array<std::pair<size_t, size_t>, 16*16> get_valid_bits() const {
        util::dynamic_array<std::pair<size_t, size_t>, 16*16> valid_bits{};
        for (auto row : std::views::iota(0uz, m_height)) {
            for (auto col : std::views::iota(0uz, m_width)) {
                if ((*this).test(col, row)) { valid_bits.push_back(std::pair(col, row)); }
            }
        }
        return valid_bits;
    }

    void print() const {
        for (auto j : std::views::iota(0uz, GRID_SIZE)) {
            for (auto i : std::views::iota(0uz, GRID_SIZE)) {
                if ((*this).test(i, j)) { std::cout << "\u25A0 "; }
                else { std::cout << "\u25A1 "; }
            }
            std::cout << '\n';
        }
    }
};

constexpr std::array<bitmap, GRID_SIZE> COL_MASKS = []() {
    std::array<bitmap, GRID_SIZE> masks{};
    for (size_t col{}; col < GRID_SIZE; col++) {
        std::array<uint16_t, 16> bmap{};
        bmap.fill(1 << (GRID_SIZE - col - 1));
        masks[col] = bitmap { bmap };
    }
    return masks;
}();

constexpr std::array<bitmap, GRID_SIZE> ROW_MASKS = []() {
    std::array<bitmap, GRID_SIZE> masks{};
    for (size_t row{}; row < GRID_SIZE; row++) {
        std::array<uint16_t, 16> bmap{};
        bmap[row] = std::numeric_limits<uint16_t>::max();
        masks[row] = bitmap { bmap };
    }
    return masks;
}();

constexpr bitmap TOP_LEFT = []() {
    bitmap mask{};
    for (size_t i{}; i < GRID_SIZE / 2; i++) {            
        mask |= ROW_MASKS[i];
    }
    for (size_t j{(GRID_SIZE + 1)/2 - 1}; j < GRID_SIZE; j++) {
        mask &= ~COL_MASKS[j];
    }
    return mask;
}();

constexpr util::dynamic_array<bitmap, MAXIMUM_ORIENTATIONS>  generate_all_orientations(const bitmap& piece) {

    bitmap clockwise{};
    bitmap counterclockwise{};
    bitmap half{};
 
    // reflected (across column) pieces
    bitmap rpiece{}; 
    bitmap rclockwise{};
    bitmap rcounterclockwise{};
    bitmap rhalf{};

    bitmap remaining { piece };
    remaining.normalize();

    auto [max_col, max_row] { remaining.max_corner() };
    
    const auto valid_bits { remaining.get_valid_bits() };
    for (auto [col, row] : valid_bits) {
            
        clockwise.set(max_row - row, col);
        half.set(max_col - col, max_row - row); 
        counterclockwise.set(row, max_col - col);
        
        rpiece.set(max_col - col, row);
        rclockwise.set(max_row - row, max_col - col);
        rhalf.set(col, max_row - row);
        rcounterclockwise.set(row, col);
    }

    util::dynamic_array<bitmap, MAXIMUM_ORIENTATIONS> orientations{};
    for (const auto& piece_orientation : 
        { piece, clockwise, half, counterclockwise,
          rpiece, rclockwise, rhalf, rcounterclockwise}) 
        {
        if (!orientations.contains(piece_orientation)) { 
            orientations.push_back(piece_orientation); 
        }
    }
    return orientations;
}

constexpr std::pair<size_t, size_t> bitmap::min_corner() const {
    size_t min_row{};
    for (; min_row < GRID_SIZE; min_row++) {
        if ((*this & ROW_MASKS[min_row]).any()) { break; }
    }
    size_t min_col{};
    for (; min_col < GRID_SIZE; min_col++) {
        if ((*this & COL_MASKS[min_col]).any()) { break; }
    }
    return std::pair(min_col, min_row);
}

constexpr std::pair<size_t, size_t> bitmap::max_corner() const {
    size_t max_row{GRID_SIZE - 1};
    for (; max_row > 0; max_row--) {
        if ((*this & ROW_MASKS[max_row]).any()) { break; }
    }
    size_t max_col{GRID_SIZE - 1};
    for (; max_col > 0; max_col--) { 
        if ((*this & COL_MASKS[max_col]).any()) { break; }
    }
    return std::pair(max_col, max_row);
}

constexpr void bitmap:: canonicalize() {
    normalize();
    auto orientations { generate_all_orientations(*this) };
    *this = *std::ranges::min_element(orientations, [](const bitmap& a, const bitmap& b) {
        return a < b;
    });
}