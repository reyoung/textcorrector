#include "tc/data/in_memory_bktree.h"
#include "tc/data/multithread_bktree.h"
#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
int main() {
  std::string line;
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv_utf8_utf32;
  tc::data::MultiThreadBKTree<tc::data::InMemoryBKTree<std::u32string>> tree(10);
  while (std::getline(std::cin, line)) {
    tree.Add(conv_utf8_utf32.from_bytes(line));
  }
  tree.Dump(std::cout);
}