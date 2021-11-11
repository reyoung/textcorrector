#pragma once
#include "tc/base/except.h"
#include "tc/data/tree_attribute.h"
#include <algorithm>
#include <codecvt>
#include <iterator>
#include <memory>
#include <optional>
#include <string_view>

namespace tc::data {

template<typename Item, typename DistT = size_t, typename DistCalc = EditDistanceCalculator<DistT>>
class MMapBKTree {
 public:
  using value_type = Item;
  using dist_type = DistT;

  explicit MMapBKTree(std::string_view buf) : buf_(buf) {
  }

  template<typename Query, typename Callback, typename OffsetT = uint16_t,
           typename ItemMMappLoader = DefaultItemMMapLoader<value_type>>
  void Search(const Query &q, DistT limit, Callback callback,
              ItemMMappLoader loader = DefaultItemMMapLoader<value_type>()) const {
    std::stack<std::string_view> frontier;
    frontier.push(buf_);
    using DistSpan = std::span<const DistT>;
    DefaultItemMMapLoader<DistSpan> dist_loader;
    while (!frontier.empty()) {
      std::string_view node = frontier.top();
      frontier.pop();

      auto [item, next] = loader(node);
      node = next;
      auto d = calc_(item, q);
      if (d <= limit) {
        bool continue_ = callback(item, d);
        if (!continue_) {
          return;
        }
      }

      auto [dists, tmp] = dist_loader(node);
      node = tmp;
      auto lower_bound = limit > d ? 0 : d - limit;
      auto upper_bound = d + limit;
      auto up_it = std::upper_bound(dists.begin(), dists.end(), upper_bound);
      auto *offsets = reinterpret_cast<const OffsetT *>(node.data());
      for (auto it = std::upper_bound(dists.begin(), dists.end(), lower_bound); it != dists.end() && it != up_it; ++it) {
        size_t pos = it - dists.begin();
        OffsetT offset = offsets[pos];
        const char *beg = reinterpret_cast<const char *>(&offsets[pos]) + offset;
        frontier.push(node.substr(beg - &node[0]));
      }
    }
  }

 private:
  std::string_view buf_;
  DistCalc calc_;
};

template<typename Item, typename DistT = size_t, typename OffsetT = uint16_t,
         typename DistCalc = EditDistanceCalculator<DistT>, typename ItemMMapLoader = DefaultItemMMapLoader<Item>>
class MMapBKTreeHolder {
 public:
  using Tree = MMapBKTree<Item, DistT, DistCalc>;
  using value_type = typename Tree::value_type;
  using dist_type = typename Tree::dist_type;
  enum {
    tree_attr = static_cast<uint32_t>(TreeAttributes::TA_SUPPORT_LOAD_MMAP)
  };

  void LoadMMap(std::string_view buf, ItemMMapLoader loader) {
    TC_ENFORCE(!tree_) << "tree must be empty";
    tree_ = Tree(buf);
    loader_ = loader;
  }

  template<typename Query, typename Callback>
  void Search(const Query &q, DistT limit, Callback callback) const {
    TC_ENFORCE(tree_) << "tree must load";
    (*tree_).template Search<Query, Callback, OffsetT>(q, limit, callback, *loader_);
  }

 private:
  std::optional<ItemMMapLoader> loader_;
  std::optional<Tree> tree_;
};

}// namespace tc::data