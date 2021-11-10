#pragma once
#include "tc/base/except.h"
#include "tc/data/default_item_dumper_loader.h"
#include "tc/data/edit_distance.h"
#include <map>
#include <queue>
#include <stack>
#include <utility>

namespace tc::data {

template<typename Item, typename DistT = size_t, typename DistCalc = EditDistanceCalculator<DistT>>
class InMemoryBKTree {
 private:
  struct Node {
    Item item_;
    std::map<DistT, std::unique_ptr<Node>> children_;

    explicit Node(Item item) : item_(std::move(item)) {}

    void Insert(Item item, const DistCalc &calc) {
      auto dist = calc(item, item_);
      if (dist == 0) {// is same
        return;
      }
      auto &next = children_[dist];
      if (next == nullptr) {
        next = std::make_unique<Node>(std::move(item));
        return;
      }
      next->Insert(std::move(item), calc);
    }

    template<typename Query, typename Callback, typename ExploreFrontier>
    bool Search(const DistCalc &calc, const Query &q, DistT limit, Callback callback, ExploreFrontier explore_frontier) const {
      auto d = calc(item_, q);
      if (d <= limit) {
        bool continue_ = callback(item_, d);
        if (!continue_) {
          return false;
        }
      }
      auto lower_bound = limit > d ? 0 : d - limit;
      auto upper_bound = d + limit;
      auto up_it = children_.upper_bound(upper_bound);
      for (auto it = children_.lower_bound(lower_bound); it != children_.end() && it != up_it; ++it) {
        Node *c = it->second.get();
        explore_frontier(c);
      }
      return true;
    }
  };

  struct LoadHelperItem {
    Node *parent;
    DistT dist;
  };

 public:
  InMemoryBKTree() = default;
  void Add(Item item) {
    if (root_ == nullptr) {
      root_ = std::make_unique<Node>(std::move(item));
      return;
    }
    root_->Insert(std::move(item), calc_);
  }

  template<typename Query, typename Callback>
  void Search(const Query &q, DistT limit, Callback callback) const {
    if (root_ == nullptr) {
      return;
    }
    std::stack<Node *> frontier;
    frontier.emplace(root_.get());
    while (!frontier.empty()) {
      auto n = frontier.top();
      frontier.pop();
      auto continue_ = n->Search(calc_, q, limit, callback, [&frontier](Node *n) {
        frontier.emplace(n);
      });
      if (!continue_) {
        return;
      }
    }
  }

  template<typename ItemDumper = DefaultItemDumper<Item>>
  void Dump(std::ostream &os, ItemDumper dumper = DefaultItemDumper<Item>()) const {
    std::queue<const Node *> frontier;
    frontier.emplace(root_.get());
    DefaultItemDumper<size_t> size_dumper;
    DefaultItemDumper<DistT> dist_dumper;
    while (!frontier.empty()) {
      const Node *n = frontier.front();
      frontier.pop();
      dumper(os, n->item_);
      size_dumper(os, n->children_.size());
      for (auto &[d, c] : n->children_) {
        dist_dumper(os, d);
        frontier.emplace(c.get());
      }
    }
  }

  template<typename ItemLoader = DefaultItemLoader<Item>>
  void Load(std::istream &is, ItemLoader loader = DefaultItemLoader<Item>()) {
    TC_ENFORCE(root_ == nullptr) << "should call Load only for empty bktree";
    std::queue<LoadHelperItem> frontier;
    root_ = std::make_unique<Node>(loader(is));
    ExtendLoadFrontier(&frontier, root_.get(), is);
    while (!frontier.empty()) {
      LoadHelperItem item = frontier.front();
      frontier.pop();

      auto &ptr = item.parent->children_[item.dist];
      ptr = std::make_unique<Node>(loader(is));
      ExtendLoadFrontier(&frontier, ptr.get(), is);
    }
  }

 private:
  void ExtendLoadFrontier(std::queue<LoadHelperItem> *frontier, Node *node, std::istream& is) {
    DefaultItemLoader<size_t> size_loader;
    DefaultItemLoader<DistT> dist_loader;
    size_t n = size_loader(is);
    for (size_t i = 0; i < n; ++i) {
      auto d = dist_loader(is);
      frontier->emplace(LoadHelperItem{node, d});
    }
  }

 private:
  std::unique_ptr<Node> root_;
  DistCalc calc_;
};

}// namespace tc::data