#include "tc/data/in_memory_bktree.h"
#include <codecvt>
#include <fstream>
#include <string>

template<typename Callback>
void TimeIt(std::string_view label, Callback callback) {
  auto now = std::chrono::steady_clock::now();
  callback();
  std::cerr << label << " time " << std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - now).count() << " sec\n";
}

int main(int argc, char **argv) {
  tc::data::InMemoryBKTree<std::u32string> tree;
  {
    std::ifstream inFile(argv[1]);
    TimeIt("load", [&] {
      tree.Load(inFile);
    });
  }
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv_utf8_utf32;
  size_t limit = 1;

  {
    std::string line;
    using Result = std::pair<std::u32string_view, size_t>;
    std::vector<Result> items;
    while (std::getline(std::cin, line)) {
      auto u32line = conv_utf8_utf32.from_bytes(line);
      items.clear();
      TimeIt("search", [&] {
        tree.Search(u32line, limit, [&items](std::u32string_view item, size_t d) {
          items.emplace_back(Result{item, d});
          return true;
        });
      });

      for (auto &[item, dist] : items) {
        std::cout << conv_utf8_utf32.to_bytes(item.data(), item.data() + item.size())
                  << ", " << dist << std::endl;
      }
    }
  }
}