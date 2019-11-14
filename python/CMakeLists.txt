set(PYBIND11_CPP_STANDARD -std=c++1z)

add_subdirectory(pybind11)
pybind11_add_module(imdpy PythonModule.cpp)
target_link_libraries(imdpy PRIVATE imd)
target_include_directories(imdpy PRIVATE ../src)

find_package (Python3)
install(TARGETS imdpy DESTINATION ${Python3_SITEARCH})