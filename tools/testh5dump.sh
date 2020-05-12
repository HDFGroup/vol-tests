#! /bin/sh
#
# Copyright by The HDF Group.
# Copyright by the Board of Trustees of the University of Illinois.
# All rights reserved.
#
# This file is part of HDF5.  The full HDF5 copyright notice, including
# terms governing use, modification, and redistribution, is contained in
# the COPYING file, which can be found at the root of the source code
# distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.
# If you do not have access to either file, you may request a copy from
# help@hdfgroup.org.
#
# Tests for the h5dump tool

# Assume we have access to zlib compression
USE_FILTER_DEFLATE="yes"

# Assume we are on a little-endian system
WORDS_BIGENDIAN="no"

TESTNAME=h5dump
EXIT_SUCCESS=0
EXIT_FAILURE=1

H5DUMP=h5dump               # The h5dump tool name
H5DIFF=h5diff               # The h5diff tool name
H5DELETE='h5delete -f'      # The h5delete tool name
H5REPACK=h5repack           # The h5repack tool name

# TODO: Unclear if we'll be keeping this.
# If we call it before repacking, okay, but if not it has to go.
H5IMPORT=h5import           # The h5import tool name

RM='rm -rf'
CMP='cmp'
DIFF='diff -c'
GREP='grep'
CP='cp'
DIRNAME='dirname'
LS='ls'
AWK='awk'

nerrors=0
verbose=yes

######################################################################
# Input files
# --------------------------------------------------------------------

# Where the tool's HDF5 input files are located
H5DUMP_TESTFILES_HDF5_DIR="./testfiles/hdf5"

# Where the tool's expected output files are located
H5DUMP_TESTFILES_OUT_DIR="./testfiles/expected/h5dump"

######################################################################
# Output files
# --------------------------------------------------------------------

# Where the text output goes
TEXT_OUTPUT_DIR=./h5dump_test_output

# Where the repacked HDF5 input files go
REPACK_OUTPUT_DIR=./h5dump_repack_output

######################################################################
# test files
# --------------------------------------------------------------------

# HDF5 test files.
#
# Kept in       $H5LS_TESTFILES_HDF5_DIR
# Repacked to   $REPACK_OUTPUT_DIR
#
# These files fail to repack w/ native VOL (investigate later)
# tall.h5
# tattrreg.h5 <-- This one does not even cause h5repack to fail!
# tbigdims.h5 (repack hangs w/ passthru VOL)
# err_attr_dspace.h5
# tfcontents1.h5
#
# These files have native-specific content
# tfamily00000.h5
# tfamily00001.h5
# tfamily00002.h5
# tfamily00003.h5
# tfamily00004.h5
# tfamily00005.h5
# tfamily00006.h5
# tfamily00007.h5
# tfamily00008.h5
# tfamily00009.h5
# tfamily00010.h5
# tfilters.h5
# filter_fail.h5 (can't be repacked w/o missing filter)
# tgrp_comments.h5
# tmulti-b.h5
# tmulti-g.h5
# tmulti-l.h5
# tmulti-o.h5
# tmulti-r.h5
# tmulti-s.h5
# tsplit_file-m.h5
# tsplit_file-r.h5
#
# Link files require an h5repack H5Lcopy work-around
# textlink.h5
# textlinksrc.h5
# textlinktar.h5
# tslink.h5
# tsoftlinks.h5
# tudlink.h5
HDF5_FILES="
charsets.h5
file_space.h5
packedbits.h5
t128bit_float.h5
taindices.h5
tarray1_big.h5
tarray1.h5
tarray2.h5
tarray3.h5
tarray4.h5
tarray5.h5
tarray6.h5
tarray7.h5
tarray8.h5
tattr2.h5
tattr4_be.h5
tattr.h5
tattrintsize.h5
tbinary.h5
tbitnopaque.h5
tchar.h5
tcmpdattrintsize.h5
tcmpdintsize.h5
tcompound_complex2.h5
tcompound_complex.h5
tcompound.h5
tdatareg.h5
tdset.h5
tempty.h5
tfcontents2.h5
tfpformat.h5
tfvalues.h5
tgroup.h5
tgrp_comments.h5
tgrpnullspace.h5
thlink.h5
thyperslab.h5
tints4dims.h5
tintsattrs.h5
tlarge_objname.h5
tldouble.h5
tloop.h5
tnamed_dtype_attr.h5
tnestedcmpddt.h5
tnestedcomp.h5
tno-subset.h5
tnullspace.h5
torderattr.h5
tordergr.h5
tsaf.h5
tscalarattrintsize.h5
tscalarintattrsize.h5
tscalarintsize.h5
tscalarstring.h5
tstr2.h5
tstr3.h5
tstr.h5
tvldtypes1.h5
tvldtypes2.h5
tvldtypes3.h5
tvldtypes4.h5
tvldtypes5.h5
tvlenstr_array.h5
tvlstr.h5
tvms.h5
zerodim.h5
"

