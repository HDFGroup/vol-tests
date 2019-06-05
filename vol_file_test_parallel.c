/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "vol_file_test_parallel.h"

static int test_split_comm_file_access(void);

/*
 * The array of parallel file tests to be performed.
 */
static int (*par_file_tests[])(void) = {
    test_split_comm_file_access
};

/*
 * Tests file access by a communicator other than MPI_COMM_WORLD.
 *
 * Splits MPI_COMM_WORLD into two groups, where one (even_comm) contains
 * the original processes of even ranks. The other (odd_comm) contains
 * the original processes of odd ranks. Processes in even_comm create a
 * file, then close it, using even_comm. Processes in old_comm just do
 * a barrier using odd_comm. Then they all do a barrier using MPI_COMM_WORLD.
 * If the file creation and close does not do correct collective action
 * according to the communicator argument, the processes will freeze up
 * sooner or later due to MPI_Barrier calls being mixed up.
 */
#define SPLIT_FILE_COMM_TEST_FILE_NAME "split_comm_file.h5"
static int
test_split_comm_file_access(void)
{
    MPI_Comm comm;
    MPI_Info info = MPI_INFO_NULL;
    hid_t    file_id = H5I_INVALID_HID;
    hid_t    fapl_id = H5I_INVALID_HID;
    int      is_old;
    int      newrank;

    TESTING("file access with a split communicator")

    /* set up MPI parameters */
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
    is_old = mpi_rank % 2;
    if (MPI_SUCCESS != MPI_Comm_split(MPI_COMM_WORLD, is_old, mpi_rank, &comm)) {
        H5_FAILED();
        HDprintf("    failed to split communicator!\n");
        goto error;
    }
    MPI_Comm_rank(comm, &newrank);

    if (is_old) {
        /* odd-rank processes */
        if (MPI_SUCCESS != MPI_Barrier(comm))
            TEST_ERROR
    }
    else {
        /* even-rank processes */
        int sub_mpi_rank;   /* rank in the sub-comm */

        MPI_Comm_rank(comm, &sub_mpi_rank);

        /* setup file access template */
        if ((fapl_id = create_mpio_fapl(comm, info)) < 0)
            TEST_ERROR

        /* create the file collectively */
        if ((file_id = H5Fcreate(SPLIT_FILE_COMM_TEST_FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create file '%s'\n", SPLIT_FILE_COMM_TEST_FILE_NAME);
            goto error;
        }

        /* Release file-access template */
        if (H5Pclose(fapl_id) < 0)
            TEST_ERROR

        /* close the file */
        if (H5Fclose(file_id) < 0) {
            H5_FAILED();
            HDprintf("    failed to close file '%s'\n", SPLIT_FILE_COMM_TEST_FILE_NAME);
            goto error;
        }

        /* delete the test file */
        if (sub_mpi_rank == 0)
            MPI_File_delete(SPLIT_FILE_COMM_TEST_FILE_NAME, info);
    }

    if (MPI_SUCCESS != MPI_Comm_free(&comm)) {
        H5_FAILED();
        HDprintf("    MPI_Comm_free failed\n");
        goto error;
    }

    if (MPI_SUCCESS != MPI_Barrier(MPI_COMM_WORLD)) {
        H5_FAILED();
        HDprintf("    MPI_Barrier on MPI_COMM_WORLD failed\n");
        goto error;
    }

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * Cleanup temporary test files
 */
static void
cleanup_files(void)
{

}

int
vol_file_test_parallel(void)
{
    size_t i;
    int    nerrors;

    HDprintf("**********************************************\n");
    HDprintf("*                                            *\n");
    HDprintf("*          VOL Parallel File Tests           *\n");
    HDprintf("*                                            *\n");
    HDprintf("**********************************************\n\n");

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(par_file_tests); i++) {
        nerrors += (*par_file_tests[i])() ? 1 : 0;
    }

    HDprintf("\n");

    cleanup_files();

    return nerrors;
}
