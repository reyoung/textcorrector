#include "ThreadPool.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "nlohmann/json.hpp"
#include "restinio/all.hpp"
#include "tc/data/in_memory_bktree.h"
#include "tc/data/multithread_bktree.h"
#include <codecvt>
#include <fstream>
#include <locale>
#include <thread>
#include <unordered_map>

DEFINE_uint32(port, 8080, "http port to listen");
DEFINE_uint32(num_worker_thread, 0, "thread for search worker, default is number of cpu");
DEFINE_uint32(num_request_thread, 1, "thread for request handler");
DEFINE_string(dicts, "", "dictionaries to load. the format is name1:file1,name2:file2...");

static std::vector<std::string> SplitStrBy(std::string str, char sep) {
  std::istringstream iss(str);
  std::string tmp;
  std::vector<std::string> result;
  while (std::getline(iss, tmp, sep)) {
    result.emplace_back(std::move(tmp));
  }
  return result;
}

int main(int argc, char **argv) {
  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (FLAGS_num_worker_thread == 0) {
    FLAGS_num_worker_thread = std::thread::hardware_concurrency();
  }

  using Tree = tc::data::MultiThreadBKTree<tc::data::InMemoryBKTree<std::u32string>>;
  std::unordered_map<std::string, Tree> trees;
  {
    auto dicts = SplitStrBy(FLAGS_dicts, ',');
    TC_ENFORCE(!dicts.empty()) << "--dicts must set";

    for (auto &dict : dicts) {
      auto item = SplitStrBy(dict, ':');
      TC_ENFORCE(item.size() == 2) << "--dicts must be DICT_NAME:DICT_PATH pairs";
      auto name = item[0];
      auto path = item[1];
      TC_ENFORCE(trees.find(name) == trees.end()) << "dict " << name << " already set";
      LOG(INFO) << "loading `" << name << "` from file " << path;
      auto &tree = trees[name];
      std::ifstream inFile(path.c_str());
      tree.Load(inFile);
      LOG(INFO) << name << " loaded";
    }
  }

  ThreadPool workerPool(FLAGS_num_worker_thread);
  ThreadPool reqPool(FLAGS_num_request_thread);

  try {
    // External io_context is necessary because we should have access to it.
    restinio::asio_ns::io_context ioctx;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv_utf8_utf32;
    restinio::run(
        ioctx,
        restinio::on_this_thread()
            .port(FLAGS_port)
            .address("0.0.0.0")
            .request_handler([&](restinio::request_handle_t req) {
              if (restinio::http_method_post() == req->header().method() && req->header().request_target() == "/") {
                nlohmann::json js;
                try {
                  js = nlohmann::json::parse(req->body());
                } catch (nlohmann::json::exception &) {
                  return req->create_response(restinio::status_bad_request()).set_body("cannot parse request as json").done();
                }
                std::string q;
                try {
                  q = js["q"];
                } catch (nlohmann::json::exception &) {
                  return req->create_response(restinio::status_bad_request()).set_body("q must set").done();
                }
                const Tree *tree;
                try {
                  std::string dict = js["dict"];
                  auto &t = trees.at(dict);
                  tree = &t;
                } catch (nlohmann::json::exception &) {
                  return req->create_response(restinio::status_bad_request()).set_body("dict must set").done();
                } catch (std::out_of_range &) {
                  return req->create_response(restinio::status_not_found()).set_body("dict not found").done();
                }
                size_t tolerance = 2;
                try {
                  tolerance = js["tolerance"];
                } catch (...) {}

                reqPool.enqueue([q, tree, req, tolerance, &workerPool, &conv_utf8_utf32] {
                  std::vector<nlohmann::json> response(tree->NumTrees());
                  auto u32q = conv_utf8_utf32.from_bytes(q);
                  tree->Search(workerPool, u32q, tolerance, [&response, &conv_utf8_utf32](size_t i, std::u32string_view val, size_t d) {
                    response[i].emplace_back(nlohmann::json{{conv_utf8_utf32.to_bytes(val.data(), val.data() + val.size()), d}});
                    return true;
                  });
                  nlohmann::json jsRsp;
                  for (auto &r : response) {
                    for (auto &item : r) {
                      jsRsp.emplace_back(std::move(item));
                    }
                  }
                  req->create_response(restinio::status_ok()).set_body(jsRsp.dump()).done();
                });

                return restinio::request_accepted();
              }
              return restinio::request_rejected();
            }));
  } catch (const std::exception &ex) {
    std::cerr << "Error: " << ex.what() << std::endl;
    return 1;
  }
}