# Expected output files.
#
# Kept in       $H5DUMP_TESTFILES_OUT_DIR
# Copied to     $TEXT_OUTPUT_DIR
#
# NOTE: This is ALL the files - they have not been culled based on the HDF5
#       files in the above list.
#
EXPECTED_OUTPUT_FILES="
charsets.ddl
err_attr_dspace.ddl
file_space.ddl
filter_fail.ddl
filter_fail.err
h5dump-help.txt
non_existing.ddl
non_existing.err
out3.h5import
packedbits.ddl
tall-1.ddl
tall-1.err
tall-2A0.ddl
tall-2A0.err
tall-2A.ddl
tall-2A.err
tall-2B.ddl
tall-2B.err
tall-2.ddl
tall-3.ddl
tall-4s.ddl
tall-5s.ddl
tall-6.ddl
tall-6.exp
tall-7.ddl
tall-7N.ddl
tallfilters.ddl
tarray1_big.ddl
tarray1_big.err
tarray1.ddl
tarray2.ddl
tarray3.ddl
tarray4.ddl
tarray5.ddl
tarray6.ddl
tarray7.ddl
tarray8.ddl
tattr-1.ddl
tattr-2.ddl
tattr-3.ddl
tattr-3.err
tattr-4_be.ddl
tattrcontents1.ddl
tattrcontents2.ddl
tattrintsize.ddl
tattrreg.ddl
tattrregR.ddl
tattrregR.err
tbigdims.ddl
tbin1.ddl
tbin1.ddl
tbin2.ddl
tbin3.ddl
tbin4.ddl
tbinregR.ddl
tbinregR.exp
tbitnopaque_be.ddl
tbitnopaque_le.ddl
tboot1.ddl
tboot2A.ddl
tboot2B.ddl
tboot2.ddl
tchar1.ddl
tchunked.ddl
tcmpdattrintsize.ddl
tcmpdintsize.ddl
tcomp-1.ddl
tcomp-2.ddl
tcomp-3.ddl
tcomp-3.err
tcomp-4.ddl
tcompact.ddl
tcompound_complex2.ddl
tcontents.ddl
tcontiguos.ddl
tdatareg.ddl
tdataregR.ddl
tdataregR.err
tdeflate.ddl
tdset-1.ddl
tdset-2.ddl
tdset-2.err
tdset-3s.ddl
tempty.ddl
texceedsubblock.ddl
texceedsubblock.err
texceedsubcount.ddl
texceedsubcount.err
texceedsubstart.ddl
texceedsubstart.err
texceedsubstride.ddl
texceedsubstride.err
texternal.ddl
textlink.ddl
textlink.err
textlinkfar.ddl
textlinkfar.err
textlinksrc.ddl
textlinksrc.err
tfamily.ddl
tfill.ddl
tfletcher32.ddl
tfpformat.ddl
tgroup-1.ddl
tgroup-2.ddl
tgroup-2.err
tgrp_comments.ddl
tgrpnullspace.ddl
thlink-1.ddl
thlink-2.ddl
thlink-3.ddl
thlink-4.ddl
thlink-5.ddl
thyperslab.ddl
tindicesno.ddl
tindicessub1.ddl
tindicessub2.ddl
tindicessub3.ddl
tindicessub4.ddl
tindicesyes.ddl
tints4dimsBlock2.ddl
tints4dimsBlockEq.ddl
tints4dimsCount2.ddl
tints4dimsCountEq.ddl
tints4dims.ddl
tints4dimsStride2.ddl
tintsattrs.ddl
tlarge_objname.ddl
tlonglinks.ddl
tloop-1.ddl
tmulti.ddl
tmultifile.ddl
tnamed_dtype_attr.ddl
tnbit.ddl
tnestcomp-1.ddl
tnestedcmpddt.ddl
tnoattrdata.ddl
tnoattrddl.ddl
tnodata.ddl
tnoddl.ddl
tnoddlfile.ddl
tnoddlfile.exp
tno-subset.ddl
tnullspace.ddl
torderattr1.ddl
torderattr2.ddl
torderattr3.ddl
torderattr4.ddl
tordercontents1.ddl
tordercontents2.ddl
tordergr1.ddl
tordergr2.ddl
tordergr3.ddl
tordergr4.ddl
tordergr5.ddl
torderlinks1.ddl
torderlinks1.err
torderlinks2.ddl
torderlinks2.err
tperror.ddl
tperror.err
tqmarkfile.ddl
tqmarkfile.err
trawdatafile.ddl
trawdatafile.exp
trawssetfile.ddl
trawssetfile.exp
treadfilter.ddl
treadintfilter.ddl
treference.ddl
tsaf.ddl
tscalarattrintsize.ddl
tscalarintattrsize.ddl
tscalarintsize.ddl
tscalarstring.ddl
tscaleoffset.ddl
tshuffle.ddl
tslink-1.ddl
tslink-2.ddl
tslink-D.ddl
tslink-D.err
tsplit_file.ddl
tstarfile.ddl
tstr-1.ddl
tstr2bin2.exp
tstr2bin6.exp
tstr-2.ddl
tstring2.ddl
tstring.ddl
tstringe.ddl
tszip.ddl
tudlink-1.ddl
tudlink-2.ddl
tuserfilter.ddl
tvldtypes1.ddl
tvldtypes2.ddl
tvldtypes3.ddl
tvldtypes4.ddl
tvldtypes5.ddl
tvlenstr_array.ddl
tvlstr.ddl
tvms.ddl
twidedisplay.ddl
twithddl.exp
twithddlfile.ddl
twithddlfile.exp
zerodim.ddl
"

######################################################################
# Utility functions
# --------------------------------------------------------------------

# Copy the expected text output files to the text output directory
# to make it easier to diff the expected and actual output.
#
COPY_EXPECTED_OUTPUT_FILES()
{
    for outfile in $EXPECTED_OUTPUT_FILES
    do
        filepath="$H5DUMP_TESTFILES_OUT_DIR/$outfile"

        # Use -f to make sure get a new copy
        $CP -f $filepath $TEXT_OUTPUT_DIR
        if [ $? -ne 0 ]; then
            echo "Error: FAILED to copy expected output file: $filepath ."
        fi
    done
}

# Repack the HDF5 files to the repack directory.
#
REPACK_HDF5_FILES()
{
    for repackfile in $HDF5_FILES
    do
        inpath="$H5DUMP_TESTFILES_HDF5_DIR/$repackfile"
        outpath="$REPACK_OUTPUT_DIR/$repackfile"

        # Repack the file
        $H5REPACK --src-vol-name=native --enable-error-stack $inpath $outpath
        if [ $? -ne 0 ]; then
            echo "Error: FAILED to repack HDF5 file: $inpath ."
        fi
    done
}

# Cleans up HDF5 and text output from the tests. Only cleans if the
# HDF5_NOCLEANUP variable is unset.
#
CLEAN_OUTPUT()
{
    # Remove output if the "no cleanup" environment variable is not defined
    if test -z "$HDF5_NOCLEANUP"; then
        # Text output
        $RM $TEXT_OUTPUT_DIR

        # HDF5 output
        #
        # Can't just rm -rf the directory if the VOL storage doesn't map to
        # a normal file, so we'll use h5delete to delete the file.
        for hdf5file in $HDF5_FILES
        do
            filepath="$REPACK_OUTPUT_DIR/$hdf5file"
            $H5DELETE $filepath
        done

        # The HDF5 output directory is always created, even if the VOL
        # storage won't use it. Delete it here.
        $RM $REPACK_OUTPUT_DIR
   fi
}

# Print a line-line message left justified in a field of 70 characters
# beginning with the word "Testing".
#
TESTING() {
   SPACES="                                                               "
   echo "Testing $* $SPACES" | cut -c1-70 | tr -d '\012'
}

# Source in the output filter function definitions.
. ./output_filter.sh

######################################################################
# Main testing functions
# --------------------------------------------------------------------

# Run a test and print PASS or *FAIL*.  If a test fails then increment
# the `nerrors' global variable and (if $verbose is set) display the
# difference between the actual output and the expected output. The
# expected output is given as the first argument to this function and
# the actual output file is calculated by replacing the `.ddl' with
# `.out'.  The actual output is not removed if $HDF5_NOCLEANUP has a
# non-zero value.
# If $1 == ignorecase then do caseless CMP and DIFF.
# ADD_H5_TEST
TOOLTEST() {
    # check if caseless compare and diff requested
    if [ "$1" = ignorecase ]; then
    caseless="-i"
    # replace cmp with diff which runs much longer.
    xCMP="$DIFF -i"
    shift
    else
    caseless=""
    # stick with faster cmp if ignorecase is not requested.
    xCMP="$CMP"
    fi

    expect="$TESTDIR/$1"
    actual="$TESTDIR/`basename $1 .ddl`.out"
    actual_err="$TESTDIR/`basename $1 .ddl`.err"
    actual_sav=${actual}-sav
    actual_err_sav=${actual_err}-sav
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    # save actual and actual_err in case they are needed later.
    cp $actual $actual_sav
    STDOUT_FILTER $actual
    cp $actual_err $actual_err_sav
    STDERR_FILTER $actual_err

  if [ ! -f $expect ]; then
    # Create the expect file if it doesn't yet exist.
     echo " CREATED"
     cp $actual $expect
     echo "    Expected result (*.ddl) missing"
     nerrors="`expr $nerrors + 1`"
    elif $xCMP $expect $actual > /dev/null 2>&1 ; then
     echo " PASSED"
    else
     echo "*FAILED*"
     echo "    Expected result (*.ddl) differs from actual result (*.out)"
     nerrors="`expr $nerrors + 1`"
     test yes = "$verbose" && $DIFF $caseless $expect $actual |sed 's/^/    /'
    fi
}


