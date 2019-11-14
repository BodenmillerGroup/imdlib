find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

aux_source_directory(test TEST_FILES)
add_executable(imdtest IMDTest.cpp ${TEST_FILES})
target_link_libraries(imdtest PRIVATE imd ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(imdtest PRIVATE ../src ${GTEST_INCLUDE_DIRS})
