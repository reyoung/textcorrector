add_library(tc_data INTERFACE)

target_link_libraries(tc_data INTERFACE tc_base)

add_executable(tc_data_test edit_distance_test.cpp)
target_link_libraries(tc_data_test tc_data tc_test_main)