#pragma once
#include <iostream>
#include <span>
#include <string>
#include <string_view>
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

template<typename T>
class DefaultItemMMapLoader {
 public:
  std::enable_if_t<std::is_pod_v<T>, std::pair<T, std::string_view>> operator()(std::string_view buf) const {
    T val;
    memcpy(&val, buf.data(), sizeof(val));
    buf = buf.substr(sizeof(val));
    return std::pair<T, std::string_view>{val, buf};
  }
};

template<>
class DefaultItemMMapLoader<std::string> {
 public:
  std::pair<std::string_view, std::string_view> operator()(std::string_view buf) const {
    DefaultItemMMapLoader<uint32_t> size_loader;
    auto tmp = size_loader(buf);
    buf = tmp.second;

    std::string_view result(buf.data(), tmp.first);
    buf = buf.substr(result.size());
    return std::pair<std::string_view, std::string_view>{result, buf};
  };
};

template<>
class DefaultItemMMapLoader<std::string_view> {
 public:
  std::pair<std::string_view, std::string_view> operator()(std::string_view buf) const {
    return DefaultItemMMapLoader<std::string>()(buf);
  }
};

template<>
class DefaultItemMMapLoader<std::u32string> {
 public:
  std::pair<std::u32string_view, std::string_view> operator()(std::string_view buf) const {
    DefaultItemMMapLoader<uint32_t> size_loader;
    auto [size, next] = size_loader(buf);
    buf = next;
    std::u32string_view sv(reinterpret_cast<const char32_t *>(buf.data()), size);
    buf = buf.substr(sizeof(char32_t) * sv.size());
    return std::pair<std::u32string_view, std::string_view>{sv, buf};
  };
};

template<>
class DefaultItemMMapLoader<std::u32string_view> {
 public:
  std::pair<std::u32string_view, std::string_view> operator()(std::string_view buf) const {
    return DefaultItemMMapLoader<std::u32string>()(buf);
  }
};

template<typename DistT>
class DefaultItemMMapLoader<std::span<const DistT>> {
 public:
  std::pair<std::span<const DistT>, std::string_view> operator()(std::string_view buf) const {
    DefaultItemMMapLoader<size_t> size_loader;
    auto [size, next] = size_loader(buf);
    buf = next;
    std::span<const DistT> result(reinterpret_cast<const DistT *>(buf.data()), size);
    buf = buf.substr(sizeof(DistT) * result.size());
    return std::pair<std::span<const DistT>, std::string_view>{result, buf};
  }
};

};// namespace tc::data