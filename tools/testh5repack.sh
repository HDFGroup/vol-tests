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
# Tests for the h5repack tool

# Assume the HDF5 library was built with gzip support
USE_FILTER_DEFLATE="yes"

TESTNAME=h5repack
EXIT_SUCCESS=0
EXIT_FAILURE=1

H5DELETE='h5delete -f'      # The h5delete tool name
H5REPACK=h5repack           # The h5repack tool name
H5DIFF=h5diff               # The h5diff tool name
H5DUMP=h5dump               # The h5dump tool name


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
H5REPACK_TESTFILES_HDF5_DIR="./testfiles/hdf5"

# Where the tool's expected output files are located
H5REPACK_TESTFILES_OUT_DIR="./testfiles/expected/h5repack"

######################################################################
# Output files
# --------------------------------------------------------------------

# Where the text output goes
TEXT_OUTPUT_DIR=./h5repack_test_output

# Where the repacked HDF5 input files go
REPACK_OUTPUT_DIR=./h5repack_repack_output

######################################################################
# test files
# --------------------------------------------------------------------

# HDF5 test files.
#
# Kept in       $H5LS_TESTFILES_HDF5_DIR
# Repacked to   $REPACK_OUTPUT_DIR
#
# These files fail to repack w/ native VOL (investigate later)
#
# These files have native-specific content
#
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
HDF5_FILES="
bounds_latest_latest.h5
h5repack_attr.h5
h5repack_attr_refs.h5
h5repack_deflate.h5
h5repack_early.h5
h5repack_ext.h5
h5repack_fill.h5
h5repack_filters.h5
h5repack_fletcher.h5
h5repack_hlink.h5
h5repack_layout.h5
h5repack_layouto.h5
h5repack_layout2.h5
h5repack_layout3.h5
h5repack_named_dtypes.h5
h5repack_nested_8bit_enum.h5
h5repack_nested_8bit_enum_deflated.h5
h5repack_nbit.h5
h5repack_objs.h5
h5repack_refs.h5
h5repack_shuffle.h5
h5repack_soffset.h5
h5repack_szip.h5
h5repack_aggr.h5
h5repack_fsm_aggr_nopersist.h5
h5repack_fsm_aggr_persist.h5
h5repack_none.h5
h5repack_paged_nopersist.h5
h5repack_paged_persist.h5
h5repack_f32le.h5
h5repack_f32le_ex.h5
h5repack_int32le_1d.h5
h5repack_int32le_1d_ex.h5
h5repack_int32le_2d.h5
h5repack_int32le_2d_ex.h5
h5repack_int32le_3d.h5
h5repack_int32le_3d_ex.h5
h5repack_uint8be.h5
h5repack_uint8be_ex.h5
h5diff_attr1.h5
tordergr.h5
"

# Expected output files.
#
# Kept in       $H5REPACK_TESTFILES_OUT_DIR
# Copied to     $TEXT_OUTPUT_DIR
#
# NOTE: This is ALL the files - they have not been culled based on the HDF5
#       files in the above list.
#
EXPECTED_OUTPUT_FILES="
h5repack-help.txt
h5repack_ext.bin
ublock.bin
h5repack.info
crtorder.tordergr.h5.ddl
deflate_limit.h5repack_layout.h5.ddl
h5repack_f32le_ex-0.dat
h5repack_int32le_1d_ex-0.dat
h5repack_int32le_1d_ex-1.dat
h5repack_int32le_2d_ex-0.dat
h5repack_int32le_3d_ex-0.dat
h5repack_layout.h5.ddl
h5repack_filters.h5-gzip_verbose_filters.tst
h5repack_layout.h5-dset2_chunk_20x10-errstk.tst
h5repack_layout.h5-plugin_test.ddl
h5repack_uint8be_ex-0.dat
h5repack_uint8be_ex-1.dat
h5repack_uint8be_ex-2.dat
h5repack_uint8be_ex-3.dat
plugin_test.h5repack_layout.h5.tst
1_vds.h5-vds_dset_chunk20x10x5-v.ddl
2_vds.h5-vds_chunk3x6x9-v.ddl
3_1_vds.h5-vds_chunk2x5x8-v.ddl
4_vds.h5-vds_compa-v.ddl
4_vds.h5-vds_conti-v.ddl
SP.h5repack_fsm_aggr_nopersist.h5.ddl
S.h5repack_fsm_aggr_persist.h5.ddl
STG.h5repack_none.h5.ddl
GS.h5repack_paged_nopersist.h5.ddl
SP.h5repack_paged_persist.h5.ddl
SPT.h5repack_aggr.h5.ddl
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
        filepath="$H5REPACK_TESTFILES_OUT_DIR/$outfile"

        # Use -f to make sure get a new copy
        $CP -f $filepath $TEXT_OUTPUT_DIR
        if [ $? -ne 0 ]; then
            echo "Error: FAILED to copy expected output file: $filepath ."
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

