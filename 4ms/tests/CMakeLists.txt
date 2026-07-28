cmake_minimum_required(VERSION 3.22)

project(CoreModules-4ms-tests)

# The DSP under test is ~10x slower unoptimized. -ffp-contract=off keeps
# IEEE-exact per-operation float semantics so the EnOsc golden-master outputs
# stay bit-identical no matter the optimization level.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()
add_compile_options(-ffp-contract=off)

FILE(GLOB TEST_SOURCES *.cc *.cpp)

add_executable(runtests
	${TEST_SOURCES}
	../../moduleFactory.cc

	# Support sources for PEG/MPEG/DLD (mirrors ../CMakeLists.txt)
	../core/peg-common/peg_base.cc
	../core/peg-common/trigout.cc
	../core/peg-common/analog_conditioning.cc
	../core/peg-common/env_update.cc
	../core/peg-common/envelope_calcs.cc
	../core/peg-common/flash_user.cc
	../core/peg-common/params.cc
	../core/peg-common/pingable_env.cc
	../core/peg-common/dig_inouts.cc
	../core/peg-common/timers.cc
	../core/peg-common/calibration.cc
	../core/peg-common/env_transition.cc
	../core/peg-common/leds.cc
	../core/peg-common/pwm.cc
	../core/peg-common/shareddrv/debounced_digins.cc
	../core/peg-common/shareddrv/dac.cc
	../core/mpeg/envelope_calcs.cc
	../core/peg/envelope_calcs.cc
	../core/looping-delay/src/calibration_storage.cc
	../../register_module.cc
)

target_compile_features(runtests PUBLIC cxx_std_23)

target_include_directories(runtests PRIVATE
	stubs # shadows patch-serial/base64.hh to avoid compiling rapidyaml
    ..
	../core
	../core/tapo
	../core/alpaca/include
	../core/looping-delay/src
	../core/peg-common
	../core/peg-common/mocks
	../../..
	../../../cpputil
	../../../core-interface
	../../../../metamodule-plugin-sdk/core-interface
)
target_compile_definitions(runtests PRIVATE ALPACA_NO_PREFETCH)

# TODO: use find_package() to locate cpputil
add_subdirectory(../../../cpputil ${CMAKE_CURRENT_BINARY_DIR}/cpputil)

target_link_libraries(runtests PRIVATE cpputil)

target_compile_definitions(runtests PRIVATE TESTPROJECT)

