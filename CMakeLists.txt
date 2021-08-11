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
set(HDF5_VOL_TEST_CMAKE_MODULE_PATH "${HDF5_VOL_TEST_SOURCE_DIR}/CMake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HDF5_VOL_TEST_CMAKE_MODULE_PATH})

#------------------------------------------------------------------------------
# Version information
#------------------------------------------------------------------------------
# TODO

#------------------------------------------------------------------------------
# Setup CMake Environment
#------------------------------------------------------------------------------
if(WIN32)
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
    "The HDF5 VOL test suite requires an out of source Build. "
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

#-----------------------------------------------------------------------------
# Testing
#-----------------------------------------------------------------------------
if(NOT HDF5_VOL_TEST_EXTERNALLY_CONFIGURED)
  enable_testing()
  include(CTest)
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
find_package(HDF5 NO_MODULE NAMES hdf5 COMPONENTS C shared)
if(HDF5_FOUND)
  set(HDF5_C_SHARED_LIBRARY hdf5-shared)
  if(NOT TARGET ${HDF5_C_SHARED_LIBRARY})
      message(FATAL_ERROR "Could not find hdf5 shared target, please make "
      "sure that HDF5 has been compiled with shared libraries enabled.")
  endif()
  set(HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES
    ${HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES}
    ${HDF5_C_SHARED_LIBRARY}
  )
else()
  # Allow for HDF5 autotools builds
  find_package(HDF5 MODULE REQUIRED)
  set(HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES
    ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
    ${HDF5_INCLUDE_DIRS}
  )
  set(HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES
    ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
    ${HDF5_LIBRARIES}
  )
endif()

# HDF5 tests in separate tests
option(HDF5_VOL_TEST_ENABLE_PART
  "Enable testing in separate tests." OFF)

# HDF5 async tests
option(HDF5_VOL_TEST_ENABLE_ASYNC
  "Enable async API tests." OFF)
if(HDF5_VOL_TEST_ENABLE_ASYNC)
  set(H5VL_TEST_HAS_ASYNC 1)
endif()

# Parallel HDF5 tests
option(HDF5_VOL_TEST_ENABLE_PARALLEL
  "Enable testing in parallel (requires MPI)." OFF)
if(HDF5_VOL_TEST_ENABLE_PARALLEL)
  set(H5VL_TEST_HAS_PARALLEL 1)
endif()
if(HDF5_VOL_TEST_ENABLE_PARALLEL OR HDF5_ENABLE_PARALLEL)
  # MPI
  find_package(MPI REQUIRED)
  set(HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES
    ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
    ${MPI_INCLUDE_PATH}
  )
  set(HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES
    ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
    ${MPI_LIBRARIES}
  )
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

# Generic VOL tests
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

if(HDF5_VOL_TEST_ENABLE_ASYNC)
  set(vol_tests
    ${vol_tests}
    async
  )
endif()

# Ported HDF5 tests
set(hdf5_tests
  testhdf5
)
if(HDF5_VOL_TEST_ENABLE_PARALLEL)
  set(hdf5_partests
    t_bigio
    t_pshutdown
    t_shapesame
    testphdf5
  )
endif()

foreach(vol_test ${vol_tests})
  set(HDF5_VOL_TEST_SRCS
    ${HDF5_VOL_TEST_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/vol_${vol_test}_test.c
  )
  if(HDF5_VOL_TEST_ENABLE_PARALLEL)
    set(HDF5_VOL_TEST_PARALLEL_SRCS
      ${HDF5_VOL_TEST_PARALLEL_SRCS}
      ${CMAKE_CURRENT_SOURCE_DIR}/vol_${vol_test}_test_parallel.c
  )
  endif()
endforeach()

add_executable(h5vl_test ${HDF5_VOL_TEST_SRCS} vol_test.c vol_test_util.c)
if(HDF5_VOL_TEST_ENABLE_PARALLEL)
  add_executable(h5vl_test_parallel
    ${HDF5_VOL_TEST_PARALLEL_SRCS} vol_test_parallel.c vol_test_util.c)
endif()
#  target_include_directories(h5vl_test
#    PUBLIC  "$<BUILD_INTERFACE:${HDF5_VOL_TEST_BUILD_INCLUDE_DEPENDENCIES}>"
#  )
target_include_directories(h5vl_test
  SYSTEM PUBLIC ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
)
if(HDF5_VOL_TEST_ENABLE_PARALLEL)
  target_include_directories(h5vl_test_parallel
    SYSTEM PUBLIC ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
  )
endif()
target_link_libraries(h5vl_test
  ${HDF5_VOL_TEST_EXPORTED_LIBS}
  ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
  ${HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES}
)
if(HDF5_VOL_TEST_ENABLE_PARALLEL)
  target_link_libraries(h5vl_test_parallel
    ${HDF5_VOL_TEST_EXPORTED_LIBS}
    ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
    ${HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES}
  )
endif()

# Include the ported HDF5 tests

# Serial tests
foreach(hdf5_test ${hdf5_tests})
  if(${hdf5_test} STREQUAL "testhdf5")
    add_executable(h5_test_${hdf5_test}
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/${hdf5_test}.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tarray.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tattr.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tchecksum.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tconfig.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tcoords.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tfile.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tgenprop.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/th5o.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/th5s.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tid.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/titerate.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tmisc.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/trefer.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tselect.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/ttime.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tunicode.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tvlstr.c
      ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/tvltypes.c
    )
  else()
    add_executable(h5_test_${hdf5_test} ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_test/${hdf5_test}.c)
  endif()

  target_include_directories(h5_test_${hdf5_test}
    SYSTEM PUBLIC ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
  )
  target_link_libraries(h5_test_${hdf5_test}
    ${HDF5_VOL_TEST_EXPORTED_LIBS}
    ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
    ${HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES}
  )
endforeach()

# Parallel tests
if(HDF5_VOL_TEST_ENABLE_PARALLEL)
  foreach(hdf5_partest ${hdf5_partests})
    if(${hdf5_partest} STREQUAL "testphdf5")
      add_executable(h5_partest_${hdf5_partest}
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/${hdf5_partest}.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_ph5basic.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_file.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_dset.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_mdset.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_coll_chunk.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_span_tree.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_prop.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_file_image.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_coll_md_read.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_chunk_alloc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/t_filter_read.c
      )
    else()
      add_executable(h5_partest_${hdf5_partest}
        ${CMAKE_CURRENT_SOURCE_DIR}/hdf5_testpar/${hdf5_partest}.c)
    endif()

    target_include_directories(h5_partest_${hdf5_partest}
      SYSTEM PUBLIC ${HDF5_VOL_TEST_EXT_INCLUDE_DEPENDENCIES}
    )
    target_link_libraries(h5_partest_${hdf5_partest}
      ${HDF5_VOL_TEST_EXPORTED_LIBS}
      ${HDF5_VOL_TEST_EXT_LIB_DEPENDENCIES}
      ${HDF5_VOL_TEST_EXT_PKG_DEPENDENCIES}
    )
  endforeach()
endif()

#------------------------------------------------------------------------------
# Tests
#------------------------------------------------------------------------------
if(HDF5_VOL_TEST_SERVER)
  # Driver options
  if(HDF5_VOL_TEST_SERVER_ALLOW_ERRORS)
    set(HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS --allow-server-errors)
  endif()
  if(HDF5_VOL_TEST_CLIENT_HELPER)
    set(HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS  ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
      --client-helper ${HDF5_VOL_TEST_CLIENT_HELPER})
  endif()
  if(HDF5_VOL_TEST_CLIENT_INIT)
    set(HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS  ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
      --client-init ${HDF5_VOL_TEST_CLIENT_INIT})
  endif()

  # Serial dynamic client/server test
  if(NOT HDF5_VOL_TEST_ENABLE_PART)
    add_test(NAME "h5vl_test"
      COMMAND $<TARGET_FILE:h5vl_test_driver>
      --server ${HDF5_VOL_TEST_SERVER}
      --client $<TARGET_FILE:h5vl_test>
      --serial
      ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
    )
  else()
    foreach(vol_test ${vol_tests})
      add_test(NAME "h5vl_test_${vol_test}"
        COMMAND $<TARGET_FILE:h5vl_test_driver>
        --server ${HDF5_VOL_TEST_SERVER}
        --client $<TARGET_FILE:h5vl_test> ${vol_test}
        --serial
        ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
      )
    endforeach()
  endif()

  foreach(hdf5_test ${hdf5_tests})
    add_test(NAME "h5_test_${hdf5_test}"
      COMMAND $<TARGET_FILE:h5vl_test_driver>
      --server ${HDF5_VOL_TEST_SERVER}
      --client $<TARGET_FILE:h5_test_${hdf5_test}>
      --serial
      ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
    )
  endforeach()

  # Hook external tests to same test suite
  foreach(ext_vol_test ${HDF5_VOL_EXT_SERIAL_TESTS})
    add_test(NAME "h5vl_ext_${ext_vol_test}"
      COMMAND $<TARGET_FILE:h5vl_test_driver>
      --server ${HDF5_VOL_TEST_SERVER}
      --client $<TARGET_FILE:${ext_vol_test}>
      --serial
      ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
    )
  endforeach()

  # Parallel dynamic client/server test
  if(HDF5_VOL_TEST_ENABLE_PARALLEL)
    add_test(NAME "h5vl_test_parallel"
      COMMAND $<TARGET_FILE:h5vl_test_driver>
      --server ${HDF5_VOL_TEST_SERVER}
      --client $<TARGET_FILE:h5vl_test_parallel>
      ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
    )

    foreach(hdf5_partest ${hdf5_partests})
      add_test(NAME "h5_partest_${hdf5_partest}"
        COMMAND $<TARGET_FILE:h5vl_test_driver>
        --server ${HDF5_VOL_TEST_SERVER}
        --client $<TARGET_FILE:h5_partest_${hdf5_partest}>
        ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
      )
    endforeach()

    # Hook external tests to same test suite
    foreach(ext_vol_test ${HDF5_VOL_EXT_PARALLEL_TESTS})
      add_test(NAME "h5vl_ext_${ext_vol_test}"
        COMMAND $<TARGET_FILE:h5vl_test_driver>
        --server ${HDF5_VOL_TEST_SERVER}
        --client $<TARGET_FILE:${ext_vol_test}>
        ${HDF5_VOL_TEST_DRIVER_EXTRA_FLAGS}
      )
    endforeach()
  endif()
else()
  if(NOT HDF5_VOL_TEST_ENABLE_PART)
    add_test(NAME "h5vl_test"
      COMMAND $<TARGET_FILE:h5vl_test>
    )
  else()
    foreach(vol_test ${vol_tests})
      add_test(NAME "h5vl_test_${vol_test}"
        COMMAND $<TARGET_FILE:h5vl_test> ${vol_test}
      )
    endforeach()
  endif()

  foreach(hdf5_test ${hdf5_tests})
    add_test(NAME "h5_test_${hdf5_test}"
      COMMAND $<TARGET_FILE:h5_test_${hdf5_test}>
    )
  endforeach()

  if(HDF5_VOL_TEST_ENABLE_PARALLEL)
    add_test(NAME "h5vl_test_parallel"
      COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS}
        ${MPIEXEC_PREFLAGS} $<TARGET_FILE:h5vl_test_parallel>
        ${MPIEXEC_POSTFLAGS}
    )
    foreach(hdf5_partest ${hdf5_partests})
      add_test(NAME "h5_partest_${hdf5_partest}"
        COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS}
          ${MPIEXEC_PREFLAGS} $<TARGET_FILE:h5_partest_${hdf5_partest}>
          ${MPIEXEC_POSTFLAGS}
      )
    endforeach()
  endif()
endif()
