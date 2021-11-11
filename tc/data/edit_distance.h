#pragma once
#include "tc/base/except.h"
#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace tc::data {

template<typename Container1, typename Container2, typename DistType = size_t>
inline DistType EditDistance(const Container1 &a, const Container2 &b) {
  size_t m = a.size() + 1;
  size_t n = b.size() + 1;
  static thread_local std::vector<DistType> d;
  d.resize(m * n);
  std::fill(d.begin(), d.end(), 0);

  for (size_t i = 1; i < m; ++i) {
    d[i * n] = i;
  }

  for (size_t j = 1; j < n; ++j) {
    d[j] = j;
  }

  for (size_t j = 1; j < n; ++j) {
    for (size_t i = 1; i < m; ++i) {
      DistType cost = a[i - 1] == b[j - 1] ? 0 : 1;
      auto deletion = d[(i - 1) * n + j] + 1;
      auto insertion = d[i * n + j - 1] + 1;
      auto subsitution = d[(i - 1) * n + j - 1] + cost;
      d[i * n + j] = std::min(deletion, std::min(insertion, subsitution));
    }
  }
  return d.back();
}

namespace details {
struct UTF8Utils {
  static std::string_view NextRune(std::string_view str) {
    uint8_t ch = *reinterpret_cast<const uint8_t *>(&str[0]);
    size_t zero_pos = __builtin_clz((~ch) & 0xFF) - 24;
    switch (zero_pos) {
      case 0:// 0xxxxxxx
        return str.substr(0, 1);
      case 1:// 10xxxxxx
        // error
        break;
      case 2:// 110xxxxx
        return str.substr(0, 2);

      case 3:// 1110xxxx
        return str.substr(0, 3);

      case 4:// 11110xxx
        return str.substr(0, 4);
      default:
        // error
        break;
    }
    TC_THROW() << "invalid utf8 text";
  }

  static void SplitRunesInto(std::string_view a, std::vector<std::string_view> *v) {
    while (!a.empty()) {
      auto r = NextRune(a);
      a = a.substr(r.size());
      v->emplace_back(r);
    }
  }
};

}// namespace details

template<typename DistType = size_t>
inline DistType EditDistanceUTF8(std::string_view a, std::string_view b) {
  static thread_local std::vector<std::string_view> aRunes;
  static thread_local std::vector<std::string_view> bRunes;
  aRunes.clear();
  bRunes.clear();
  details::UTF8Utils::SplitRunesInto(a, &aRunes);
  details::UTF8Utils::SplitRunesInto(b, &bRunes);
  return EditDistance<std::vector<std::string_view>, std::vector<std::string_view>, DistType>(aRunes, bRunes);
}

template<typename DistType = size_t>
class EditDistanceCalculator {
 public:
  template<typename Container1, typename Container2>
  DistType operator()(const Container1 &a, const Container2 &b) const {
    return EditDistance(a, b);
  }
};

template<typename DistType = size_t>
class UTF8EditDistanceCalculator {
 public:
  DistType operator()(std::string_view a, std::string_view b) const {
    return EditDistanceUTF8(a, b);
  }
};

}// namespace tc::data