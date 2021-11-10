#pragma once
#include <iostream>
#include <string>
#include <type_traits>
namespace tc::data {
template<typename Item>
class DefaultItemDumper {
 public:
  std::enable_if_t<std::is_pod_v<Item>, void> operator()(std::ostream &os, const Item &item) const {
    os.write(reinterpret_cast<const char *>(&item), sizeof(item));
  }
};

template<typename Item>
class DefaultItemLoader {
 public:
  std::enable_if_t<std::is_pod_v<Item>, Item> operator()(std::istream &is) const {
    Item item;
    is.read(reinterpret_cast<char *>(&item), sizeof(Item));
    return item;
  }
};

template<>
class DefaultItemDumper<std::string> {
 public:
  void operator()(std::ostream &os, const std::string &item) const {
    uint32_t sz = item.size();
    os.write(reinterpret_cast<const char *>(&sz), sizeof(sz));
    os.write(item.data(), item.size());
  }
};

template<>
class DefaultItemLoader<std::string> {
 public:
  std::string operator()(std::istream &is) const {
    uint32_t sz;
    is.read(reinterpret_cast<char *>(&sz), sizeof(sz));
    std::string result;
    result.resize(sz);
    is.read(result.data(), result.size());
    return result;
  }
};

}// namespace tc::data