# same as TOOLTEST1 but compares generated file to expected output
#                   and compares the generated data file to the expected data file
# used for the binary tests that expect a full path in -o without -b
# ADD_H5_EXPORT_TEST
TOOLTEST2() {

    expectdata="$TESTDIR/$1"
    expect="$TESTDIR/`basename $1 .exp`.ddl"
    actualdata="$TESTDIR/`basename $1 .exp`.txt"
    actual="$TESTDIR/`basename $1 .exp`.out"
    actual_err="$TESTDIR/`basename $1 .exp`.err"
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    if [ ! -f $expect ]; then
    # Create the expect file if it doesn't yet exist.
     echo " CREATED"
     cp $actual $expect
     echo "    Expected result (*.ddl) missing"
     nerrors="`expr $nerrors + 1`"
    elif $CMP $expect $actual; then
      if [ ! -f $expectdata ]; then
      # Create the expect data file if it doesn't yet exist.
        echo " CREATED"
        cp $actualdata $expectdata
        echo "    Expected data (*.exp) missing"
        nerrors="`expr $nerrors + 1`"
      elif $CMP $expectdata $actualdata; then
        echo " PASSED"
      else
        echo "*FAILED*"
        echo "    Expected datafile (*.exp) differs from actual datafile (*.txt)"
        nerrors="`expr $nerrors + 1`"
        test yes = "$verbose" && $DIFF $expectdata $actualdata |sed 's/^/    /'
      fi
    else
     echo "*FAILED*"
     echo "    Expected result (*.ddl) differs from actual result (*.out)"
     nerrors="`expr $nerrors + 1`"
     test yes = "$verbose" && $DIFF $expect $actual |sed 's/^/    /'
    fi
}

# same as TOOLTEST2 but compares generated file to expected ddl file
#                   and compares the generated data file to the expected data file
# used for the binary tests that expect a full path in -o without -b
# ADD_H5_TEST_EXPORT
TOOLTEST2A() {

    expectdata="$TESTDIR/$1"
    expect="$TESTDIR/`basename $1 .exp`.ddl"
    actualdata="$TESTDIR/`basename $1 .exp`.txt"
    actual="$TESTDIR/`basename $1 .exp`.out"
    actual_err="$TESTDIR/`basename $1 .exp`.err"
    shift
    expectmeta="$TESTDIR/$1"
    actualmeta="$TESTDIR/`basename $1 .exp`.txt"
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    if [ ! -f $expect ]; then
    # Create the expect file if it doesn't yet exist.
     echo " CREATED"
     cp $actual $expect
     echo "    Expected result (*.ddl) missing"
     nerrors="`expr $nerrors + 1`"
    elif $CMP $expect $actual; then
      if [ ! -f $expectdata ]; then
      # Create the expect data file if it doesn't yet exist.
        echo " CREATED"
        cp $actualdata $expectdata
        echo "    Expected data (*.exp) missing"
        nerrors="`expr $nerrors + 1`"
      elif $DIFF $expectdata $actualdata; then
        if [ ! -f $expectmeta ]; then
        # Create the expect meta file if it doesn't yet exist.
          echo " CREATED"
          cp $actualmeta $expectmeta
          echo "    Expected metafile (*.ddl) missing"
          nerrors="`expr $nerrors + 1`"
        elif $CMP $expectmeta $actualmeta; then
          echo " PASSED"
        else
          echo "*FAILED*"
          echo "    Expected metafile (*.ddl) differs from actual metafile (*.txt)"
          nerrors="`expr $nerrors + 1`"
          test yes = "$verbose" && $DIFF $expectmeta $actualmeta |sed 's/^/    /'
        fi
      else
        echo "*FAILED*"
        echo "    Expected datafile (*.exp) differs from actual datafile (*.txt)"
        nerrors="`expr $nerrors + 1`"
        test yes = "$verbose" && $DIFF $expectdata $actualdata |sed 's/^/    /'
      fi
    else
     echo "*FAILED*"
     echo "    Expected result (*.ddl) differs from actual result (*.out)"
     nerrors="`expr $nerrors + 1`"
     test yes = "$verbose" && $DIFF $expect $actual |sed 's/^/    /'
    fi
}

# same as TOOLTEST2 but only compares the generated data file to the expected data file
# used for the binary tests that expect a full path in -o with -b
# ADD_H5_EXPORT_TEST
TOOLTEST2B() {

    expectdata="$TESTDIR/$1"
    actualdata="$TESTDIR/`basename $1 .exp`.txt"
    actual="$TESTDIR/`basename $1 .exp`.out"
    actual_err="$TESTDIR/`basename $1 .exp`.err"
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    if [ ! -f $expectdata ]; then
    # Create the expect data file if it doesn't yet exist.
      echo " CREATED"
      cp $actualdata $expectdata
      echo "    Expected data (*.exp) missing"
      nerrors="`expr $nerrors + 1`"
    elif $CMP $expectdata $actualdata; then
      echo " PASSED"
    else
      echo "*FAILED*"
      echo "    Expected datafile (*.exp) differs from actual datafile (*.txt)"
      nerrors="`expr $nerrors + 1`"
      test yes = "$verbose" && $DIFF $expectdata $actualdata |sed 's/^/    /'
    fi
}

# same as TOOLTEST but filters error stack outp
# Extract file name, line number, version and thread IDs because they may be different
TOOLTEST3() {

    expect="$TESTDIR/$1"
    actual="$TESTDIR/`basename $1 .ddl`.out"
    actual_err="$TESTDIR/`basename $1 .ddl`.err"
    actual_ext="$TESTDIR/`basename $1 .ddl`.ext"
    actual_sav=${actual}-sav
    actual_err_sav=${actual_err}-sav
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    # save actual and actual_err in case they are needed later.
    cp $actual $actual_sav
    STDOUT_FILTER $actual
    cp $actual_err $actual_err_sav
    STDERR_FILTER $actual_err

    # Extract file name, line number, version and thread IDs because they may be different
    sed -e 's/thread [0-9]*/thread (IDs)/' -e 's/: .*\.c /: (file name) /' \
        -e 's/line [0-9]*/line (number)/' \
        -e 's/v[1-9]*\.[0-9]*\./version (number)\./' \
        -e 's/[1-9]*\.[0-9]*\.[0-9]*[^)]*/version (number)/' \
        -e 's/H5Eget_auto[1-2]*/H5Eget_auto(1 or 2)/' \
        -e 's/H5Eset_auto[1-2]*/H5Eset_auto(1 or 2)/' \
     $actual_err > $actual_ext

    if [ ! -f $expect ]; then
    # Create the expect file if it doesn't yet exist.
     echo " CREATED"
     cp $actual $expect
     echo "    Expected result (*.ddl) missing"
     nerrors="`expr $nerrors + 1`"
    elif $CMP $expect $actual; then
     echo " PASSED"
    else
     echo "*FAILED*"
     echo "    Expected result (*.ddl) differs from actual result (*.out)"
     nerrors="`expr $nerrors + 1`"
     test yes = "$verbose" && $DIFF $expect $actual |sed 's/^/    /'
    fi
}

