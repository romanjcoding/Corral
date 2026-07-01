#pragma once

#include "fast_map.h"
#include "bitmap.h"
#include <map>

constexpr size_t NUM_PIECES {1uz};

constexpr util::fast_map<bitmap, std::size_t, NUM_PIECES> AVAILABLE_PIECES = {{
    std::pair<bitmap, std::size_t>{ bitmap {{
        0b1'1'1'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'1'1'1'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'1'1'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'1'1'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
        0b0'0'0'0'0'0'0'0'0'0'0'0'0'0'0'0,
    }}, 1},
}};

constexpr auto val { AVAILABLE_PIECES.data.begin()->first.get_valid_bits() };
constexpr auto max { AVAILABLE_PIECES.data.begin()->first.max_corner() };