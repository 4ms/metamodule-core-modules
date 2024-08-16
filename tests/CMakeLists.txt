cmake_minimum_required(VERSION 3.22)

project(CoreModules-tests)

FILE(GLOB TEST_SOURCES *.cc *.cpp)

add_executable(runtests
	${TEST_SOURCES}
	../moduleFactory.cc
)


target_compile_features(runtests PUBLIC cxx_std_23)

#TODO: use find_package() to find metamodule-core-interface
target_include_directories(runtests PRIVATE
	../..
	../../cpputil
	../../metamodule-core-interface
	../../../metamodule-plugin-sdk/metamodule-core-interface
)


target_compile_definitions(runtests PRIVATE TESTPROJECT)