# same as TOOLTEST3 but filters error stack output and compares to an error file
# Extract file name, line number, version and thread IDs because they may be different
# ADD_H5ERR_MASK_TEST
TOOLTEST4() {

    expect="$TESTDIR/$1"
    expect_err="$TESTDIR/`basename $1 .ddl`.err"
    actual="$TESTDIR/`basename $1 .ddl`.out"
    actual_err="$TESTDIR/`basename $1 .ddl`.oerr"
    actual_ext="$TESTDIR/`basename $1 .ddl`.ext"
    actual_sav=${actual}-sav
    actual_err_sav=${actual_err}-sav
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $ENVCMD $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    # save actual and actual_err in case they are needed later.
    cp $actual $actual_sav
    STDOUT_FILTER $actual
    cp $actual_err $actual_err_sav
    STDERR_FILTER $actual_err

    # Extract file name, line number, version and thread IDs because they may be different
    sed -e 's/thread [0-9]*/thread (IDs)/' -e 's/: .*\.c /: (file name) /' \
        -e 's/line [0-9]*/line (number)/' \
        -e 's/v[1-9]*\.[0-9]*\./version (number)\./' \
        -e 's/[1-9]*\.[0-9]*\.[0-9]*[^)]*/version (number)/' \
        -e 's/H5Eget_auto[1-2]*/H5Eget_auto(1 or 2)/' \
        -e 's/H5Eset_auto[1-2]*/H5Eset_auto(1 or 2)/' \
     $actual_err > $actual_ext

    if [ ! -f $expect ]; then
    # Create the expect file if it doesn't yet exist.
     echo " CREATED"
     cp $actual $expect
     echo "    Expected result (*.ddl) missing"
     nerrors="`expr $nerrors + 1`"
    elif $CMP $expect $actual; then
     if $CMP $expect_err $actual_ext; then
      echo " PASSED"
     else
      echo "*FAILED*"
      echo "    Expected result (*.err) differs from actual result (*.oerr)"
      nerrors="`expr $nerrors + 1`"
      test yes = "$verbose" && $DIFF $expect_err $actual_ext |sed 's/^/    /'
     fi
    else
     echo "*FAILED*"
     echo "    Expected result (*.ddl) differs from actual result (*.out)"
     nerrors="`expr $nerrors + 1`"
     test yes = "$verbose" && $DIFF $expect $actual |sed 's/^/    /'
    fi
}

# same as TOOLTEST4 but disables plugin filter loading
# silences extra error output on some platforms
# ADD_H5ERR_MASK_TEST
TOOLTEST5() {

    expect="$TESTDIR/$1"
    expect_err="$TESTDIR/`basename $1 .ddl`.err"
    actual="$TESTDIR/`basename $1 .ddl`.out"
    actual_err="$TESTDIR/`basename $1 .ddl`.oerr"
    actual_ext="$TESTDIR/`basename $1 .ddl`.ext"
    actual_sav=${actual}-sav
    actual_err_sav=${actual_err}-sav
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $ENVCMD $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    # save actual and actual_err in case they are needed later.
    cp $actual $actual_sav
    STDOUT_FILTER $actual
    cp $actual_err $actual_err_sav
    STDERR_FILTER $actual_err

    # Extract file name, line number, version and thread IDs because they may be different
    sed -e 's/thread [0-9]*/thread (IDs)/' -e 's/: .*\.c /: (file name) /' \
        -e 's/line [0-9]*/line (number)/' \
        -e 's/v[1-9]*\.[0-9]*\./version (number)\./' \
        -e 's/[1-9]*\.[0-9]*\.[0-9]*[^)]*/version (number)/' \
        -e 's/H5Eget_auto[1-2]*/H5Eget_auto(1 or 2)/' \
        -e 's/H5Eset_auto[1-2]*/H5Eset_auto(1 or 2)/' \
     $actual_err > $actual_ext

    if [ ! -f $expect ]; then
    # Create the expect file if it doesn't yet exist.
     echo " CREATED"
     cp $actual $expect
     echo "    Expected result (*.ddl) missing"
     nerrors="`expr $nerrors + 1`"
    elif $CMP $expect $actual; then
     if $CMP $expect_err $actual_ext; then
      echo " PASSED"
     else
      echo "*FAILED*"
      echo "    Expected result (*.err) differs from actual result (*.oerr)"
      nerrors="`expr $nerrors + 1`"
      test yes = "$verbose" && $DIFF $expect_err $actual_ext |sed 's/^/    /'
     fi
    else
     echo "*FAILED*"
     echo "    Expected result (*.ddl) differs from actual result (*.out)"
     nerrors="`expr $nerrors + 1`"
     test yes = "$verbose" && $DIFF $expect $actual |sed 's/^/    /'
    fi
}

# ADD_HELP_TEST
TOOLTEST_HELP() {

    expect="$TEXT_OUTPUT_DIR/$1"
    actual="$TEXT_OUTPUT_DIR/`basename $1 .txt`.out"
    actual_err="$TEXT_OUTPUT_DIR/`basename $1 .txt`.err"
    shift

    # Run test.
    TESTING $H5DUMP $@
    (
      $RUNSERIAL $H5DUMP "$@"
    ) >$actual 2>$actual_err

    if [ ! -f $expectdata ]; then
    # Create the expect data file if it doesn't yet exist.
      echo " CREATED"
      cp $actual $expect-CREATED
      echo "    Expected output (*.txt) missing"
      nerrors="`expr $nerrors + 1`"
    elif $CMP $expect $actual; then
      echo " PASSED"
    else
      echo "*FAILED*"
      echo "    Expected output (*.txt) differs from actual output (*.out)"
      nerrors="`expr $nerrors + 1`"
    fi
}

