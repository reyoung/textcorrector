add_subdirectory(exceptxx)

add_library(TC_ThreadPool INTERFACE)
target_include_directories(TC_ThreadPool INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thread_pool)

add_library(restinio INTERFACE)
target_include_directories(restinio INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/restinio/dev/)