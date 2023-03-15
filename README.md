# HDF5 VOL Tests

This repository contains several test applications that exercise [HDF5](https://github.com/HDFGroup/hdf5)'s
public API and serve as regression tests for HDF5 [VOL Connectors](https://portal.hdfgroup.org/display/HDF5/Virtual+Object+Layer).

### Software Requirements

The following are required to build the HDF5 VOL tests:

- CMake 2.8.12.2 or newer
- HDF5 1.12.0 or newer

Note: The HDF5 VOL tests project is currently unsupported for building on Windows.

### Build Process

The HDF5 VOL tests are built using CMake in an out-of-source fashion. A typical build
would look like:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE:STRING=Release ${BUILD_OPTIONS} -G ${CMAKE_GENERATOR} ..
    make

Build time options that can be passed for `${BUILD_OPTIONS}` are discussed in the following
section. The CMake documentation should be consulted when determining what to set for
`CMAKE_GENERATOR`.

### Build Options

The following build options are available to influence how the VOL tests get built. To set an
option, it should be prepended with `-D` when passed to the `cmake` command. For example,

    cmake -DHDF5_DIR=/temp/hdf5 ..

would set the value of the `HDF5_DIR` build option to `/temp/hdf5`.

##### Required

`HDF5_DIR` - The VOL test build process will attempt to automatically locate an HDF5 installation
on the system. However, if one cannot be found, then this option should be set to point to a
directory which contains the CMake-related files for an HDF5 installation. For example, on linux
systems this directory will usually be:

    <hdf5_install_root>/share/cmake/hdf5/

and will contain `hdf5-config.cmake`, among other files.

##### Optional

`HDF5_VOL_TEST_ENABLE_PARALLEL` (Default: OFF) - This option enables parallel HDF5 tests and requires that the HDF5 installation used is built with the parallel library enabled.

`HDF5_VOL_TEST_ENABLE_ASYNC` (Default: OFF) - This option enables tests that use HDF5's asynchronous API routines. To enable these tests, the HDF5 installation used must be version 1.13 or newer.

`HDF5_VOL_TEST_ENABLE_PART` (Default: OFF) - This option enables building of the main test executable,
`h5vl_test`, as a set of individual executables, one per HDF5 'interface', rather than as a single executable.
This option is mostly helpful for CI integration, but otherwise is safe to leave off.

### Usage

The HDF5 VOL tests currently only support usage with HDF5 VOL connectors that can be loaded dynamically
as a plugin. For information on how to build a VOL connector in this manner, refer to section 2.3 of the
[HDF5 VOL Connector Author Guide](https://portal.hdfgroup.org/display/HDF5/HDF5+VOL+Connector+Authors+Guide?preview=/53610813/59903039/vol_connector_author_guide.pdf).

To use the HDF5 VOL tests with a particular VOL connector, first set the following two environment variables:

`HDF5_VOL_CONNECTOR` - This environment variable should be set to the name chosen for the VOL connector to
be used. For example, HDF5's DAOS VOL connector uses the name "[daos](https://github.com/HDFGroup/vol-daos/blob/master/src/daos_vol_public.h#L34)" and would therefore set:

    HDF5_VOL_CONNECTOR=daos

`HDF5_PLUGIN_PATH` - This environment variable should be set to the directory that contains the built library
for the VOL connector to be used.

Once these are set, the HDF5 VOL tests can be run by executing the following command from the build
directory:

    ctest . -VV

Individual test executables can also be manually run from the build directory.

If HDF5 is unable to locate or load the VOL connector specified, it will fall back to running the tests with
the native HDF5 VOL connector and an error similar to the following will appear in the test output:

    HDF5-DIAG: Error detected in HDF5 (1.13.0) MPI-process 0:
      #000: /home/user/git/hdf5/src/H5.c line 1010 in H5open(): library initialization failed
        major: Function entry/exit
        minor: Unable to initialize object
      #001: /home/user/git/hdf5/src/H5.c line 277 in H5_init_library(): unable to initialize vol interface
        major: Function entry/exit
        minor: Unable to initialize object
      #002: /home/user/git/hdf5/src/H5VLint.c line 199 in H5VL_init_phase2(): unable to set default VOL connector
        major: Virtual Object Layer
        minor: Can't set value
      #003: /home/user/git/hdf5/src/H5VLint.c line 429 in H5VL__set_def_conn(): can't register connector
        major: Virtual Object Layer
        minor: Unable to register new ID
      #004: /home/user/git/hdf5/src/H5VLint.c line 1321 in H5VL__register_connector_by_name(): unable to load VOL connector
        major: Virtual Object Layer
        minor: Unable to initialize object

### Help and Support

For help with building or using the HDF5 VOL tests, please contact the [HDF Help Desk](https://portal.hdfgroup.org/display/support/The+HDF+Help+Desk).