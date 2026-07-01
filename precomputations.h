#include "AVAILABLE_PIECES.h"
#include "dynamic_array.h"
#include "bitmap.h"
#include <ranges>

util::fast_map<bitmap, bitmap, MAXIMUM_ORIENTATIONS * NUM_PIECES> PRECOMPUTED_TO_CANONICAL{};

constexpr auto ORIENTATIONS_PRECOMPUTED = []() {
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

constexpr auto TO_CANONICAL_PRECOMPUTED = []{} (
    util::fast_map<bitmap, bitmap, MAXIMUM_ORIENTATIONS * NUM_PIECES> m{};
    return m;
)

void run_precomputations() {

    for (const auto piece : std::views::keys(AVAILABLE_PIECES)) {  
        auto piece_cpy {piece};
        piece_cpy.canonicalize();
        for (const auto orient : generate_all_orientations(piece_cpy)) {
            ORIENTATIONS_PRECOMPUTED.at(piece).push_back(orient);
            // PRECOMPUTED_TO_CANONICAL[orient] = piece;
            auto vec { orient.get_valid_bits() };
            // PRECOMPUTED_TRUE_BITS[orient] = vec;
        }
    }
}

// util::fast_map<bitmap, bitmap> TO_CANONICAL_PRECOMPUTED{};
// util::fast_map<bitmap, std::vector<std::pair<size_t, size_t>>> TRUE_BITS_PRECOMPUTED{};

