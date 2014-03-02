cmake_minimum_required(VERSION 2.8)
project(diff_match_patch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_TESTS "Build tests" ON)

add_library(diff_match_patch diff_match_patch.cc)

INSTALL(
    TARGETS diff_match_patch LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
INSTALL(FILES diff_match_patch.h DESTINATION include)

if (BUILD_EXAMPLES)
  add_executable(example example.cc)
  target_link_libraries(example diff_match_patch)
endif ()

find_package(GTest)
if (BUILD_TESTS AND GTEST_FOUND)
  enable_testing()
  add_executable(diff_match_patch_test diff_match_patch_test.cc)
target_include_directories(diff_match_patch_test PRIVATE ${GTEST_INCLUDE_DIR})
  target_link_libraries(diff_match_patch_test diff_match_patch ${GTEST_LIBRARY}
                        ${GTEST_MAIN_LIBRARY})
  add_test(NAME diff_match_patch_test COMMAND diff_match_patch_test)
endif ()
