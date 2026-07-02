#include "AVAILABLE_PIECES.h"
#include "dynamic_array.h"
#include "fast_map.h"
#include "bitmap.h"

#include <ranges>
#include <cstdint>
#include <utility>
#include <algorithm>

inline constexpr auto ORIENTATIONS_PRECOMPUTED = []() {
    util::fast_map<bitmap, util::dynamic_array<bitmap, MAXIMUM_ORIENTATIONS>, NUM_PIECES> m{};
    auto it { m.data.begin() };
    for (auto piece : std::views::keys(AVAILABLE_PIECES)) {
        piece.canonicalize();
        it->first = piece;
        it->second = generate_all_orientations(piece);
        it++;
    }
    return m;
}();

inline constexpr auto TO_CANONICAL_PRECOMPUTED = []() {
    util::fast_map<bitmap, bitmap, MAXIMUM_ORIENTATIONS * NUM_PIECES> m{};
    auto it { m.data.begin() };
    for (auto piece : std::views::keys(AVAILABLE_PIECES)) {
        piece.canonicalize();
        for (auto orient : generate_all_orientations(piece)) {
            it->first = piece;
            it->second = orient;
            it++;
        }
    }
    return m;
}();

inline constexpr auto MAX_PIECE_BITS = []() {
    size_t max{0uz};
    for (const auto& piece : std::views::keys(AVAILABLE_PIECES)) { max = std::max(max, piece.count()); }
    return max;
}();

inline constexpr auto VALID_BITS_PRECOMPUTED = []() {
    util::fast_map<bitmap, util::dynamic_array<std::pair<size_t, size_t>, MAX_PIECE_BITS>, NUM_PIECES> m{};
    auto it { m.data.begin() };
    for (auto piece : std::views::keys(AVAILABLE_PIECES)) {
        piece.canonicalize();
        it->first = piece;
        for (auto row : std::views::iota(0uz, GRID_SIZE)) {
            for (auto col : std::views::iota(0uz, GRID_SIZE)) {
                if (piece.test(col, row)) {
                    it->second.push_back(std::pair(col, row));
                }
            }
        }
        it++;
    }
    return m;
}();