# Call the h5dump tool and grep for a value
# txttype ERRTXT greps test error output, otherwise greps test output
GREPTEST()
{
    txttype=$1
    expectdata=$2
    actual=$TESTDIR/$3
    actual_err="$TESTDIR/`basename $3 .ddl`.oerr"
    shift
    shift
    shift

    # Run test.
    TESTING $H5DUMP -p $@
    (
      $ENVCMD $RUNSERIAL $H5DUMP -p "$@"
    ) >$actual 2>$actual_err
    if [ "$txttype" = "ERRTXT" ]; then
        $GREP "$expectdata" $actual_err > /dev/null
    else
        $GREP "$expectdata" $actual > /dev/null
    fi
    if [ $? -eq 0 ]; then
        echo " PASSED"
    else
        echo " FAILED"
        nerrors="`expr $nerrors + 1`"
    fi
}

# Call the h5dump tool and grep for a value but disables plugin filter loading
# txttype ERRTXT greps test error output, otherwise greps test output
GREPTEST2()
{
    txttype=$1
    expectdata=$2
    actual=$TESTDIR/$3
    actual_err="$TESTDIR/`basename $3 .ddl`.oerr"
    shift
    shift
    shift

    # Run test.
    TESTING $H5DUMP -p $@
    (
      $ENVCMD $RUNSERIAL $H5DUMP -p "$@"
    ) >$actual 2>$actual_err
    if [ "$txttype" = "ERRTXT" ]; then
        $GREP "$expectdata" $actual_err > /dev/null
    else
        $GREP "$expectdata" $actual > /dev/null
    fi
    if [ $? -eq 0 ]; then
        echo " PASSED"
    else
        echo " FAILED"
        nerrors="`expr $nerrors + 1`"
    fi
}

# Print a "SKIP" message
SKIP() {
   TESTING $H5DUMP $@
    echo  " -SKIP-"
}

# Print a line-line message left justified in a field of 70 characters
#
PRINT_H5DIFF() {
 SPACES="                                                               "
 echo " Running h5diff $* $SPACES" | cut -c1-70 | tr -d '\012'
}


# Call the h5diff tool
#
DIFFTEST()
{
    PRINT_H5DIFF  $@
    (
  $RUNSERIAL $H5DIFF "$@" -q
    )
    RET=$?
    if [ $RET != 0 ] ; then
         echo "*FAILED*"
         nerrors="`expr $nerrors + 1`"
    else
         echo " PASSED"
    fi

}

# Print a line-line message left justified in a field of 70 characters
# beginning with the word "Verifying".
#
PRINT_H5IMPORT() {
 SPACES="                                                               "
 echo " Running h5import $* $SPACES" | cut -c1-70 | tr -d '\012'
}

# Call the h5import tool
#
IMPORTTEST()
{
    # remove the output hdf5 file if it exists
    hdf5_file="$TESTDIR/$5"
    if [ -f $hdf5_file ]; then
     rm -f $hdf5_file
    fi

    PRINT_H5IMPORT  $@
    (
  $RUNSERIAL $H5IMPORT "$@"
    )
    RET=$?
    if [ $RET != 0 ] ; then
         echo "*FAILED*"
         nerrors="`expr $nerrors + 1`"
    else
         echo " PASSED"
    fi
}


##############################################################################
##############################################################################
###        T H E   T E S T S                                               ###
##############################################################################
##############################################################################
# Prepare for test
#
# We create the repack output dir in case it's needed. If a VOL connector doesn't
# use normal files, it'll just stay empty and get deleted later.
CLEAN_OUTPUT
test -d $TEXT_OUTPUT_DIR || mkdir -p $TEXT_OUTPUT_DIR
test -d $REPACK_OUTPUT_DIR || mkdir -p $REPACK_OUTPUT_DIR
COPY_EXPECTED_OUTPUT_FILES
REPACK_HDF5_FILES


# Run h5dump tests

# test the help syntax
TOOLTEST_HELP h5dump-help.txt -h

: <<'END'
# test data output redirection
TOOLTEST tnoddl.ddl --enable-error-stack --ddl -y packedbits.h5
TOOLTEST tnodata.ddl --enable-error-stack --output packedbits.h5
TOOLTEST tnoattrddl.ddl --enable-error-stack -O -y tattr.h5
TOOLTEST tnoattrdata.ddl --enable-error-stack -A -o tattr.h5
TOOLTEST2 trawdatafile.exp --enable-error-stack -y -o trawdatafile.txt packedbits.h5
TOOLTEST2 tnoddlfile.exp --enable-error-stack -O -y -o tnoddlfile.txt packedbits.h5
TOOLTEST2A twithddlfile.exp twithddl.exp --enable-error-stack --ddl=twithddl.txt -y -o twithddlfile.txt packedbits.h5
TOOLTEST2 trawssetfile.exp --enable-error-stack -d "/dset1[1,1;;;]" -y -o trawssetfile.txt tdset.h5

# test for maximum display datasets
TOOLTEST twidedisplay.ddl --enable-error-stack -w0 packedbits.h5

# test for signed/unsigned datasets
TOOLTEST packedbits.ddl --enable-error-stack packedbits.h5
# test for compound signed/unsigned datasets
TOOLTEST tcmpdintsize.ddl --enable-error-stack tcmpdintsize.h5
# test for signed/unsigned scalar datasets
TOOLTEST tscalarintsize.ddl --enable-error-stack tscalarintsize.h5
# test for signed/unsigned attributes
TOOLTEST tattrintsize.ddl --enable-error-stack tattrintsize.h5
# test for compound signed/unsigned attributes
TOOLTEST tcmpdattrintsize.ddl --enable-error-stack tcmpdattrintsize.h5
# test for signed/unsigned scalar attributes
TOOLTEST tscalarattrintsize.ddl --enable-error-stack tscalarattrintsize.h5
# test for signed/unsigned scalar datasets with attributes
TOOLTEST tscalarintattrsize.ddl --enable-error-stack tscalarintattrsize.h5
# test for signed/unsigned datasets attributes
TOOLTEST tintsattrs.ddl --enable-error-stack tintsattrs.h5
# test for string scalar dataset attribute
TOOLTEST tscalarstring.ddl --enable-error-stack tscalarstring.h5
# test for displaying groups
TOOLTEST tgroup-1.ddl --enable-error-stack tgroup.h5
# test for displaying the selected groups
TOOLTEST4 tgroup-2.ddl --enable-error-stack --group=/g2 --group / -g /y tgroup.h5

# test for displaying simple space datasets
TOOLTEST tdset-1.ddl --enable-error-stack tdset.h5
# test for displaying selected datasets
TOOLTEST4 tdset-2.ddl --enable-error-stack -H -d dset1 -d /dset2 --dataset=dset3 tdset.h5

# test for displaying attributes
TOOLTEST tattr-1.ddl --enable-error-stack tattr.h5
# test for displaying the selected attributes of string type and scalar space
TOOLTEST tattr-2.ddl --enable-error-stack -a "/\/attr1" --attribute /attr4 --attribute=/attr5 tattr.h5
TOOLTEST tattr-2.ddl --enable-error-stack -N "/\/attr1" --any_path /attr4 --any_path=/attr5 tattr.h5
# test for header and error messages
TOOLTEST4 tattr-3.ddl --enable-error-stack --header -a /attr2 --attribute=/attr tattr.h5
# test for displaying at least 9 attributes on root from a BE machine
TOOLTEST tattr-4_be.ddl --enable-error-stack tattr4_be.h5
# test for displaying attributes in shared datatype (also in group and dataset)
TOOLTEST tnamed_dtype_attr.ddl --enable-error-stack tnamed_dtype_attr.h5