# Print a $* message left justified in a field of 70 characters
#
MESSAGE() {
   SPACES="                                                               "
   echo "$* $SPACES" | cut -c1-70 | tr -d '\012'
}

# Print a line-line message left justified in a field of 70 characters
# beginning with the word "Testing".
#
TESTING() {
    MESSAGE "Testing $*"
}

# Print a line-line message left justified in a field of 70 characters
# beginning with the word "Verifying".
#
VERIFY() {
    MESSAGE "Verifying $*"
}

# Print a message that a test has been skipped (because a required filter
# was unavailable)
SKIP() {
    TESTING $H5REPACK $@
    echo " -SKIP-"
}

######################################################################
# Main testing functions
# --------------------------------------------------------------------

# Call the h5diff tool
#
DIFFTEST()
{
    VERIFY h5diff output $@
    (
        $RUNSERIAL $H5DIFF -q  "$@"
    )
    RET=$?
    if [ $RET != 0 ] ; then
         echo "*FAILED*"
         nerrors="`expr $nerrors + 1`"
    else
         echo " PASSED"
    fi

}

# call TOOLTEST_MAIN
TOOLTEST()
{
    echo $@
    infile=$2
    outfile=out-$1.$2
    shift
    shift

    # Run test.
    TESTING $H5REPACK $@
    (
        $ENVCMD $RUNSERIAL $H5REPACK "$@" $infile $outfile
    )
    RET=$?
    if [ $RET != 0 ] ; then
        echo "*FAILED*"
        nerrors="`expr $nerrors + 1`"
    else
        echo " PASSED"
        DIFFTEST $infile $outfile
    fi
}

#------------------------------------------
# Verifying layouts of a dataset
VERIFY_LAYOUT_DSET()
{
    layoutfile=layout-$1.$2
    dset=$3
    expectlayout=$4
    infile=$2
    outfile=out-$1.$2
    shift
    shift
    shift
    shift

    TESTING  $H5REPACK $@
    (
        $RUNSERIAL $H5REPACK "$@" $infile $outfile
    )
    RET=$?
    if [ $RET != 0 ] ; then
        echo "*FAILED*"
        nerrors="`expr $nerrors + 1`"
    else
        echo " PASSED"
        DIFFTEST $infile $outfile
    fi

    #---------------------------------
    # check the layout from a dataset
    VERIFY  "a dataset layout"
    (
        $RUNSERIAL $H5DUMP -d $dset -pH $outfile > $layoutfile
    )
    $GREP $expectlayout $TESTDIR/$layoutfile > /dev/null
    if [ $? -eq 0 ]; then
        echo " PASSED"
    else
        echo " FAILED"
        nerrors="`expr $nerrors + 1`"
    fi
}

