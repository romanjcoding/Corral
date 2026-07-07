#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <algorithm>
#include <stdexcept>

namespace util {

// Code credit -- Jason Turner, from https://xuhuisun.com/post/c++-weekly-2-constexpr-map/
template <typename Key, typename Value, size_t Size>
struct fast_map {
  std::array<std::pair<Key, Value>, Size> m_data;
 
  constexpr auto begin() const { return m_data.begin(); }
  constexpr auto end()   const { return m_data.end(); }
  constexpr auto begin()       { return m_data.begin(); }
  constexpr auto end()         { return m_data.end(); }

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
        std::ranges::find_if(m_data, [&key](const auto& v) { return v.first == key; });
    if (itr != end()) {
      return itr->second;
    } else {
      throw std::range_error("Not Found");
    }
  }

  constexpr size_t size() const { return m_data.size(); }
};

} // namespace util