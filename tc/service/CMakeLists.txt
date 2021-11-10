add_executable(tc_service main.cpp)
target_link_libraries(tc_service tc_data asio::asio_static gflags restinio glog::glog fmt::fmt http-parser::http_parser
        nlohmann_json::nlohmann_json)