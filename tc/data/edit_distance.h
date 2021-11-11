#pragma once
#include <algorithm>
#include <utility>
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

template<typename DistType = size_t>
class EditDistanceCalculator {
 public:
  using dist_type = DistType;
  template<typename Container1, typename Container2>
  DistType operator()(const Container1 &a, const Container2 &b) const {
    return EditDistance(a, b);
  }
};

}// namespace tc::data