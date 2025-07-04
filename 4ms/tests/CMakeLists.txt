cmake_minimum_required(VERSION 3.22)

project(CoreModules-4ms-tests)

FILE(GLOB TEST_SOURCES *.cc *.cpp)

add_executable(runtests
	${TEST_SOURCES}
	../../moduleFactory.cc
)

target_compile_features(runtests PUBLIC cxx_std_23)

target_include_directories(runtests PRIVATE
    ..
	../core
	../../..
	../../../cpputil
	../../../core-interface
	../../../../metamodule-plugin-sdk/core-interface
)

# TODO: use find_package() to locate cpputil
add_subdirectory(../../../cpputil ${CMAKE_CURRENT_BINARY_DIR}/cpputil)

target_link_libraries(runtests PRIVATE cpputil)

target_compile_definitions(runtests PRIVATE TESTPROJECT)