# test for displaying soft links and user-defined links
TOOLTEST tslink-1.ddl --enable-error-stack tslink.h5
TOOLTEST tudlink-1.ddl --enable-error-stack tudlink.h5
# test for displaying the selected link
TOOLTEST tslink-2.ddl --enable-error-stack -l slink2 tslink.h5
TOOLTEST tslink-2.ddl --enable-error-stack -N slink2 tslink.h5
TOOLTEST tudlink-2.ddl --enable-error-stack -l udlink2 tudlink.h5
# test for displaying dangling soft links
TOOLTEST4 tslink-D.ddl --enable-error-stack -d /slink1 tslink.h5

# tests for hard links
TOOLTEST thlink-1.ddl --enable-error-stack thlink.h5
TOOLTEST thlink-2.ddl --enable-error-stack -d /g1/dset2 --dataset /dset1 --dataset=/g1/g1.1/dset3 thlink.h5
TOOLTEST thlink-3.ddl --enable-error-stack -d /g1/g1.1/dset3 --dataset /g1/dset2 --dataset=/dset1 thlink.h5
TOOLTEST thlink-4.ddl --enable-error-stack -g /g1 thlink.h5
TOOLTEST thlink-4.ddl --enable-error-stack -N /g1 thlink.h5
TOOLTEST thlink-5.ddl --enable-error-stack -d /dset1 -g /g2 -d /g1/dset2 thlink.h5
TOOLTEST thlink-5.ddl --enable-error-stack -N /dset1 -N /g2 -N /g1/dset2 thlink.h5

# tests for compound data types
TOOLTEST tcomp-1.ddl --enable-error-stack tcompound.h5
# test for named data types
TOOLTEST tcomp-2.ddl --enable-error-stack -t /type1 --datatype /type2 --datatype=/group1/type3 tcompound.h5
TOOLTEST tcomp-2.ddl --enable-error-stack -N /type1 --any_path /type2 --any_path=/group1/type3 tcompound.h5
# test for unamed type
TOOLTEST4 tcomp-3.ddl --enable-error-stack -t /#6632 -g /group2 tcompound.h5
# test complicated compound datatype
TOOLTEST tcomp-4.ddl --enable-error-stack tcompound_complex.h5
TOOLTEST tcompound_complex2.ddl --enable-error-stack tcompound_complex2.h5
# tests for bitfields and opaque data types
if test $WORDS_BIGENDIAN != "yes"; then
TOOLTEST tbitnopaque_le.ddl --enable-error-stack tbitnopaque.h5
else
TOOLTEST tbitnopaque_be.ddl --enable-error-stack tbitnopaque.h5
fi

#test for the nested compound type
TOOLTEST tnestcomp-1.ddl --enable-error-stack tnestedcomp.h5
TOOLTEST tnestedcmpddt.ddl --enable-error-stack tnestedcmpddt.h5

# test for options
TOOLTEST4 tall-1.ddl --enable-error-stack tall.h5
TOOLTEST tall-2.ddl --enable-error-stack --header -g /g1/g1.1 -a attr2 tall.h5
TOOLTEST tall-3.ddl --enable-error-stack -d /g2/dset2.1 -l /g1/g1.2/g1.2.1/slink tall.h5
TOOLTEST tall-3.ddl --enable-error-stack -N /g2/dset2.1 -N /g1/g1.2/g1.2.1/slink tall.h5
TOOLTEST tall-7.ddl --enable-error-stack -a attr1 tall.h5
TOOLTEST tall-7N.ddl --enable-error-stack -N attr1 tall.h5

# test for loop detection
TOOLTEST tloop-1.ddl --enable-error-stack tloop.h5

# test for string
TOOLTEST tstr-1.ddl --enable-error-stack tstr.h5
TOOLTEST tstr-2.ddl --enable-error-stack tstr2.h5

# test for file created by Lib SAF team
TOOLTEST tsaf.ddl --enable-error-stack tsaf.h5

# test for file with variable length data
TOOLTEST tvldtypes1.ddl --enable-error-stack tvldtypes1.h5
TOOLTEST tvldtypes2.ddl --enable-error-stack tvldtypes2.h5
TOOLTEST tvldtypes3.ddl --enable-error-stack tvldtypes3.h5
TOOLTEST tvldtypes4.ddl --enable-error-stack tvldtypes4.h5
TOOLTEST tvldtypes5.ddl --enable-error-stack tvldtypes5.h5

#test for file with variable length string data
TOOLTEST tvlstr.ddl --enable-error-stack tvlstr.h5
TOOLTEST tvlenstr_array.ddl --enable-error-stack tvlenstr_array.h5

# test for files with array data
TOOLTEST tarray1.ddl --enable-error-stack tarray1.h5
# # added for bug# 2092 - tarray1_big.h
GREPTEST ERRTXT "NULL token size" tarray1_big.ddl --enable-error-stack -R tarray1_big.h5
TOOLTEST tarray2.ddl --enable-error-stack tarray2.h5
TOOLTEST tarray3.ddl --enable-error-stack tarray3.h5
TOOLTEST tarray4.ddl --enable-error-stack tarray4.h5
TOOLTEST tarray5.ddl --enable-error-stack tarray5.h5
TOOLTEST tarray6.ddl --enable-error-stack tarray6.h5
TOOLTEST tarray7.ddl --enable-error-stack tarray7.h5
TOOLTEST tarray8.ddl --enable-error-stack tarray8.h5

# test for wildcards in filename (does not work with cmake)
# inconsistent across platforms TOOLTEST3 tstarfile.ddl --enable-error-stack -H -d Dataset1 tarr*.h5
#TOOLTEST4 tqmarkfile.ddl --enable-error-stack -H -d Dataset1 tarray?.h5
TOOLTEST tmultifile.ddl --enable-error-stack -H -d Dataset1 tarray2.h5 tarray3.h5 tarray4.h5 tarray5.h5 tarray6.h5 tarray7.h5

# test for files with empty data
TOOLTEST tempty.ddl --enable-error-stack tempty.h5

# test for files with groups that have comments
TOOLTEST tgrp_comments.ddl --enable-error-stack tgrp_comments.h5

# test the --filedriver flag
TOOLTEST tsplit_file.ddl --enable-error-stack --filedriver=split tsplit_file
TOOLTEST tfamily.ddl --enable-error-stack --filedriver=family tfamily%05d.h5
TOOLTEST tmulti.ddl --enable-error-stack --filedriver=multi tmulti

# test for files with group names which reach > 1024 bytes in size
TOOLTEST tlarge_objname.ddl --enable-error-stack -w157 tlarge_objname.h5

# test '-A' to suppress data but print attr's
TOOLTEST4 tall-2A.ddl --enable-error-stack -A tall.h5

