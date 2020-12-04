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

#include "vol_async_test_parallel.h"

#ifdef _H5ESpublic_H

static int test_one_dataset_io(void);
static int test_multi_dataset_io(void);
static int test_multi_file_dataset_io(void);
static int test_multi_file_grp_dset_io(void);
static int test_set_extent(void);

/*
 * The array of parallel async tests to be performed.
 */
static int (*par_async_tests[])(void) = {
    test_one_dataset_io,
    test_multi_dataset_io,
    test_multi_file_dataset_io,
    test_multi_file_grp_dset_io,
    test_set_extent,
};

/* Highest "printf" file created (starting at 0) */
int max_printf_file = -1;

/*
 * Create file and dataset. Each rank writes to a portion
 * of the dataset.
 */
#define ONE_DATASET_IO_TEST_SPACE_RANK 2
static int
test_one_dataset_io(void)
{
    hsize_t *dims = NULL;
    hsize_t  start[ONE_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  stride[ONE_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  count[ONE_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  block[ONE_DATASET_IO_TEST_SPACE_RANK];
    hbool_t  op_failed;
    size_t   i, data_size, num_in_progress;
    hid_t    file_id = H5I_INVALID_HID;
    hid_t    fapl_id = H5I_INVALID_HID;
    hid_t    dset_id = H5I_INVALID_HID;
    hid_t    space_id = H5I_INVALID_HID;
    hid_t    mspace_id = H5I_INVALID_HID;
    hid_t    es_id = H5I_INVALID_HID;
    int     *write_buf = NULL;
    int     *read_buf = NULL;

    TESTING_MULTIPART("single dataset I/O")

    TESTING_2("test setup")

    if ((fapl_id = create_mpi_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    /* Create dataspace */
    if (generate_random_parallel_dimensions(ONE_DATASET_IO_TEST_SPACE_RANK, &dims) < 0)
        TEST_ERROR

    if((space_id = H5Screate_simple(ONE_DATASET_IO_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Create file asynchronously */
    if((file_id = H5Fcreate_async(PAR_ASYNC_VOL_TEST_FILE, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, es_id)) < 0)
        TEST_ERROR

    /* Create the dataset asynchronously */
    if((dset_id = H5Dcreate_async(file_id, "dset", H5T_NATIVE_INT, space_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Calculate size of data buffers */
    for (i = 1, data_size = 1; i < ONE_DATASET_IO_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= sizeof(int);

    if (NULL == (write_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        TEST_ERROR
    }

    if (NULL == (read_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        TEST_ERROR
    }

    /* Select this rank's portion of the dataspace */
    for (i = 0; i < ONE_DATASET_IO_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = (hsize_t)mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    /* Setup memory space for write_buf */
    {
        hsize_t mdims[] = { data_size / sizeof(int) };

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(single_dset_eswait) {
            TESTING_2("synchronization using H5ESwait()")

            /* Initialize write_buf */
            for (i = 0; i < data_size / sizeof(int); i++)
                ((int *) write_buf)[i] = mpi_rank;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    write_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_eswait)

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(single_dset_eswait)
            if(op_failed)
                PART_TEST_ERROR(single_dset_eswait)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    read_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_eswait)

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(single_dset_eswait)
            if(op_failed)
                PART_TEST_ERROR(single_dset_eswait)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(single_dset_eswait)
                } /* end if */

            PASSED();
        } PART_END(single_dset_eswait);

        PART_BEGIN(single_dset_dclose) {
            TESTING_2("synchronization using H5Dclose()")

            /* Initialize write_buf */
            for (i = 0; i < data_size / sizeof(int); i++)
                ((int *) write_buf)[i] = i;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    write_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Close the dataset synchronously */
            if(H5Dclose(dset_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Re-open the dataset asynchronously */
            if((dset_id = H5Dopen_async(file_id, "dset", H5P_DEFAULT, es_id)) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    read_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Close the dataset synchronously */
            if(H5Dclose(dset_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(single_dset_dclose)
                } /* end if */

            /* Re-open the dataset asynchronously */
            if((dset_id = H5Dopen_async(file_id, "dset", H5P_DEFAULT, es_id)) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            PASSED();
        } PART_END(single_dset_dclose);

        PART_BEGIN(single_dset_dflush) {
            TESTING_2("synchronization using H5Oflush_async()")

            /* Initialize write_buf */
            for (i = 0; i < data_size / sizeof(int); i++)
                ((int *) write_buf)[i] = 10 * i;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    write_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dflush)

            /* Flush the dataset asynchronously.  This will effectively work as a
             * barrier, guaranteeing the read takes place after the write. */
            if(H5Oflush_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_dflush)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    read_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dflush)

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(single_dset_dflush)
            if(op_failed)
                PART_TEST_ERROR(single_dset_dflush)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(single_dset_dflush)
                } /* end if */

            PASSED();
        } PART_END(single_dset_dflush);

        PART_BEGIN(single_dset_fclose) {
            TESTING_2("synchronization using H5Fclose()")

            /* Initialize write_buf */
            for (i = 0; i < data_size / sizeof(int); i++)
                ((int *) write_buf)[i] = i + 5;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    write_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the dataset asynchronously */
            if(H5Dclose_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the file synchronously */
            if(H5Fclose(file_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Reopen the file asynchronously. */
            if((file_id = H5Fopen_async(PAR_ASYNC_VOL_TEST_FILE, H5F_ACC_RDONLY, fapl_id, es_id)) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Re-open the dataset asynchronously */
            if((dset_id = H5Dopen_async(file_id, "dset", H5P_DEFAULT, es_id)) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                    read_buf, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the dataset asynchronously */
            if(H5Dclose_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the file synchronously */
            if(H5Fclose(file_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(single_dset_fclose)
                } /* end if */

            PASSED();
        } PART_END(single_dset_fclose);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if(H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5Sclose(mspace_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(space_id);
        H5Sclose(mspace_id);
        H5Dclose(dset_id);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
}
#undef ONE_DATASET_IO_TEST_SPACE_RANK

/*
 * Create file and multiple datasets. Each rank writes to a
 * portion of each dataset and reads back their portion of
 * each dataset.
 */
#define MULTI_DATASET_IO_TEST_SPACE_RANK 2
#define MULTI_DATASET_IO_TEST_NDSETS 5
static int
test_multi_dataset_io(void)
{
    hsize_t *dims = NULL;
    hsize_t  start[MULTI_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  stride[MULTI_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  count[MULTI_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  block[MULTI_DATASET_IO_TEST_SPACE_RANK];
    hbool_t  op_failed;
    size_t   i, j, data_size, num_in_progress;
    hid_t    file_id = H5I_INVALID_HID;
    hid_t    fapl_id = H5I_INVALID_HID;
    hid_t    dset_id[MULTI_DATASET_IO_TEST_NDSETS] =
                          { H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID };
    hid_t    space_id = H5I_INVALID_HID;
    hid_t    mspace_id = H5I_INVALID_HID;
    hid_t    es_id = H5I_INVALID_HID;
    char     dset_name[32];
    int     *write_buf = NULL;
    int     *read_buf = NULL;

    TESTING_MULTIPART("multi dataset I/O")

    TESTING_2("test setup")

    if ((fapl_id = create_mpi_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    /* Create dataspace */
    if (generate_random_parallel_dimensions(MULTI_DATASET_IO_TEST_SPACE_RANK, &dims) < 0)
        TEST_ERROR

    /* Create dataspace */
    if((space_id = H5Screate_simple(MULTI_DATASET_IO_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Create file asynchronously */
    if((file_id = H5Fcreate_async(PAR_ASYNC_VOL_TEST_FILE, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, es_id)) < 0)
        TEST_ERROR

    /* Calculate size of data buffers */
    for (i = 1, data_size = 1; i < MULTI_DATASET_IO_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= sizeof(int);
    data_size *= MULTI_DATASET_IO_TEST_NDSETS;

    if (NULL == (write_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        TEST_ERROR
    }

    if (NULL == (read_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        TEST_ERROR
    }

    /* Select this rank's portion of the dataspace */
    for (i = 0; i < MULTI_DATASET_IO_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = (hsize_t)mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    /* Setup memory space for write_buf */
    {
        hsize_t mdims[] = { data_size / MULTI_DATASET_IO_TEST_NDSETS / sizeof(int) };

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(multi_dset_open) {
            size_t buf_start_idx;

            TESTING_2("keeping datasets open")

            /* Loop over datasets */
            for(i = 0; i < MULTI_DATASET_IO_TEST_NDSETS; i++) {
                size_t buf_end_idx;

                /* Set dataset name */
                sprintf(dset_name, "dset%d", (int)i);

                /* Create the dataset asynchronously */
                if((dset_id[i] = H5Dcreate_async(file_id, dset_name, H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_dset_open)

                /* Initialize write_buf.  Must use a new slice of write_buf for
                 * each dset since we can't overwrite the buffers until I/O is done. */
                buf_start_idx = i * (data_size / MULTI_DATASET_IO_TEST_NDSETS / sizeof(int));
                buf_end_idx = buf_start_idx + (data_size / MULTI_DATASET_IO_TEST_NDSETS / sizeof(int));
                for (j = buf_start_idx; j < buf_end_idx; j++)
                    ((int *) write_buf)[j] = mpi_rank;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[i], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &write_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_open)
            } /* end for */

            /* Flush the file asynchronously.  This will effectively work as a
             * barrier, guaranteeing the read takes place after the write. */
            if(H5Fflush_async(file_id, H5F_SCOPE_LOCAL, es_id) < 0)
                PART_TEST_ERROR(multi_dset_open)

            /* Loop over datasets */
            for(i = 0; i < MULTI_DATASET_IO_TEST_NDSETS; i++) {
                buf_start_idx = i * (data_size / MULTI_DATASET_IO_TEST_NDSETS / sizeof(int));

                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id[i], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &read_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_open)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_dset_open)
            if(op_failed)
                PART_TEST_ERROR(multi_dset_open)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(multi_dset_open)
                } /* end if */

            /* Close the datasets */
            for(i = 0; i < MULTI_DATASET_IO_TEST_NDSETS; i++)
                if(H5Dclose(dset_id[i]) < 0)
                    PART_TEST_ERROR(multi_dset_open)

            PASSED();
        } PART_END(multi_dset_open);

        PART_BEGIN(multi_dset_close) {
            size_t buf_start_idx;

            TESTING_2("closing datasets between I/O")

            /* Loop over datasets */
            for(i = 0; i < MULTI_DATASET_IO_TEST_NDSETS; i++) {
                size_t buf_end_idx;

                /* Set dataset name */
                sprintf(dset_name, "dset%d", (int)i);

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id, dset_name, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_dset_close)

                /* Initialize write_buf. */
                buf_start_idx = i * (data_size / MULTI_DATASET_IO_TEST_NDSETS / sizeof(int));
                buf_end_idx = buf_start_idx + (data_size / MULTI_DATASET_IO_TEST_NDSETS / sizeof(int));
                for (j = buf_start_idx; j < buf_end_idx; j++)
                    ((int *) write_buf)[j] = mpi_rank * 10;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[0], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &write_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_close)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_close)
            } /* end for */

            /* Flush the file asynchronously.  This will effectively work as a
             * barrier, guaranteeing the read takes place after the write. */
            if(H5Fflush_async(file_id, H5F_SCOPE_LOCAL, es_id) < 0)
                PART_TEST_ERROR(multi_dset_close)

            /* Loop over datasets */
            for(i = 0; i < MULTI_DATASET_IO_TEST_NDSETS; i++) {
                /* Set dataset name */
                sprintf(dset_name, "dset%d", (int)i);

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id, dset_name, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_dset_close)

                /* Read the dataset asynchronously */
                buf_start_idx = i * (data_size / MULTI_DATASET_IO_TEST_NDSETS / sizeof(int));
                if(H5Dread_async(dset_id[0], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &read_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_close)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_close)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_dset_close)
            if(op_failed)
                PART_TEST_ERROR(multi_dset_close)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(multi_dset_close)
                } /* end if */

            PASSED();
        } PART_END(multi_dset_close);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if(H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5Sclose(mspace_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(space_id);
        H5Sclose(mspace_id);
        for(i = 0; i < MULTI_DATASET_IO_TEST_NDSETS; i++)
            H5Dclose(dset_id[i]);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
}
#undef MULTI_DATASET_IO_TEST_SPACE_RANK
#undef MULTI_DATASET_IO_TEST_NDSETS

/*
 * Create multiple files, each with a single dataset. Each rank writes
 * to a portion of each dataset and reads from a portion of each dataset.
 */
#define MULTI_FILE_DATASET_IO_TEST_SPACE_RANK 2
#define MULTI_FILE_DATASET_IO_TEST_NFILES 5
static int
test_multi_file_dataset_io(void)
{
    hsize_t *dims = NULL;
    hsize_t  start[MULTI_FILE_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  stride[MULTI_FILE_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  count[MULTI_FILE_DATASET_IO_TEST_SPACE_RANK];
    hsize_t  block[MULTI_FILE_DATASET_IO_TEST_SPACE_RANK];
    hbool_t  op_failed;
    size_t   i, j, data_size, num_in_progress;
    hid_t    fapl_id = H5I_INVALID_HID;
    hid_t    file_id[MULTI_FILE_DATASET_IO_TEST_NFILES] =
                          { H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID };
    hid_t    dset_id[MULTI_FILE_DATASET_IO_TEST_NFILES] =
                          { H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID,
                            H5I_INVALID_HID };
    hid_t    space_id = H5I_INVALID_HID;
    hid_t    mspace_id = H5I_INVALID_HID;
    hid_t    es_id = H5I_INVALID_HID;
    char     file_name[32];
    int     *write_buf = NULL;
    int     *read_buf = NULL;

    TESTING_MULTIPART("multi file dataset I/O")

    TESTING_2("test setup")

    if ((fapl_id = create_mpi_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    /* Create dataspace */
    if (generate_random_parallel_dimensions(MULTI_FILE_DATASET_IO_TEST_SPACE_RANK, &dims) < 0)
        TEST_ERROR

    /* Create dataspace */
    if((space_id = H5Screate_simple(MULTI_FILE_DATASET_IO_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Calculate size of data buffers */
    for (i = 1, data_size = 1; i < MULTI_FILE_DATASET_IO_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= sizeof(int);
    data_size *= MULTI_FILE_DATASET_IO_TEST_NFILES;

    if (NULL == (write_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        TEST_ERROR
    }

    if (NULL == (read_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        TEST_ERROR
    }

    /* Select this rank's portion of the dataspace */
    for (i = 0; i < MULTI_FILE_DATASET_IO_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = (hsize_t)mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    /* Setup memory space for write_buf */
    {
        hsize_t mdims[] = { data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int) };

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(multi_file_dset_open) {
            size_t buf_start_idx;

            TESTING_2("keeping files and datasets open")

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++) {
                size_t buf_end_idx;

                /* Set file name */
                sprintf(file_name, PAR_ASYNC_VOL_TEST_FILE_PRINTF, (int)i);

                /* Create file asynchronously */
                if((file_id[i] = H5Fcreate_async(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)
                if((int)i > max_printf_file)
                    max_printf_file = i;

                /* Create the dataset asynchronously */
                if((dset_id[i] = H5Dcreate_async(file_id[i], "dset", H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

                /* Initialize write_buf.  Must use a new slice of write_buf for
                 * each dset since we can't overwrite the buffers until I/O is done. */
                buf_start_idx = i * (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                buf_end_idx = buf_start_idx + (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                for (j = buf_start_idx; j < buf_end_idx; j++)
                    ((int *) write_buf)[j] = mpi_rank;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[i], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &write_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)
            } /* end for */

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++) {
                /* Flush the dataset asynchronously.  This will effectively work as a
                 * barrier, guaranteeing the read takes place after the write. */
                if(H5Oflush_async(dset_id[i], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

                /* Read the dataset asynchronously */
                buf_start_idx = i * (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                if(H5Dread_async(dset_id[i], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &read_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_dset_open)
            if(op_failed)
                PART_TEST_ERROR(multi_file_dset_open)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(multi_file_dset_open)
                } /* end if */

            /* Close the datasets */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++)
                if(H5Dclose(dset_id[i]) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

            PASSED();
        } PART_END(multi_file_dset_open);

        PART_BEGIN(multi_file_dset_dclose) {
            size_t buf_start_idx;

            TESTING_2("closing datasets between I/O")

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++) {
                size_t buf_end_idx;

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[i], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

                /* Initialize write_buf. */
                buf_start_idx = i * (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                buf_end_idx = buf_start_idx + (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                for (j = buf_start_idx; j < buf_end_idx; j++)
                    ((int *) write_buf)[j] = mpi_rank * 10;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[0], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &write_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)
            } /* end for */

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++) {
                /* Flush the file asynchronously.  This will effectively work as a
                 * barrier, guaranteeing the read takes place after the write. */
                if(H5Fflush_async(file_id[i], H5F_SCOPE_LOCAL, es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[i], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

                /* Read the dataset asynchronously */
                buf_start_idx = i * (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                if(H5Dread_async(dset_id[0], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &read_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_dset_dclose)
            if(op_failed)
                PART_TEST_ERROR(multi_file_dset_dclose)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(multi_file_dset_dclose)
                } /* end if */

            /* Close the files */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++)
                if(H5Fclose(file_id[i]) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

            PASSED();
        } PART_END(multi_file_dset_dclose);

        PART_BEGIN(multi_file_dset_fclose) {
            size_t buf_start_idx;

            TESTING_2("closing files between I/O")

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++) {
                size_t buf_end_idx;

                /* Set file name */
                sprintf(file_name, PAR_ASYNC_VOL_TEST_FILE_PRINTF, (int)i);

                /* Open the file asynchronously */
                if((file_id[0] = H5Fopen_async(file_name, H5F_ACC_RDWR, fapl_id, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[0], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Initialize write_buf. */
                buf_start_idx = i * (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                buf_end_idx = buf_start_idx + (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                for (j = buf_start_idx; j < buf_end_idx; j++)
                    ((int *) write_buf)[j] = mpi_rank + 5;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[0], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &write_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Close the file asynchronously */
                if(H5Fclose_async(file_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_dset_fclose)
            if(op_failed)
                PART_TEST_ERROR(multi_file_dset_fclose)

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++) {
                /* Set file name */
                sprintf(file_name, PAR_ASYNC_VOL_TEST_FILE_PRINTF, (int)i);

                /* Open the file asynchronously */
                if((file_id[0] = H5Fopen_async(file_name, H5F_ACC_RDONLY, fapl_id, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[0], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Read the dataset asynchronously */
                buf_start_idx = i * (data_size / MULTI_FILE_DATASET_IO_TEST_NFILES / sizeof(int));
                if(H5Dread_async(dset_id[0], H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &read_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Close the file asynchronously */
                if(H5Fclose_async(file_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_dset_fclose)
            if(op_failed)
                PART_TEST_ERROR(multi_file_dset_fclose)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(multi_file_dset_fclose)
                } /* end if */

            PASSED();
        } PART_END(multi_file_dset_fclose);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
       TEST_ERROR

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if(H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5Sclose(mspace_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(space_id);
        H5Sclose(mspace_id);
        for(i = 0; i < MULTI_FILE_DATASET_IO_TEST_NFILES; i++) {
            H5Dclose(dset_id[i]);
            H5Fclose(file_id[i]);
        }
        H5Pclose(fapl_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
}
#undef MULTI_FILE_DATASET_IO_TEST_SPACE_RANK
#undef MULTI_FILE_DATASET_IO_TEST_NFILES

/*
 * Create multiple files, each with a single group and dataset. Each rank
 * writes to a portion of each dataset and reads from a portion of each dataset.
 */
#define MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK 2
#define MULTI_FILE_GRP_DSET_IO_TEST_NFILES 5
static int
test_multi_file_grp_dset_io(void)
{
    hsize_t *dims = NULL;
    hsize_t  start[MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK];
    hsize_t  stride[MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK];
    hsize_t  count[MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK];
    hsize_t  block[MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK];
    hbool_t  op_failed;
    size_t   i, j, data_size, num_in_progress;
    hid_t    file_id = H5I_INVALID_HID;
    hid_t    fapl_id = H5I_INVALID_HID;
    hid_t    grp_id = H5I_INVALID_HID;
    hid_t    dset_id = H5I_INVALID_HID;
    hid_t    space_id = H5I_INVALID_HID;
    hid_t    mspace_id = H5I_INVALID_HID;
    hid_t    es_id = H5I_INVALID_HID;
    char     file_name[32];
    int     *write_buf = NULL;
    int     *read_buf = NULL;

    TESTING_MULTIPART("multi file dataset I/O with groups")

    TESTING_2("test setup")

    if ((fapl_id = create_mpi_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    /* Create dataspace */
    if (generate_random_parallel_dimensions(MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK, &dims) < 0)
        TEST_ERROR

    /* Create dataspace */
    if((space_id = H5Screate_simple(MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Calculate size of data buffers */
    for (i = 1, data_size = 1; i < MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= sizeof(int);
    data_size *= MULTI_FILE_GRP_DSET_IO_TEST_NFILES;

    if (NULL == (write_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        TEST_ERROR
    }

    if (NULL == (read_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        TEST_ERROR
    }

    /* Select this rank's portion of the dataspace */
    for (i = 0; i < MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = (hsize_t)mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    /* Setup memory space for write_buf */
    {
        hsize_t mdims[] = { data_size / MULTI_FILE_GRP_DSET_IO_TEST_NFILES / sizeof(int) };

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(multi_file_grp_dset_no_kick) {
            size_t buf_start_idx;

            TESTING_2("without intermediate calls to H5ESwait()")

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_GRP_DSET_IO_TEST_NFILES; i++) {
                size_t buf_end_idx;

                /* Set file name */
                sprintf(file_name, PAR_ASYNC_VOL_TEST_FILE_PRINTF, (int)i);

                /* Create file asynchronously */
                if((file_id = H5Fcreate_async(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)
                if((int)i > max_printf_file)
                    max_printf_file = i;

                /* Create the group asynchronously */
                if((grp_id = H5Gcreate_async(file_id, "grp", H5P_DEFAULT,
                        H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Create the dataset asynchronously */
                if((dset_id = H5Dcreate_async(grp_id, "dset", H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Initialize write_buf.  Must use a new slice of write_buf for
                 * each dset since we can't overwrite the buffers until I/O is done. */
                buf_start_idx = i * (data_size / MULTI_FILE_GRP_DSET_IO_TEST_NFILES / sizeof(int));
                buf_end_idx = buf_start_idx + (data_size / MULTI_FILE_GRP_DSET_IO_TEST_NFILES / sizeof(int));
                for (j = buf_start_idx; j < buf_end_idx; j++)
                    ((int *) write_buf)[j] = mpi_rank;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &write_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Close the group asynchronously */
                if(H5Gclose_async(grp_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Close the file asynchronously */
                if(H5Fclose_async(file_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_grp_dset_no_kick)
            if(op_failed)
                PART_TEST_ERROR(multi_file_grp_dset_no_kick)

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_GRP_DSET_IO_TEST_NFILES; i++) {
                /* Set file name */
                sprintf(file_name, PAR_ASYNC_VOL_TEST_FILE_PRINTF, (int)i);

                /* Open the file asynchronously */
                if((file_id = H5Fopen_async(file_name, H5F_ACC_RDONLY, fapl_id, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Open the group asynchronously */
                if((grp_id = H5Gopen_async(file_id, "grp", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Open the dataset asynchronously */
                if((dset_id = H5Dopen_async(grp_id, "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Read the dataset asynchronously */
                buf_start_idx = i * (data_size / MULTI_FILE_GRP_DSET_IO_TEST_NFILES / sizeof(int));
                if(H5Dread_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &read_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Close the group asynchronously */
                if(H5Gclose_async(grp_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Close the file asynchronously */
                if(H5Fclose_async(file_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_grp_dset_no_kick)
            if(op_failed)
                PART_TEST_ERROR(multi_file_grp_dset_no_kick)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(multi_file_grp_dset_no_kick)
                } /* end if */

            PASSED();
        } PART_END(multi_file_grp_dset_no_kick);

        PART_BEGIN(multi_file_grp_dset_kick) {
            size_t buf_start_idx;

            TESTING_2("with intermediate calls to H5ESwait() (0 timeout)")

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_GRP_DSET_IO_TEST_NFILES; i++) {
                size_t buf_end_idx;

                /* Set file name */
                sprintf(file_name, PAR_ASYNC_VOL_TEST_FILE_PRINTF, (int)i);

                /* Create file asynchronously */
                if((file_id = H5Fcreate_async(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)
                if((int)i > max_printf_file)
                    max_printf_file = i;

                /* Create the group asynchronously */
                if((grp_id = H5Gcreate_async(file_id, "grp", H5P_DEFAULT,
                        H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Create the dataset asynchronously */
                if((dset_id = H5Dcreate_async(grp_id, "dset", H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Initialize write_buf.  Must use a new slice of write_buf for
                 * each dset since we can't overwrite the buffers until I/O is done. */
                buf_start_idx = i * (data_size / MULTI_FILE_GRP_DSET_IO_TEST_NFILES / sizeof(int));
                buf_end_idx = buf_start_idx + (data_size / MULTI_FILE_GRP_DSET_IO_TEST_NFILES / sizeof(int));
                for (j = buf_start_idx; j < buf_end_idx; j++)
                    ((int *) write_buf)[j] = mpi_rank;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &write_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Close the group asynchronously */
                if(H5Gclose_async(grp_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Close the file asynchronously */
                if(H5Fclose_async(file_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Kick the event stack to make progress */
                if(H5ESwait(es_id, 0, &num_in_progress, &op_failed) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)
                if(op_failed)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_grp_dset_kick)
            if(op_failed)
                PART_TEST_ERROR(multi_file_grp_dset_kick)

            /* Loop over files */
            for(i = 0; i < MULTI_FILE_GRP_DSET_IO_TEST_NFILES; i++) {
                /* Set file name */
                sprintf(file_name, PAR_ASYNC_VOL_TEST_FILE_PRINTF, (int)i);

                /* Open the file asynchronously */
                if((file_id = H5Fopen_async(file_name, H5F_ACC_RDONLY, fapl_id, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Open the group asynchronously */
                if((grp_id = H5Gopen_async(file_id, "grp", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Open the dataset asynchronously */
                if((dset_id = H5Dopen_async(grp_id, "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Read the dataset asynchronously */
                buf_start_idx = i * (data_size / MULTI_FILE_GRP_DSET_IO_TEST_NFILES / sizeof(int));
                if(H5Dread_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                        &read_buf[buf_start_idx], es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Close the group asynchronously */
                if(H5Gclose_async(grp_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Close the file asynchronously */
                if(H5Fclose_async(file_id, es_id) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Kick the event stack to make progress */
                if(H5ESwait(es_id, 0, &num_in_progress, &op_failed) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)
                if(op_failed)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_grp_dset_kick)
            if(op_failed)
                PART_TEST_ERROR(multi_file_grp_dset_kick)

            /* Verify the read data */
            for (i = 0; i < data_size / sizeof(int); i++)
                if(write_buf[i] != read_buf[i]) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    PART_ERROR(multi_file_grp_dset_kick)
                } /* end if */

            PASSED();
        } PART_END(multi_file_grp_dset_kick);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
       TEST_ERROR

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if(H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5Sclose(mspace_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(space_id);
        H5Sclose(mspace_id);
        H5Dclose(dset_id);
        H5Gclose(grp_id);
        H5Fclose(file_id);
        H5Pclose(fapl_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
}
#undef MULTI_FILE_GRP_DSET_IO_TEST_SPACE_RANK
#undef MULTI_FILE_GRP_DSET_IO_TEST_NFILES

/*
 * Creates a single file and dataset, then each rank writes to a portion
 * of the dataset. Next, the dataset is continually extended in the first
 * dimension by 1 "row" per mpi rank and partially written to by each rank.
 * Finally, each rank reads from a portion of the dataset.
 */
#define SET_EXTENT_TEST_SPACE_RANK 2
#define SET_EXTENT_TEST_NUM_EXTENDS 6
static int
test_set_extent(void)
{
    hsize_t *dims = NULL;
    hsize_t *maxdims = NULL;
    hsize_t *cdims = NULL;
    hsize_t  start[SET_EXTENT_TEST_SPACE_RANK];
    hsize_t  stride[SET_EXTENT_TEST_SPACE_RANK];
    hsize_t  count[SET_EXTENT_TEST_SPACE_RANK];
    hsize_t  block[SET_EXTENT_TEST_SPACE_RANK];
    hbool_t  op_failed;
    size_t   i, j, data_size, num_in_progress;
    hid_t    file_id = H5I_INVALID_HID;
    hid_t    fapl_id = H5I_INVALID_HID;
    hid_t    dset_id = H5I_INVALID_HID;
    hid_t    dcpl_id = H5I_INVALID_HID;
    hid_t    space_id = H5I_INVALID_HID;
    hid_t    space_id_out = H5I_INVALID_HID;
    hid_t    mspace_id = H5I_INVALID_HID;
    hid_t    es_id = H5I_INVALID_HID;
    int     *write_buf = NULL;
    int     *read_buf = NULL;

    TESTING("extending dataset")

    if ((fapl_id = create_mpi_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    /* Create dataspace */
    if (generate_random_parallel_dimensions(SET_EXTENT_TEST_SPACE_RANK, &dims) < 0)
        TEST_ERROR

    if (NULL == (maxdims = HDmalloc(SET_EXTENT_TEST_SPACE_RANK * sizeof(hsize_t)))) {
        H5_FAILED();
        HDprintf("    couldn't allocate max dataspace dimension buffer\n");
        TEST_ERROR
    }

    if (NULL == (cdims = HDmalloc(SET_EXTENT_TEST_SPACE_RANK * sizeof(hsize_t)))) {
        H5_FAILED();
        HDprintf("    couldn't allocate chunk dimension buffer\n");
        TEST_ERROR
    }

    for(i = 0; i < SET_EXTENT_TEST_SPACE_RANK; i++) {
        maxdims[i] = (i == 0) ? dims[i] + (SET_EXTENT_TEST_NUM_EXTENDS * mpi_size) : dims[i];
        cdims[i] = (dims[i] == 1) ? 1 : dims[i] / 2;
    }

    /* Create file dataspace */
    if((space_id = H5Screate_simple(SET_EXTENT_TEST_SPACE_RANK, dims, maxdims)) < 0)
        TEST_ERROR

    /* Create DCPL */
    if((dcpl_id = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        TEST_ERROR

    /* Set chunking */
    if(H5Pset_chunk(dcpl_id, SET_EXTENT_TEST_SPACE_RANK, cdims) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Create file asynchronously */
    if((file_id = H5Fcreate_async(PAR_ASYNC_VOL_TEST_FILE, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, es_id)) < 0)
        TEST_ERROR

    /* Create the dataset asynchronously */
    if((dset_id = H5Dcreate_async(file_id, "dset", H5T_NATIVE_INT, space_id,
            H5P_DEFAULT, dcpl_id, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Calculate size of data buffers */
    for (i = 1, data_size = 1; i < SET_EXTENT_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= sizeof(int);
    data_size *= SET_EXTENT_TEST_NUM_EXTENDS;

    if (NULL == (write_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        TEST_ERROR
    }

    if (NULL == (read_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        TEST_ERROR
    }

    /* Select this rank's portion of the dataspace */
    for (i = 0; i < SET_EXTENT_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = (hsize_t)mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    /* Setup memory space for write_buf */
    {
        hsize_t mdims[] = { data_size / SET_EXTENT_TEST_NUM_EXTENDS / sizeof(int) };

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    /* Initialize write_buf */
    for (i = 0; i < data_size / sizeof(int); i++)
        ((int *) write_buf)[i] = mpi_rank;

    /* Extend the dataset in the first dimension n times, extending by 1 "row" per
     * mpi rank involved on each iteration. Each rank will claim one of the new
     * "rows" for I/O in an interleaved fashion. */
    for(i = 0; i < SET_EXTENT_TEST_NUM_EXTENDS; i++) {
        htri_t tri_ret;

        /* No need to extend on the first iteration */
        if(i) {
            /* Extend datapace */
            dims[0] += (hsize_t)mpi_size;
            if(H5Sset_extent_simple(space_id, SET_EXTENT_TEST_SPACE_RANK, dims, maxdims) < 0)
                TEST_ERROR

            /* Extend dataset asynchronously */
            if(H5Dset_extent_async(dset_id, dims, es_id) < 0)
                TEST_ERROR

            /* Select hyperslab in file space to match new region */
            for (j = 0; j < SET_EXTENT_TEST_SPACE_RANK; j++) {
                if (j == 0) {
                    start[j] = (hsize_t)mpi_rank;
                    block[j] = 1;
                    stride[j] = mpi_size;
                    count[j] = i + 1;
                }
                else {
                    start[j] = 0;
                    block[j] = dims[j];
                    stride[j] = 1;
                    count[j] = 1;
                }
            }

            if (H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
                H5_FAILED();
                HDprintf("    couldn't select hyperslab for dataset write\n");
                goto error;
            }

            /* Adjust memory dataspace to match as well */
            {
                hsize_t mdims[] = { (i + 1) * (data_size / SET_EXTENT_TEST_NUM_EXTENDS / sizeof(int)) };

                if(H5Sset_extent_simple(mspace_id, 1, mdims, NULL) < 0)
                    TEST_ERROR

                if(H5Sselect_all(mspace_id) < 0)
                    TEST_ERROR
            }
        } /* end if */

        /* Get dataset dataspace */
        if((space_id_out = H5Dget_space_async(dset_id, es_id)) < 0)
            TEST_ERROR

        /* Verify extent is correct */
        if((tri_ret = H5Sextent_equal(space_id, space_id_out)) < 0)
            TEST_ERROR
        if(!tri_ret)
            FAIL_PUTS_ERROR("    dataspaces are not equal\n");

        /* Close output dataspace */
        if(H5Sclose(space_id_out) < 0)
            TEST_ERROR

        /* Write the dataset slice asynchronously */
        if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
                write_buf, es_id) < 0)
            TEST_ERROR
    }

    /* Flush the dataset asynchronously.  This will effectively work as a
     * barrier, guaranteeing the read takes place after the write. */
    if(H5Oflush_async(dset_id, es_id) < 0)
        TEST_ERROR

    /* Read the entire dataset asynchronously */
    if(H5Dread_async(dset_id, H5T_NATIVE_INT, mspace_id, space_id, H5P_DEFAULT,
            read_buf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for (i = 0; i < data_size / sizeof(int); i++)
        if(write_buf[i] != read_buf[i]) {
            H5_FAILED();
            HDprintf("    data verification failed, expected %d but got %d\n", write_buf[i], read_buf[i]);
            goto error;
        } /* end if */

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }

    if (cdims) {
        HDfree(cdims);
        cdims = NULL;
    }

    if (maxdims) {
        HDfree(maxdims);
        maxdims = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if(H5Dclose(dset_id) < 0)
        TEST_ERROR
    if(H5Fclose(file_id) < 0)
        TEST_ERROR
    if(H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5Sclose(mspace_id) < 0)
        TEST_ERROR
    if(H5Pclose(dcpl_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (cdims) HDfree(cdims);
        if (maxdims) HDfree(maxdims);
        if (dims) HDfree(dims);
        H5Sclose(space_id);
        H5Sclose(mspace_id);
        H5Sclose(space_id_out);
        H5Dclose(dset_id);
        H5Pclose(dcpl_id);
        H5Fclose(file_id);
        H5Pclose(fapl_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
}
#undef SET_EXTENT_TEST_SPACE_RANK
#undef SET_EXTENT_TEST_NUM_EXTENDS

/*
 * Cleanup temporary test files
 */
static void
cleanup_files(void)
{
    char file_name[64];
    int i;

    if(MAINPROCESS) {
        H5Fdelete(PAR_ASYNC_VOL_TEST_FILE, H5P_DEFAULT);
        for(i = 0; i <= max_printf_file; i++) {
            snprintf(file_name, 64, PAR_ASYNC_VOL_TEST_FILE_PRINTF, i);
            H5Fdelete(file_name, H5P_DEFAULT);
        } /* end for */
    }
}

int
vol_async_test_parallel(void)
{
    size_t i;
    int    nerrors;

    if (MAINPROCESS) {
        HDprintf("**********************************************\n");
        HDprintf("*                                            *\n");
        HDprintf("*      VOL Parallel Async Tests              *\n");
        HDprintf("*                                            *\n");
        HDprintf("**********************************************\n\n");
    }

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(par_async_tests); i++) {
        nerrors += (*par_async_tests[i])() ? 1 : 0;

        if (MPI_SUCCESS != MPI_Barrier(MPI_COMM_WORLD)) {
            if (MAINPROCESS)
                HDprintf("    MPI_Barrier() failed!\n");
        }
    }

    if (MAINPROCESS) {
        HDprintf("\n");
        HDprintf("Cleaning up testing files\n");
    }

    cleanup_files();

    return nerrors;
}

#else /* _H5ESpublic_H */

int
vol_async_test_parallel(void)
{
    if (MAINPROCESS) {
        HDprintf("**********************************************\n");
        HDprintf("*                                            *\n");
        HDprintf("*      VOL Parallel Async Tests              *\n");
        HDprintf("*                                            *\n");
        HDprintf("**********************************************\n\n");
    }

    HDprintf("SKIPPED due to no async support in HDF5 library\n");

    return 0;
}

#endif
