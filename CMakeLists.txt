cmake_minimum_required(VERSION 2.8.12.2 FATAL_ERROR)
project(HDF5_VOL_TEST C)

#------------------------------------------------------------------------------
# Setup install and output Directories
#------------------------------------------------------------------------------

# Setting this ensures that "make install" will leave rpaths to external
# libraries intact on "make install". This ensures that one can install a
# version of HDF5 DAOS on the build machine without any issues. If this not
# desired, simply specify CMAKE_INSTALL_RPATH_USE_LINK_PATH when configuring
# HDF5 VOL DAOS and "make install" will strip all rpaths, which is default
# behavior.
#if(NOT CMAKE_INSTALL_RPATH_USE_LINK_PATH)
#  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#endif()

#------------------------------------------------------------------------------
# Set module path
#------------------------------------------------------------------------------
#set(HDF5_VOL_TEST_CMAKE_MODULE_PATH "${HDF5_VOL_TEST_SOURCE_DIR}/CMake")
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HDF5_VOL_TEST_CMAKE_MODULE_PATH})

#------------------------------------------------------------------------------
# Version information
#------------------------------------------------------------------------------
# TODO

#------------------------------------------------------------------------------
# Setup CMake Environment
#------------------------------------------------------------------------------
if(APPLE OR WIN32)
  message("The HDF5 VOL test suite is currently not supported on this platform." FATAL_ERROR)
endif()

#------------------------------------------------------------------------------
if(NOT HDF5_VOL_TEST_EXTERNALLY_CONFIGURED)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Executables."
  )
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all Libraries"
  )
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      ${PROJECT_BINARY_DIR}/bin CACHE PATH "Single Directory for all static libraries."
  )
endif()

#------------------------------------------------------------------------------
# Disallow in-source build
#------------------------------------------------------------------------------
if("${HDF5_VOL_TEST_SOURCE_DIR}" STREQUAL "${HDF5_VOL_TEST_BINARY_DIR}")
  message(FATAL_ERROR
    "HDF5 DAOS VOL requires an out of source Build. "
    "Please create a separate binary directory and run CMake there.")
endif()

#------------------------------------------------------------------------------
# Set a default build type if none was specified
#------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

if(NOT CMAKE_C_FLAGS AND CMAKE_COMPILER_IS_GNUCC)
  message(STATUS "GCC detected, setting additional flags")
  set(CMAKE_C_FLAGS "-Wall -Wextra -Winline -Wcast-qual -std=gnu99 -Wshadow" CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

#------------------------------------------------------------------------------
# Include source and build directories
#------------------------------------------------------------------------------
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

#------------------------------------------------------------------------------
# External dependencies
#------------------------------------------------------------------------------
# HDF5
# We will require 1.12 once it is released
find_package(HDF5 1.11 NO_MODULE NAMES hdf5 COMPONENTS C shared)
if(HDF5_FOUND)
  if(NOT TARGET ${HDF5_C_SHARED_LIBRARY})
      message(FATAL_ERROR "Could not find hdf5 shared target, please make "
      "sure that HDF5 has ben compiled with shared libraries enabled.")
  endif()
  set(HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES
    ${HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES}
    ${HDF5_C_SHARED_LIBRARY}
  )
else()
  # Allow for HDF5 autotools builds
  find_package(HDF5 1.11 MODULE REQUIRED)
  if(HDF5_FOUND)
    set(HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES
      ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
      ${HDF5_INCLUDE_DIRS}
    )
    set(HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES
      ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
      ${HDF5_LIBRARIES}
    )
  else()
    message(FATAL_ERROR "Could not find HDF5, please check HDF5_DIR.")
  endif()
endif()

option(HDF5_VOL_TEST_ENABLE_PARALLEL
  "Enable testing in parallel (requires MPI)." OFF)
if(HDF5_VOL_TEST_ENABLE_PARALLEL)
  set(H5VL_TEST_HAS_PARALLEL 1)
endif()
if(HDF5_VOL_TEST_ENABLE_PARALLEL OR HDF5_ENABLE_PARALLEL)
  # MPI
  # Temporary workaround because HDF5 does not pull MPI dependency
  if(NOT MPI_FOUND)
  find_package(MPI REQUIRED)
  endif()
  if(MPI_FOUND)
    set(HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES
      ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
      ${MPI_INCLUDE_PATH}
    )
    set(HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES
      ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
      ${MPI_LIBRARIES}
    )
  else()
    message(FATAL_ERROR "Could not find MPI.")
  endif()
endif()

#set(HDF5_VOL_TEST_INIT_COMMAND "" CACHE STRING
#  "Command to run before a test begins. Multiple commands are separated by ';'.")
#mark_as_advanced(HDF5_VOL_TEST_INIT_COMMAND)

#------------------------------------------------------------------------------
# Compile kwsys library and setup TestDriver
#------------------------------------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/h5vl_test_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/h5vl_test_config.h
)

add_subdirectory(driver)

#-----------------------------------------------------------------------------
# Define Sources and tests
#-----------------------------------------------------------------------------
set(vol_tests
  attribute
  dataset
  datatype
  file
  group
  link
  misc
  object
)

foreach(vol_test ${vol_tests})
  set(HDF5_VOL_TEST_SRCS
    ${HDF5_VOL_TEST_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/vol_${vol_test}_test.c
  )
endforeach()

add_executable(h5vl_test ${HDF5_VOL_TEST_SRCS} vol_test.c)
#  target_include_directories(h5vl_test
#    PUBLIC  "$<BUILD_INTERFACE:${HDF5_VOL_TEST_BUILD_INCLUDE_DEPENDENCIES}>"
#  )
target_include_directories(h5vl_test
  SYSTEM PUBLIC ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(h5vl_test
  ${HDF5_VOL_TEST_EXPORTED_LIBS}
  ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
  ${HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES}
)
if(NOT HDF5_VOL_TEST_SERVER)
  if(NOT HDF5_VOL_TEST_ENABLE_PARALLEL)
    add_test(NAME "h5vl_test"
      COMMAND $<TARGET_FILE:h5vl_test>
    )
  else()
    add_test(NAME "h5vl_test"
      COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS}
        ${MPIEXEC_PREFLAGS} $<TARGET_FILE:h5vl_test>
        ${MPIEXEC_POSTFLAGS}
    )
  endif()
else()
   # Dynamic client/server test
  add_test(NAME "h5vl_test"
    COMMAND $<TARGET_FILE:h5vl_test_driver>
    --server ${HDF5_VOL_TEST_SERVER}
    --client $<TARGET_FILE:h5vl_test>
  )
endif()
