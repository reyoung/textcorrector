#pragma once
#include "ThreadPool.h"
#include "fcntl.h"
#include "sys/mman.h"
#include "sys/stat.h"
#include "tc/base/except.h"
#include "unistd.h"
#include <vector>
namespace tc::data {

template<typename BKTree>
class MultiThreadBKTree {
 public:
  using value_type = typename BKTree::value_type;
  using dist_type = typename BKTree::dist_type;
  enum {
    tree_attr = BKTree::tree_attr
  };

  explicit MultiThreadBKTree(size_t n) : trees_(n) {}

  MultiThreadBKTree() = default;
  ~MultiThreadBKTree() {
    if (mmap_buf_.has_value()) {
      munmap(const_cast<char *>(mmap_buf_->data()), mmap_buf_->size());
    }

    if (fd_.has_value()) {
      close(*fd_);
    }
  }

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

  template<typename ItemDumper = DefaultItemDumper<value_type>, typename OffsetType = uint16_t>
  void DumpMMap(std::ostream &os, ItemDumper dumper = DefaultItemDumper<value_type>()) const {
    DefaultItemDumper<size_t> size_dumper;
    size_dumper(os, trees_.size());
    std::vector<size_t> sizes;
    std::ios::pos_type size_begin = os.tellp();
    for (size_t i = 0; i < trees_.size(); i++) {
      size_dumper(os, 0);
    }

    for (size_t i = 0; i < trees_.size(); i++) {
      auto &tree = trees_[i];
      auto tree_beg = os.tellp();
      tree.template DumpMMap<ItemDumper, OffsetType>(os, dumper);
      auto tree_end = os.tellp();
      sizes.emplace_back(tree_end - tree_beg);
    }

    auto os_end = os.tellp();
    os.seekp(size_begin);
    for (auto &s : sizes) {
      std::cerr << "dump tree size " << s << std::endl;
      size_dumper(os, s);
    }
    os.seekp(os_end);
  }

  template<typename ItemMMapLoader = DefaultItemMMapLoader<value_type>>
  void
  LoadMMap(const char *filename, ItemMMapLoader loader = DefaultItemMMapLoader<value_type>()) {
    TC_ENFORCE(trees_.empty()) << "tree must be empty when use Load";
    fd_ = open(filename, O_RDONLY);
    TC_ENFORCE(*fd_ > 0) << "cannot open file " << filename;
    struct stat sb {};
    TC_ENFORCE(fstat(*fd_, &sb) != -1) << "cannot fstat";
    size_t size = sb.st_size;
    void *addr = mmap(nullptr, size, PROT_READ, MAP_SHARED, *fd_, 0);
    mmap_buf_ = std::string_view(reinterpret_cast<char *>(addr), size);

    DefaultItemMMapLoader<size_t> size_loader;
    auto tmp = size_loader(*mmap_buf_);
    size = tmp.first;
    std::string_view next = tmp.second;
    std::vector<size_t> sizes;
    sizes.reserve(size);
    for (size_t i = 0; i < size; ++i) {
      tmp = size_loader(next);
      sizes.emplace_back(tmp.first);
      next = tmp.second;
    }

    for (size_t s : sizes) {
      trees_.emplace_back();
      auto &tree = trees_.back();
      tree.LoadMMap(next.substr(0, s), loader);
      next = next.substr(s);
    }
  }

  size_t NumTrees() const {
    return trees_.size();
  }

 private:
  std::vector<BKTree> trees_;
  size_t add_counter_{0};
  std::optional<int> fd_;
  std::optional<std::string_view> mmap_buf_;
};

}// namespace tc::data