# test '-A' to suppress attr's but print data
TOOLTEST4 tall-2A0.ddl --enable-error-stack -A 0 tall.h5

# test '-r' to print attributes in ASCII instead of decimal
TOOLTEST4 tall-2B.ddl --enable-error-stack -A -r tall.h5

# test Subsetting
TOOLTEST tall-4s.ddl --enable-error-stack --dataset=/g1/g1.1/dset1.1.1 --start=1,1 --stride=2,3 --count=3,2 --block=1,1 tall.h5
TOOLTEST tall-5s.ddl --enable-error-stack -d "/g1/g1.1/dset1.1.2[0;2;10;]" tall.h5
TOOLTEST tdset-3s.ddl --enable-error-stack -d "/dset1[1,1;;;]" tdset.h5
TOOLTEST tno-subset.ddl --enable-error-stack --no-compact-subset -d "AHFINDERDIRECT::ah_centroid_t[0] it=0 tl=0" tno-subset.h5

TOOLTEST tints4dimsCount2.ddl --enable-error-stack -d FourDimInts -s 0,0,0,0 -c 2,2,2,2 tints4dims.h5
TOOLTEST tints4dimsBlock2.ddl --enable-error-stack -d FourDimInts -s 0,0,0,0 -c 1,1,1,1 -k 2,2,2,2 tints4dims.h5
TOOLTEST tints4dimsStride2.ddl --enable-error-stack -d FourDimInts -s 0,0,0,0 -S 2,2,2,2 -c 2,2,2,2 tints4dims.h5
TOOLTEST tints4dimsCountEq.ddl --enable-error-stack -d FourDimInts -s 0,0,0,0 -S 2,2,1,1 -k 1,2,1,1 -c 2,2,4,4 tints4dims.h5
TOOLTEST tints4dimsBlockEq.ddl --enable-error-stack -d FourDimInts -s 0,0,0,0 -S 2,2,1,1 -c 2,2,1,1 -k 1,2,4,4 tints4dims.h5

# test printing characters in ASCII instead of decimal
TOOLTEST tchar1.ddl --enable-error-stack -r tchar.h5

# test datatypes in ASCII and UTF8
TOOLTEST charsets.ddl --enable-error-stack charsets.h5

# rev. 2004

# tests for super block
TOOLTEST tboot1.ddl --enable-error-stack -H -B -d dset tfcontents1.h5
TOOLTEST tboot2.ddl --enable-error-stack -B tfcontents2.h5
TOOLTEST tboot2A.ddl --enable-error-stack --boot-block tfcontents2.h5
TOOLTEST tboot2B.ddl --enable-error-stack --superblock tfcontents2.h5
TOOLTEST file_space.ddl --enable-error-stack -B file_space.h5

# test -p with a non existing dataset
TOOLTEST4 tperror.ddl --enable-error-stack -p -d bogus tfcontents1.h5

# test for file contents
TOOLTEST tcontents.ddl --enable-error-stack -n tfcontents1.h5
TOOLTEST tordercontents1.ddl --enable-error-stack -n --sort_by=name --sort_order=ascending tfcontents1.h5
TOOLTEST tordercontents2.ddl --enable-error-stack -n --sort_by=name --sort_order=descending tfcontents1.h5
TOOLTEST tattrcontents1.ddl --enable-error-stack -n 1 --sort_order=ascending tall.h5
TOOLTEST tattrcontents2.ddl --enable-error-stack -n 1 --sort_order=descending tall.h5

# tests for storage layout
# compact
TOOLTEST tcompact.ddl --enable-error-stack -H -p -d compact tfilters.h5
# contiguous
TOOLTEST tcontiguos.ddl --enable-error-stack -H -p -d contiguous tfilters.h5
# chunked
TOOLTEST tchunked.ddl --enable-error-stack -H -p -d chunked tfilters.h5
# external
TOOLTEST texternal.ddl --enable-error-stack -H -p -d external tfilters.h5

# fill values
TOOLTEST tfill.ddl --enable-error-stack -p tfvalues.h5

# several datatype, with references , print path
TOOLTEST treference.ddl --enable-error-stack tattr2.h5

# escape/not escape non printable characters
TOOLTEST tstringe.ddl --enable-error-stack -e tstr3.h5
TOOLTEST tstring.ddl --enable-error-stack tstr3.h5
# char data as ASCII with non escape
TOOLTEST tstring2.ddl --enable-error-stack -r -d str4 tstr3.h5

# array indices print/not print
TOOLTEST tindicesyes.ddl --enable-error-stack taindices.h5
TOOLTEST tindicesno.ddl --enable-error-stack -y taindices.h5

########## array indices with subsetting
# 1D case
TOOLTEST tindicessub1.ddl --enable-error-stack -d 1d -s 1 -S 10 -c 2  -k 3 taindices.h5

# 2D case
TOOLTEST tindicessub2.ddl --enable-error-stack -d 2d -s 1,2  -S 3,3 -c 3,2 -k 2,2 taindices.h5

# 3D case
TOOLTEST tindicessub3.ddl --enable-error-stack -d 3d -s 0,1,2 -S 1,3,3 -c 2,2,2  -k 1,2,2  taindices.h5

# 4D case
TOOLTEST tindicessub4.ddl --enable-error-stack -d 4d -s 0,0,1,2  -c 2,2,3,2 -S 1,1,3,3 -k 1,1,2,2  taindices.h5

#Exceed the dimensions for subsetting
TOOLTEST texceedsubstart.ddl --enable-error-stack -d 1d -s 1,3 taindices.h5
TOOLTEST texceedsubcount.ddl --enable-error-stack -d 1d -c 1,3 taindices.h5
TOOLTEST texceedsubstride.ddl --enable-error-stack -d 1d -S 1,3 taindices.h5
TOOLTEST texceedsubblock.ddl --enable-error-stack -d 1d -k 1,3 taindices.h5


# tests for filters
# SZIP
TOOLTEST tszip.ddl --enable-error-stack -H -p -d szip tfilters.h5
# deflate
TOOLTEST tdeflate.ddl --enable-error-stack -H -p -d deflate tfilters.h5
# shuffle
TOOLTEST tshuffle.ddl --enable-error-stack -H -p -d shuffle tfilters.h5
# fletcher32
TOOLTEST tfletcher32.ddl --enable-error-stack -H -p -d fletcher32  tfilters.h5
# nbit
TOOLTEST tnbit.ddl --enable-error-stack -H -p -d nbit  tfilters.h5
# scaleoffset
TOOLTEST tscaleoffset.ddl --enable-error-stack -H -p -d scaleoffset  tfilters.h5
# all
TOOLTEST tallfilters.ddl --enable-error-stack -H -p -d all  tfilters.h5
# user defined
TOOLTEST tuserfilter.ddl --enable-error-stack -H  -p -d myfilter  tfilters.h5

