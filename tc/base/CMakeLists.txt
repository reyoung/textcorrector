add_library(tc_base except.cpp)
target_include_directories(tc_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../)
target_link_libraries(tc_base PUBLIC exceptxx::exceptxx )

add_library(tc_test_base test_main.cpp)
target_link_libraries(tc_test_base PUBLIC Catch2::Catch2)
add_executable(tc_except_test except_test.cpp)
target_link_libraries(tc_except_test tc_base tc_test_base)