cmake_minimum_required(VERSION 3.12)
project(imdlib)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_PYTHON_MODULE "Build python module" ON)

set(IMDLIB_VERSION_MAJOR 0)
set(IMDLIB_VERSION_MINOR 3)
set(IMDLIB_VERSION_PATCH 0)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/IMDFile.cpp src/IMDData.cpp)
set(HEADER_FILES src/IMDFile.h src/IMDData.h src/IMDFileIOException.h src/IMDFileMalformedException.h)

add_library(imd SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(imd PUBLIC pugixml)
set_target_properties(imd PROPERTIES VERSION ${IMDLIB_VERSION_MAJOR}.${IMDLIB_VERSION_MINOR}.${IMDLIB_VERSION_PATCH})

install(TARGETS imd LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

if (BUILD_TESTS)
    add_subdirectory(test)
endif ()

if (BUILD_PYTHON_MODULE)
    add_subdirectory(python)
endif ()