if test $USE_FILTER_DEFLATE = "yes" ; then
  # data read internal filters
  TOOLTEST treadintfilter.ddl --enable-error-stack -d deflate -d shuffle -d fletcher32 -d nbit -d scaleoffset tfilters.h5
  if test $USE_FILTER_SZIP = "yes"; then
    # data read
    TOOLTEST treadfilter.ddl --enable-error-stack -d all -d szip tfilters.h5
  fi
fi

# test for displaying objects with very long names
TOOLTEST tlonglinks.ddl --enable-error-stack tlonglinks.h5

# dimensions over 4GB, print boundary
TOOLTEST tbigdims.ddl --enable-error-stack -d dset4gb -s 4294967284 -c 22 tbigdims.h5

# hyperslab read
TOOLTEST thyperslab.ddl --enable-error-stack thyperslab.h5


#

# test for displaying dataset and attribute of null space
TOOLTEST tnullspace.ddl --enable-error-stack tnullspace.h5
TOOLTEST tgrpnullspace.ddl -p --enable-error-stack tgrpnullspace.h5

# test for displaying dataset and attribute of space with 0 dimension size
TOOLTEST zerodim.ddl --enable-error-stack zerodim.h5

# test for long double (some systems do not have long double)
#TOOLTEST tldouble.ddl --enable-error-stack tldouble.h5

# test for vms
TOOLTEST tvms.ddl --enable-error-stack tvms.h5

# test for binary output
TOOLTEST tbin1.ddl --enable-error-stack -d integer -o out1.bin -b LE tbinary.h5

# test for string binary output
TOOLTEST2B tstr2bin2.exp --enable-error-stack -d /g2/dset2 -b -o tstr2bin2.txt tstr2.h5
TOOLTEST2B tstr2bin6.exp --enable-error-stack -d /g6/dset6 -b -o tstr2bin6.txt tstr2.h5

# NATIVE default. the NATIVE test can be validated with h5import/h5diff
TOOLTEST   tbin1.ddl --enable-error-stack -d integer -o out1.bin  -b  tbinary.h5
IMPORTTEST out1.bin -c out3.h5import -o out1.h5
DIFFTEST   tbinary.h5 out1.h5 /integer /integer
# Same but use h5dump as input to h5import
IMPORTTEST out1.bin -c tbin1.ddl -o out1D.h5
#DIFFTEST   tbinary.h5 out1D.h5 /integer /integer

TOOLTEST   tbin2.ddl --enable-error-stack -b BE -d float  -o out2.bin  tbinary.h5

# the NATIVE test can be validated with h5import/h5diff
TOOLTEST   tbin3.ddl --enable-error-stack -d integer -o out3.bin -b NATIVE tbinary.h5
IMPORTTEST out3.bin -c out3.h5import -o out3.h5
DIFFTEST   tbinary.h5 out3.h5 /integer /integer
# Same but use h5dump as input to h5import
IMPORTTEST out3.bin -c tbin3.ddl -o out3D.h5
#DIFFTEST   tbinary.h5 out3D.h5 /integer /integer

TOOLTEST   tbin4.ddl --enable-error-stack -d double  -b FILE -o out4.bin    tbinary.h5

# Clean up binary output files
if test -z "$HDF5_NOCLEANUP"; then
 rm -f out[1-4].bin
 rm -f out1.h5
 rm -f out3.h5
fi

# test for dataset region references
TOOLTEST  tdatareg.ddl --enable-error-stack tdatareg.h5
GREPTEST ERRTXT "NULL token size" tdataregR.ddl --enable-error-stack -R tdatareg.h5
TOOLTEST  tattrreg.ddl --enable-error-stack tattrreg.h5
GREPTEST ERRTXT "NULL token size" tattrregR.ddl --enable-error-stack -R tattrreg.h5
TOOLTEST2 tbinregR.exp --enable-error-stack -d /Dataset1 -s 0 -R -y -o tbinregR.txt    tdatareg.h5

# Clean up text output files
if test -z "$HDF5_NOCLEANUP"; then
 rm -f tbinregR.txt
fi

# tests for group creation order
# "1" tracked, "2" name, root tracked
TOOLTEST tordergr1.ddl --enable-error-stack --group=1 --sort_by=creation_order --sort_order=ascending tordergr.h5
TOOLTEST tordergr2.ddl --enable-error-stack --group=1 --sort_by=creation_order --sort_order=descending tordergr.h5
TOOLTEST tordergr3.ddl --enable-error-stack -g 2 -q name -z ascending tordergr.h5
TOOLTEST tordergr4.ddl --enable-error-stack -g 2 -q name -z descending tordergr.h5
TOOLTEST tordergr5.ddl --enable-error-stack -q creation_order tordergr.h5

# tests for attribute order
TOOLTEST torderattr1.ddl --enable-error-stack -H --sort_by=name --sort_order=ascending torderattr.h5
TOOLTEST torderattr2.ddl --enable-error-stack -H --sort_by=name --sort_order=descending torderattr.h5
TOOLTEST torderattr3.ddl --enable-error-stack -H --sort_by=creation_order --sort_order=ascending torderattr.h5
TOOLTEST torderattr4.ddl --enable-error-stack -H --sort_by=creation_order --sort_order=descending torderattr.h5

# tests for link references and order
TOOLTEST4 torderlinks1.ddl --enable-error-stack --sort_by=name --sort_order=ascending tfcontents1.h5
TOOLTEST4 torderlinks2.ddl --enable-error-stack --sort_by=name --sort_order=descending tfcontents1.h5

# tests for floating point user defined printf format
TOOLTEST tfpformat.ddl --enable-error-stack -m %.7f tfpformat.h5

# tests for traversal of external links
TOOLTEST4 textlinksrc.ddl --enable-error-stack textlinksrc.h5
TOOLTEST4 textlinkfar.ddl --enable-error-stack textlinkfar.h5

# test for dangling external links
TOOLTEST4 textlink.ddl --enable-error-stack textlink.h5

# test for error stack display (BZ2048)
GREPTEST2 ERRTXT "filter plugins disabled" filter_fail.ddl --enable-error-stack filter_fail.h5

# test for -o -y for dataset with attributes
TOOLTEST2 tall-6.exp --enable-error-stack -y -o tall-6.txt -d /g1/g1.1/dset1.1.1 tall.h5

# test for non-existing file
TOOLTEST3 non_existing.ddl --enable-error-stack tgroup.h5 non_existing.h5

# test to verify HDFFV-10333: error similar to H5O_attr_decode in the jira issue
TOOLTEST err_attr_dspace.ddl err_attr_dspace.h5

# test to verify HDFFV-9407: long double full precision
GREPTEST OUTTXT "1.123456789012345" t128bit_float.ddl -m %.35Lf t128bit_float.h5

END

# Clean up generated files/directories
CLEAN_OUTPUT

# Report test results and exit
if test $nerrors -eq 0 ; then
    echo "All $TESTNAME tests passed."
    exit $EXIT_SUCCESS
else
    echo "$TESTNAME tests failed with $nerrors errors."
    exit $EXIT_FAILURE
fi
