#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <algorithm>

namespace util {

// Code credit -- Jason Turner, from https://xuhuisun.com/post/c++-weekly-2-constexpr-map/
template <typename Key, typename Value, size_t Size>
struct fast_map {
  std::array<std::pair<Key, Value>, Size> data;

  constexpr auto begin() const { return data.begin(); }
  constexpr auto end()   const { return data.end(); }
  constexpr auto begin()       { return data.begin(); }
  constexpr auto end()         { return data.end(); }

  [[nodiscard]] constexpr const Value& at(const Key &key) const {
    const auto itr =
        std::find_if(begin(), end(),
                     [&key](const auto &v) { return v.first == key; });
    if (itr != end()) {
      return itr->second;
    } else {
      throw std::range_error("Not Found");
    }
  }

  [[nodiscard]] constexpr Value& at(const Key& key) {
    const auto itr =
        std::ranges::find_if(data, [&key](const auto& v) { return v.first == key; });
    if (itr != end()) {
      return itr->second;
    } else {
      throw std::range_error("Not Found");
    }
  }
};

} // namespace util