#----------------------------------------
# Verifying layouts from entire file
VERIFY_LAYOUT_ALL()
{
    infile=$2
    outfile=out-$1.$2
    layoutfile=layout-$1.$2
    expectlayout=$3
    shift
    shift
    shift

    TESTING  $H5REPACK $@
    (
        $RUNSERIAL $H5REPACK "$@" $infile $outfile
    )
    RET=$?
    if [ $RET != 0 ] ; then
        echo "*FAILED*"
        nerrors="`expr $nerrors + 1`"
    else
        echo " PASSED"
        DIFFTEST $infile $outfile
    fi


    #---------------------------------
    # check the layout from a dataset
    # check if the other layouts still exsit
    VERIFY  "layouts"
    (
        echo
        # if CONTIGUOUS
        if [ $expectlayout = "CONTIGUOUS" ]; then
            TESTING $H5DUMP -pH $outfile
            (
                $RUNSERIAL $H5DUMP -pH $outfile > $layoutfile
            )
            $GREP "COMPACT" $layoutfile  > /dev/null
            if [ $? -eq 0 ]; then
                echo " FAILED"
                nerrors="`expr $nerrors + 1`"
            else
                $GREP "CHUNKED" $layoutfile  > /dev/null
                if [ $? -eq 0 ]; then
                    echo " FAILED"
                    nerrors="`expr $nerrors + 1`"
                else
                    echo " PASSED"
                fi
            fi
        else
            # if COMPACT
            if [ $expectlayout = "COMPACT" ]; then
                TESTING $H5DUMP -pH $outfile
                (
                    $RUNSERIAL $H5DUMP -pH $outfile > $layoutfile
                )
                $GREP "CHUNKED" $layoutfile  > /dev/null
                if [ $? -eq 0 ]; then
                    echo " FAILED"
                    nerrors="`expr $nerrors + 1`"
                else
                    $GREP "CONTIGUOUS" $layoutfile  > /dev/null
                    if [ $? -eq 0 ]; then
                        echo " FAILED"
                        nerrors="`expr $nerrors + 1`"
                    else
                        echo " PASSED"
                    fi
                fi
            else
                # if CHUNKED
                if [ $expectlayout = "CHUNKED" ]; then
                    TESTING $H5DUMP -pH $outfile
                    (
                        $RUNSERIAL $H5DUMP -pH $outfile > $layoutfile
                    )
                    $GREP "CONTIGUOUS" $layoutfile  > /dev/null
                    if [ $? -eq 0 ]; then
                        echo " FAILED"
                        nerrors="`expr $nerrors + 1`"
                    else
                        $GREP "COMPACT" $layoutfile  > /dev/null
                        if [ $? -eq 0 ]; then
                            echo " FAILED"
                            nerrors="`expr $nerrors + 1`"
                        else
                            echo " PASSED"
                        fi
                    fi
                fi
           fi
        fi
    )
}

# -----------------------------------------------------------------------------
# Expect h5diff to fail
# -----------------------------------------------------------------------------
DIFFFAIL()
{
    VERIFY h5diff unequal $@
    (
        $RUNSERIAL $H5DIFF -q  "$@"
    )
    RET=$?
    if [ $RET == 0 ] ; then
         echo "*FAILED*"
         nerrors="`expr $nerrors + 1`"
    else
         echo " PASSED"
    fi
}

# -----------------------------------------------------------------------------
# same as TOOLTEST, but it uses the old syntax -i input_file -o output_file
# -----------------------------------------------------------------------------
TOOLTEST0()
{
    infile=$2
    outfile=out-$1.$2
    shift
    shift

    # Run test.
    TESTING $H5REPACK $@
    (
        $RUNSERIAL $H5REPACK -i $infile -o $outfile "$@"
    )
    RET=$?
    if [ $RET != 0 ] ; then
        echo "*FAILED*"
        nerrors="`expr $nerrors + 1`"
    else
        echo " PASSED"
        DIFFTEST $infile $outfile
    fi
}


# same as TOOLTEST, but it uses without -i -o options
# used to test the family driver, where these files reside
#
TOOLTEST1()
{
    infile=$2
    outfile=out-$1.$2
    shift
    shift

    # Run test.
    TESTING $H5REPACK $@
    (
        $ENVCMD $RUNSERIAL $H5REPACK "$@" $infile $outfile
    )
    RET=$?
    if [ $RET != 0 ] ; then
        echo "*FAILED*"
        nerrors="`expr $nerrors + 1`"
    else
        echo " PASSED"
        DIFFTEST $infile $outfile
    fi
}

# This is same as TOOLTEST() with comparing display output
# from -v option
#
TOOLTESTV()
{
    expect="$TESTDIR/$2-$1.tst"
    actual="$TESTDIR/`basename $2 .ddl`.out"
    actual_err="$TESTDIR/`basename $2 .ddl`.err"

    infile=$2
    outfile=out-$1.$2
    shift
    shift

    # Run test.
    TESTING $H5REPACK $@
    (
        $RUNSERIAL $H5REPACK "$@" $infile $outfile
    ) >$actual 2>$actual_err
    RET=$?
    if [ $RET != 0 ] ; then
        echo "*FAILED*"
        nerrors="`expr $nerrors + 1`"
    else
        echo " PASSED"
        DIFFTEST $infile $outfile
    fi

    # display output compare
    STDOUT_FILTER $actual
    cat $actual_err >> $actual

    VERIFY output from $H5REPACK $@
    if cmp -s $expect $actual; then
        echo " PASSED"
    else
        echo "*FAILED*"
        echo "    Expected result (*.tst) differs from actual result (*.out)"
        nerrors="`expr $nerrors + 1`"
        test yes = "$verbose" && diff -c $expect $actual |sed 's/^/    /'
    fi
}

