#pragma once
#include <algorithm>
#include <utility>
namespace tc::data {
template<typename Container1, typename Container2, typename DistType = size_t>
inline DistType EditDistance(const Container1 &a, const Container2 &b) {
  if (a.size() > b.size()) {
    return EditDistance<Container2, Container1, DistType>(b, a);
  }
  size_t min_size = a.size();
  size_t max_size = b.size();
  std::vector<DistType> lev_dist(min_size + 1);
  for (size_t i = 0; i <= min_size; ++i) {
    lev_dist[i] = i;
  }
  for (size_t j = 1; j < max_size; ++j) {
    DistType previous_diagonal = lev_dist[0];
    DistType previous_diagonal_save;
    ++lev_dist[0];
    for (size_t i = 1; i <= min_size; ++i) {
      previous_diagonal_save = lev_dist[i];
      if (a[i - 1] == b[j - 1]) {
        lev_dist[i] = previous_diagonal;
      } else {
        lev_dist[i] = std::min(std::min(lev_dist[i - 1], lev_dist[i]), previous_diagonal) + 1;
      }
      previous_diagonal = previous_diagonal_save;
    }
  }
  return lev_dist[min_size];
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