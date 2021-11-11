#include "catch2/catch.hpp"
#include "tc/data/in_memory_bktree.h"
#include "tc/data/mmap_bktree.h"
#include "tc/data/multithread_bktree.h"
#include <fstream>
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

TEST_CASE("mmap  bktree") {
  std::string dictionary[] = {"hell", "help", "shell", "smell",
                              "fell", "felt", "oops", "pop", "oouch", "halt"};
  InMemoryBKTree<std::string> tree;
  for (auto &item : dictionary) {
    tree.Add(item);
  }
  std::string buf;
  {
    std::ostringstream oss;
    tree.DumpMMap(oss);
    buf = oss.str();
  }
  MMapBKTree<std::string_view> mmapTree(buf);
  mmapTree.Search(std::string_view("helt"), 2, [](std::string_view w, size_t dist) {
    std::cout << w << ", " << dist << std::endl;
    return true;
  });
}
TEST_CASE("mmap file bktree") {
  std::string dictionary[] = {"hell", "help", "shell", "smell",
                              "fell", "felt", "oops", "pop", "oouch", "halt"};
  MultiThreadBKTree<InMemoryBKTree<std::string>> tree(2);
  for (auto &item : dictionary) {
    tree.Add(item);
  }
  {
    std::ofstream fout("mmap.bin");
    tree.DumpMMap(fout);
  }

  MultiThreadBKTree<MMapBKTreeHolder<std::string_view>> mmapTree;
  mmapTree.LoadMMap("mmap.bin");

  ThreadPool pool(1);
  mmapTree.Search(pool, std::string_view("helt"), 2, [](size_t pos, std::string_view w, size_t dist) {
    std::cout << w << ", " << dist << std::endl;
    return true;
  });
}

}// namespace tc::data