# same as TOOLTESTV but filters error stack output and compares to an error file
# Extract file name, line number, version and thread IDs because they may be different
# ADD_H5ERR_MASK_TEST
TOOLTESTM() {

    expect="$TESTDIR/$2-$1.tst"
    actual="$TESTDIR/`basename $2 .tst`.out"
    actual_err="$TESTDIR/`basename $2 .tst`.oerr"
    actual_sav=${actual}-sav
    actual_err_sav=${actual_err}-sav

    infile=$2
    outfile=out-$1.$2
    shift
    shift

    # Run test.
    TESTING $H5REPACK $@
    (
        $ENVCMD $RUNSERIAL $H5REPACK "$@" $infile $outfile
    ) >$actual 2>$actual_err

    # save actual and actual_err in case they are needed later.
    cp $actual $actual_sav
    cp $actual_err $actual_err_sav

    # Extract file name, line number, version and thread IDs because they may be different
    sed -e 's/thread [0-9]*/thread (IDs)/' -e 's/: .*\.c /: (file name) /' \
        -e 's/line [0-9]*/line (number)/' \
        -e 's/v[1-9]*\.[0-9]*\./version (number)\./' \
        -e 's/[1-9]*\.[0-9]*\.[0-9]*[^)]*/version (number)/' \
        -e 's/H5Eget_auto[1-2]*/H5Eget_auto(1 or 2)/' \
        -e 's/H5Eset_auto[1-2]*/H5Eset_auto(1 or 2)/' \
     $actual_err > $actual

    if [ ! -f $expect ]; then
        # Create the expect file if it doesn't yet exist.
        echo " CREATED"
        cp $actual $expect
        echo "    Expected result (*.tst) missing"
        nerrors="`expr $nerrors + 1`"
    elif $CMP $expect $actual; then
        echo " PASSED"
    else
        echo "*FAILED*"
        echo "    Expected result (*.tst) differs from actual result (*.out)"
        nerrors="`expr $nerrors + 1`"
        test yes = "$verbose" && $DIFF $expect $actual |sed 's/^/    /'
    fi

}

# This is same as TOOLTESTV() with comparing h5dump output
# from -pH option
#
TOOLTEST_DUMP()
{
    infile=$2
    outfile=out-$1.$2
    expect="$TESTDIR/$1.$2.ddl"
    actual="$TESTDIR/out-$1.$2.out"
    actual_err="$TESTDIR/out-$1.$2.err"

    shift
    shift

    # Run test.
    TESTING $H5REPACK $@
    (
        $RUNSERIAL $H5REPACK "$@" $infile $outfile
    ) >$actual 2>$actual_err
    RET=$?
    if [ $RET != 0 ] ; then
        echo "*FAILED*"
        nerrors="`expr $nerrors + 1`"
    else
        echo " PASSED"
        VERIFY h5dump output $@
        (
            $RUNSERIAL $H5DUMP -q creation_order -pH $outfile
        ) >$actual 2>$actual_err
        cat $actual_err >> $actual

        RET=$?
    fi

    if cmp -s $expect $actual; then
        echo " PASSED"
    else
        echo "*FAILED*"
        echo "    Expected result (*.ddl) differs from actual result (*.out)"
        nerrors="`expr $nerrors + 1`"
        test yes = "$verbose" && diff -c $expect $actual |sed 's/^/    /'
    fi
}

