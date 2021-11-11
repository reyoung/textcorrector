#pragma once
#include "ThreadPool.h"
#include "tc/base/except.h"
#include <vector>

namespace tc::data {

template<typename BKTree>
class MultiThreadBKTree {
 public:
  using value_type = typename BKTree::value_type;
  using dist_type = typename BKTree::dist_type;

  explicit MultiThreadBKTree(size_t n) : trees_(n) {}

  MultiThreadBKTree() = default;

  void Add(value_type item) {
    auto &tree = trees_[add_counter_++ % (trees_.size())];
    tree.Add(std::move(item));
  }

  template<typename Query, typename Callback>
  void Search(ThreadPool &pool, const Query &q, dist_type limit, Callback callback) const {
    std::vector<std::future<void>> futures_;
    for (size_t i = 0; i < trees_.size(); ++i) {
      futures_.emplace_back(pool.enqueue([i, this, &q, &limit, &callback] {
        auto &tree = trees_[i];
        tree.Search(q, limit, [&](const value_type &item, dist_type d) {
          return callback(i, item, d);
        });
      }));
    }

    for (auto &fut : futures_) {
      fut.wait();
    }
  }

  template<typename ItemDumper = DefaultItemDumper<value_type>, typename OffsetType = uint16_t>
  void DumpMMap(std::ostream &os, ItemDumper dumper = DefaultItemDumper<value_type>()) {
    DefaultItemDumper<size_t> size_dumper;
    size_dumper(os, trees_.size());
    for (auto &tree : trees_) {
      tree.template DumpMMap<ItemDumper, OffsetType>(os, dumper);
    }
  }

  template<typename ItemDumper = DefaultItemDumper<value_type>>
  void Dump(std::ostream &os, ItemDumper dumper = DefaultItemDumper<value_type>()) const {
    DefaultItemDumper<size_t> size_dumper;
    size_dumper(os, trees_.size());
    for (auto &tree : trees_) {
      tree.Dump(os, dumper);
    }
  }
  template<typename ItemLoader = DefaultItemLoader<value_type>>
  void Load(std::istream &is, ItemLoader loader = DefaultItemLoader<value_type>()) {
    TC_ENFORCE(trees_.empty()) << "tree must be empty when use Load";
    DefaultItemLoader<size_t> size_loader;
    auto n = size_loader(is);
    trees_.resize(n);
    for (auto &tree : trees_) {
      tree.Load(is, loader);
    }
  }

  size_t NumTrees() const {
    return trees_.size();
  }

 private:
  std::vector<BKTree> trees_;
  size_t add_counter_{0};
};

}// namespace tc::data