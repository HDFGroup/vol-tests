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

#include "vol_test_parallel.h"
#include "vol_test_util.h"

#include "vol_file_test_parallel.h"
#include "vol_group_test_parallel.h"
#include "vol_dataset_test_parallel.h"
#include "vol_datatype_test_parallel.h"
#include "vol_attribute_test_parallel.h"
#include "vol_link_test_parallel.h"
#include "vol_object_test_parallel.h"
#include "vol_misc_test_parallel.h"

char vol_test_parallel_filename[VOL_TEST_FILENAME_MAX_LENGTH];

int mpi_size, mpi_rank;

hid_t
create_mpio_fapl(MPI_Comm comm, MPI_Info info)
{
    hid_t ret_pl = H5I_INVALID_HID;

    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    if ((ret_pl = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        goto error;

    if (H5Pset_fapl_mpio(ret_pl, comm, info) < 0)
        goto error;
    if (H5Pset_all_coll_metadata_ops(ret_pl, TRUE) < 0)
        goto error;
    if (H5Pset_coll_metadata_write(ret_pl, TRUE) < 0)
        goto error;

    return ret_pl;

error:
    return H5I_INVALID_HID;
} /* end create_mpio_fapl() */

int
main(int argc, char **argv)
{
    char *vol_connector_name;
    int   nerrors = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);

    srand((unsigned) HDtime(NULL));

    HDsnprintf(vol_test_parallel_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", PARALLEL_TEST_FILE_NAME);

    if (NULL == (vol_connector_name = HDgetenv("HDF5_VOL_CONNECTOR"))) {
        HDprintf("No VOL connector selected; using native VOL connector\n");
        vol_connector_name = "native";
    }

    if (MAINPROCESS) {
        HDprintf("Running parallel VOL tests with VOL connector '%s'\n\n", vol_connector_name);
        HDprintf("Test parameters:\n");
        HDprintf("  - Test file name: '%s'\n", vol_test_parallel_filename);
        HDprintf("\n\n");
    }

    /*
     * Create the file that will be used for all of the tests,
     * except for those which test file creation.
     */
    if (create_test_container(vol_test_parallel_filename) < 0) {
        if (MAINPROCESS)
            HDfprintf(stderr, "Unable to create testing container file '%s'\n", vol_test_parallel_filename);
        nerrors++;
        goto done;
    }

    nerrors += vol_file_test_parallel();
    nerrors += vol_group_test_parallel();
    nerrors += vol_dataset_test_parallel();
    nerrors += vol_datatype_test_parallel();
    nerrors += vol_attribute_test_parallel();
    nerrors += vol_link_test_parallel();
    nerrors += vol_object_test_parallel();
    nerrors += vol_misc_test_parallel();

    if (nerrors) {
        if (MAINPROCESS)
            HDprintf("*** %d TEST%s FAILED WITH VOL CONNECTOR '%s' ***\n", nerrors, (!nerrors || nerrors > 1) ? "S" : "", vol_connector_name);
        goto done;
    }

    if (MAINPROCESS)
        HDprintf("All VOL tests passed with VOL connector '%s'\n\n", vol_connector_name);

done:
    H5close();

    MPI_Finalize();

    HDexit((nerrors ? EXIT_FAILURE : EXIT_SUCCESS));
}
