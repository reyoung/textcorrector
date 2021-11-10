add_library(tc_data in_memory_bktree.cpp)

target_link_libraries(tc_data PUBLIC tc_base TC_ThreadPool)

add_executable(tc_data_test edit_distance_test.cpp in_memory_bktree_test.cpp)
target_link_libraries(tc_data_test tc_data tc_test_main)