RUNTEST_HELP() {

    expect="$TEXT_OUTPUT_DIR/$1"
    actual="$TEXT_OUTPUT_DIR/`basename $1 .txt`.out"
    actual_err="$TEXT_OUTPUT_DIR/`basename $1 .txt`.err"
    shift

    # Run test.
    TESTING $H5REPACK $@
    (
      $RUNSERIAL $H5REPACK "$@"
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

# This is different from output_filter.sh
STDOUT_FILTER() {
    result_file=$1
    tmp_file=/tmp/h5test_tmp_$$
    # Filter name of files.
    cp $result_file $tmp_file
    sed -e '/^Opening file/d' -e '/^Making file/d' \
    < $tmp_file > $result_file
    # cleanup
    rm -f $tmp_file
}

#
# The tests
# We use the files generated by h5repacktst
# Each run generates "<file>.out.h5" and the tool h5diff is used to
# compare the input and output files
#

##############################################################################
##############################################################################
###              T H E   T E S T S                                         ###
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

RUNTEST_HELP h5repack-help.txt -h
: <<'END'

# copy files (these files have no filters)
TOOLTEST fill h5repack_fill.h5
TOOLTEST objs h5repack_objs.h5
TOOLTEST attr h5repack_attr.h5
TOOLTEST hlink h5repack_hlink.h5
TOOLTEST layout h5repack_layout.h5
TOOLTEST early h5repack_early.h5

# nested 8bit enum in both deflated and non-deflated datafiles
if [ $USE_FILTER_DEFLATE != "yes" ]; then
TOOLTEST nested_8bit_enum h5repack_nested_8bit_enum.h5
else
TOOLTEST nested_8bit_enum h5repack_nested_8bit_enum_deflated.h5
fi

###########################################################
# the following tests assume the input files have filters
###########################################################

#limit
arg="h5repack_layout.h5 -f GZIP=1 -m 1024"
if test $USE_FILTER_DEFLATE != "yes"  ; then
 SKIP $arg
else
 TOOLTEST_DUMP deflate_limit $arg
fi

#file
arg="h5repack_layout.h5 -e h5repack.info"
if test $USE_FILTER_DEFLATE != "yes" ; then
 SKIP $arg
else
 TOOLTEST deflate_file $arg
fi

#crtorder
arg="tordergr.h5 -L"
TOOLTEST_DUMP crtorder $arg


#########################################################
# layout options (these files have no filters)
#########################################################
VERIFY_LAYOUT_DSET dset2_chunk_20x10 h5repack_layout.h5 dset2 CHUNKED --layout dset2:CHUNK=20x10

VERIFY_LAYOUT_ALL chunk_20x10 h5repack_layout.h5 CHUNKED -l CHUNK=20x10

VERIFY_LAYOUT_DSET dset2_conti h5repack_layout.h5 dset2 CONTIGUOUS -l dset2:CONTI

VERIFY_LAYOUT_ALL conti h5repack_layout.h5 CONTIGUOUS -l CONTI

TOOLTESTM dset2_chunk_20x10-errstk h5repack_layout.h5 --layout=dset2:CHUNK=20x10x5 --enable-error-stack

################################################################
# layout conversions (file has no filters)
###############################################################

VERIFY_LAYOUT_DSET dset_compa_conti h5repack_layout.h5 dset_compact CONTIGUOUS -l dset_compact:CONTI

VERIFY_LAYOUT_DSET dset_compa_chunk h5repack_layout.h5 dset_compact CHUNKED -l dset_compact:CHUNK=2x5

VERIFY_LAYOUT_DSET dset_conti_chunk h5repack_layout.h5 dset_contiguous CHUNKED -l dset_contiguous:CHUNK=3x6

VERIFY_LAYOUT_DSET dset_conti_conti h5repack_layout.h5 dset_contiguous CONTIGUOUS -l dset_contiguous:CONTI

VERIFY_LAYOUT_DSET chunk_conti h5repack_layout.h5 dset_chunk CONTIGUOUS -l dset_chunk:CONTI

VERIFY_LAYOUT_DSET chunk_18x13 h5repack_layout.h5 dset_chunk CHUNKED -l dset_chunk:CHUNK=18x13

#---------------------------------------------------------------------------
# Test file contains chunked datasets (need multiple dsets) with
# unlimited max dims.   (HDFFV-7933)
# Use first dset to test.
#---------------------------------------------------------------------------
# chunk to chunk - specify chunk dim bigger than any current dim
VERIFY_LAYOUT_DSET chunk2chunk h5repack_layout3.h5 chunk_unlimit1 CHUNK -l chunk_unlimit1:CHUNK=100x300

# chunk to contiguous
VERIFY_LAYOUT_DSET chunk2conti h5repack_layout3.h5 chunk_unlimit1 CONTI -l chunk_unlimit1:CONTI

#--------------------------------------------------------------------------
# Test -f for some specific cases. Chunked dataset with unlimited max dims.
# (HDFFV-8012)
#--------------------------------------------------------------------------
# - should not fail
# - should not change max dims from unlimit

# chunk dim is bigger than dataset dim. ( dset size < 64k )
VERIFY_LAYOUT_DSET error1 h5repack_layout3.h5 chunk_unlimit1 H5S_UNLIMITED -f chunk_unlimit1:NONE
# chunk dim is bigger than dataset dim. ( dset size > 64k )
VERIFY_LAYOUT_DSET error2 h5repack_layout3.h5 chunk_unlimit2 H5S_UNLIMITED -f chunk_unlimit2:NONE

# chunk dims are smaller than dataset dims. ( dset size < 64k )
#TOOLTEST_MAIN h5repack_layout3.h5  -f chunk_unlimit3:NONE
VERIFY_LAYOUT_DSET error3 h5repack_layout3.h5 chunk_unlimit3 H5S_UNLIMITED -f chunk_unlimit3:NONE

# file input - should not fail
TOOLTEST error4 h5repack_layout3.h5 -f NONE

#--------------------------------------------------------------------------
# Test base: Convert CHUNK to CONTI for a chunked dataset with small dataset
# (dset size < 64K) and with unlimited max dims on a condition as follow.
# (HDFFV-8214)
#--------------------------------------------------------------------------

# chunk dim is bigger than dataset dim. should succeed.
VERIFY_LAYOUT_DSET ckdim_biger h5repack_layout3.h5 chunk_unlimit2 CONTI -l chunk_unlimit2:CONTI
# chunk dim is smaller than dataset dim. should succeed.
VERIFY_LAYOUT_DSET ckdim_smaller h5repack_layout3.h5 chunk_unlimit3 CONTI -l chunk_unlimit3:CONTI


# Native option
# Do not use FILE1, as the named dtype will be converted to native, and h5diff will
# report a difference.
TOOLTEST native_fill h5repack_fill.h5 -n
TOOLTEST native_attr h5repack_attr.h5 -n


# latest file format with long switches. use FILE4=h5repack_layout.h5 (no filters)
arg="h5repack_layout.h5 --layout CHUNK=20x10 --filter GZIP=1 --minimum=10 --native --latest --compact=8 --indexed=6 --ssize=8[:dtype]"
if test $USE_FILTER_DEFLATE != "yes" ; then
 SKIP $arg
else
 VERIFY_LAYOUT_ALL layout_long_switches h5repack_layout.h5 CHUNKED --layout CHUNK=20x10 --filter GZIP=1 --minimum=10 --native --latest --compact=8 --indexed=6 --ssize=8[:dtype]
fi

# latest file format with short switches. use FILE4=h5repack_layout.h5 (no filters)
arg="h5repack_layout.h5 -l CHUNK=20x10 -f GZIP=1 -m 10 -n -L -c 8 -d 6 -s 8[:dtype]"
if test $USE_FILTER_DEFLATE != "yes" ; then
 SKIP $arg
else
 VERIFY_LAYOUT_ALL layout_short_switches h5repack_layout.h5 CHUNKED -l CHUNK=20x10 -f GZIP=1 -m 10 -n -L -c 8 -d 6 -s 8[:dtype]
fi

# several global filters

arg="h5repack_layout.h5 --filter GZIP=1 --filter SHUF"
if test $USE_FILTER_DEFLATE != "yes" ; then
 SKIP $arg
else
 TOOLTEST global_filters $arg
fi

# syntax of -i infile -o outfile
# latest file format with short switches. use FILE4=h5repack_layout.h5 (no filters)
arg="h5repack_layout.h5 -l CHUNK=20x10 -f GZIP=1 -m 10 -n -L -c 8 -d 6 -s 8[:dtype]"
if test $USE_FILTER_DEFLATE != "yes" ; then
 SKIP $arg
else
 TOOLTEST0 old_style_layout_short_switches $arg
fi

# add alignment
arg="h5repack_objs.h5 -t 1 -a 1 "
TOOLTEST add_alignment $arg

# test for datum size > H5TOOLS_MALLOCSIZE
arg="h5repack_objs.h5 -f GZIP=1"
if test $USE_FILTER_DEFLATE != "yes" ; then
 SKIP $arg
else
 TOOLTEST gt_mallocsize $arg
fi

# Check repacking file with committed datatypes in odd configurations
TOOLTEST committed_dt h5repack_named_dtypes.h5

# test various references (bug 1814 and 1726)
TOOLTEST bug1814 h5repack_refs.h5

# test attribute with various references (bug1797 / HDFFV-5932)
# the references in attribute of compund or vlen datatype
TOOLTEST HDFFV-5932 h5repack_attr_refs.h5

# Add test for memory leak in attirbute. This test is verified by CTEST.
# 1. leak from vlen string
# 2. leak from compound type without reference member
# (HDFFV-7840, )
# Note: this test is experimental for sharing test file among tools
TOOLTEST HDFFV-7840 h5diff_attr1.h5

# Clean up generated files/directories
CLEAN_OUTPUT

END

if test $nerrors -eq 0 ; then
    echo "All $TESTNAME tests passed."
    exit $EXIT_SUCCESS
else
    echo "$TESTNAME tests failed with $nerrors errors."
    exit $EXIT_FAILURE
fi

