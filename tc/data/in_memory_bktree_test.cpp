#include "catch2/catch.hpp"
#include "tc/data/in_memory_bktree.h"
#include <iostream>
#include <sstream>
namespace tc::data {
TEST_CASE("in_memory bktree") {
  std::string dictionary[] = {"hell", "help", "shell", "smell",
                              "fell", "felt", "oops", "pop", "oouch", "halt"};
  InMemoryBKTree<std::string> tree;
  for (auto &item : dictionary) {
    tree.Add(item);
  }

  std::ostringstream oss;
  tree.Dump(oss);

  InMemoryBKTree<std::string> tree2;
  std::istringstream is(oss.str());
  tree2.Load(is);

  tree2.Search(std::string_view("helt"), 2, [](std::string_view w, size_t dist) {
    std::cout << w << ", " << dist << std::endl;
    return true;
  });
}
}// namespace tc::data