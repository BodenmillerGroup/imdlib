cmake_minimum_required(VERSION 3.5)
project(imdlib)

option(BUILD_TEST "Build tests" OFF)
option(BUILD_PYTHON_MODULE "Build python module" ON)

include(GNUInstallDirs)

set(IMDLIB_VERSION_MAJOR 0)
set(IMDLIB_VERSION_MINOR 2)
set(IMDLIB_VERSION_PATCH 1)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES src/IMDFile.cpp src/IMDFileData.cpp)
set(HEADER_FILES src/IMDFile.h src/IMDFileData.h src/IMDFileIOException.h src/IMDFileMalformedException.h)

add_library(imd SHARED ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(imd PUBLIC pugixml)
set_target_properties(imd PROPERTIES VERSION ${IMDLIB_VERSION_MAJOR}.${IMDLIB_VERSION_MINOR}.${IMDLIB_VERSION_PATCH})

install(TARGETS imd LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

if (BUILD_TEST)
    add_subdirectory(test)
endif ()

if (BUILD_PYTHON_MODULE)
    add_subdirectory(python)
endif ()