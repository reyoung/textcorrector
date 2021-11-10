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
class DefaultItemDumper<std::u32string> {
 public:
  void operator()(std::ostream &os, const std::u32string &item) const {
    uint32_t sz = item.size();
    os.write(reinterpret_cast<const char *>(&sz), sizeof(sz));
    os.write(reinterpret_cast<const char *>(item.data()), item.size() * sizeof(std::u32string::value_type));
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

template<>
class DefaultItemLoader<std::u32string> {
 public:
  std::u32string operator()(std::istream &is) const {
    uint32_t sz;
    is.read(reinterpret_cast<char *>(&sz), sizeof(sz));
    std::u32string result;
    result.resize(sz);
    is.read(reinterpret_cast<char *>(result.data()), result.size() * sizeof(std::u32string::value_type));
    return result;
  }
};

}// namespace tc::data