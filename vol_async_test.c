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

#include "vol_async_test.h"

#ifdef _H5ESpublic_H

static int test_one_dataset_io(void);
static int test_multi_dataset_io(void);
static int test_multi_file_dataset_io(void);
static int test_multi_file_grp_dset_io(void);
static int test_set_extent(void);
static int test_attribute_io(void);
static int test_attribute_io_tconv(void);
static int test_attribute_io_compound(void);

/*
 * The array of async tests to be performed.
 */
static int (*async_tests[])(void) = {
        test_one_dataset_io,
        test_multi_dataset_io,
        test_multi_file_dataset_io,
        test_multi_file_grp_dset_io,
        test_set_extent,
        test_attribute_io,
        test_attribute_io_tconv,
        test_attribute_io_compound,
};

/* Highest "printf" file created (starting at 0) */
int max_printf_file = -1;

/*
 * Create file and dataset, write to dataset
 */
static int
test_one_dataset_io(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t dset_id = H5I_INVALID_HID;
    hid_t space_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {6, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    int wbuf[6][10];
    int rbuf[6][10];
    int i, j;

    TESTING_MULTIPART("single dataset I/O")

    TESTING_2("test setup")

    /* Create dataspace */
    if((space_id = H5Screate_simple(2, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Create file asynchronously */
    if((file_id = H5Fcreate_async(ASYNC_VOL_TEST_FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Create the dataset asynchronously */
    if((dset_id = H5Dcreate_async(file_id, "dset", H5T_NATIVE_INT, space_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(single_dset_eswait) {
            TESTING_2("synchronization using H5ESwait()")

            /* Initialize wbuf */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    wbuf[i][j] = 10 * i + j;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    wbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_eswait)

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(single_dset_eswait)
            if(op_failed)
                PART_TEST_ERROR(single_dset_eswait)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    rbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_eswait)

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(single_dset_eswait)
            if(op_failed)
                PART_TEST_ERROR(single_dset_eswait)

            /* Verify the read data */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    if(wbuf[i][j] != rbuf[i][j]) {
                        H5_FAILED();
                        HDprintf("    data verification failed\n");
                        PART_ERROR(single_dset_eswait)
                    } /* end if */

            PASSED();
        } PART_END(single_dset_eswait);

        PART_BEGIN(single_dset_dclose) {
            TESTING_2("synchronization using H5Dclose()")

            /* Update wbuf */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    wbuf[i][j] += 6 * 10;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    wbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Close the dataset synchronously */
            if(H5Dclose(dset_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Re-open the dataset asynchronously */
            if((dset_id = H5Dopen_async(file_id, "dset", H5P_DEFAULT, es_id)) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    rbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Close the dataset synchronously */
            if(H5Dclose(dset_id) < 0)
                PART_TEST_ERROR(single_dset_dclose)

            /* Verify the read data */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    if(wbuf[i][j] != rbuf[i][j]) {
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

            /* Update wbuf */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    wbuf[i][j] += 6 * 10;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    wbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dflush)

            /* Flush the dataset asynchronously.  This will effectively work as a
             * barrier, guaranteeing the read takes place after the write. */
            if(H5Oflush_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_dflush)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    rbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dflush)

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(single_dset_dflush)
            if(op_failed)
                PART_TEST_ERROR(single_dset_dflush)

            /* Verify the read data */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    if(wbuf[i][j] != rbuf[i][j]) {
                        H5_FAILED();
                        HDprintf("    data verification failed\n");
                        PART_ERROR(single_dset_dflush)
                    } /* end if */

            PASSED();
        } PART_END(single_dset_dflush);

        PART_BEGIN(single_dset_fclose) {
            TESTING_2("synchronization using H5Fclose()")

            /* Update wbuf */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    wbuf[i][j] += 6 * 10;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    wbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the dataset asynchronously */
            if(H5Dclose_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the file synchronously */
            if(H5Fclose(file_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Reopen the file asynchronously. */
            if((file_id = H5Fopen_async(ASYNC_VOL_TEST_FILE, H5F_ACC_RDONLY, H5P_DEFAULT, es_id)) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Re-open the dataset asynchronously */
            if((dset_id = H5Dopen_async(file_id, "dset", H5P_DEFAULT, es_id)) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    rbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the dataset asynchronously */
            if(H5Dclose_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Close the file synchronously */
            if(H5Fclose(file_id) < 0)
                PART_TEST_ERROR(single_dset_fclose)

            /* Verify the read data */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    if(wbuf[i][j] != rbuf[i][j]) {
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

    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        H5Dclose(dset_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_one_dataset_io() */


/*
 * Create file and multiple datasets, write to them and read from them
 */
static int
test_multi_dataset_io(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t dset_id[5] = {H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID};
    hid_t space_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {6, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    char dset_name[32];
    int wbuf[5][6][10];
    int rbuf[5][6][10];
    int i, j, k;

    TESTING_MULTIPART("multi dataset I/O")

    TESTING_2("test setup")

    /* Create dataspace */
    if((space_id = H5Screate_simple(2, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Create file asynchronously */
    if((file_id = H5Fcreate_async(ASYNC_VOL_TEST_FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(multi_dset_open) {
            TESTING_2("keeping datasets open")

            /* Loop over datasets */
            for(i = 0; i < 5; i++) {
                /* Set dataset name */
                sprintf(dset_name, "dset%d", i);

                /* Create the dataset asynchronously */
                if((dset_id[i] = H5Dcreate_async(file_id, dset_name, H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_dset_open)

                /* Initialize wbuf.  Must use a new slice of wbuf for each dset
                 * since we can't overwrite the buffers until I/O is done. */
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        wbuf[i][j][k] = 6* 10 * i + 10 * j + k;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[i], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        wbuf[i], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_open)
            } /* end for */

            /* Flush the file asynchronously.  This will effectively work as a
             * barrier, guaranteeing the read takes place after the write. */
            if(H5Fflush_async(file_id, H5F_SCOPE_LOCAL, es_id) < 0)
                PART_TEST_ERROR(multi_dset_open)

            /* Loop over datasets */
            for(i = 0; i < 5; i++) {
                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id[i], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        rbuf[i], es_id) < 0)
                    PART_TEST_ERROR(multi_dset_open)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_dset_open)
            if(op_failed)
                PART_TEST_ERROR(multi_dset_open)
/*printf("\nwbuf:\n");
for(i = 0; i < 5; i++) {
    for(j = 0; j < 6; j++) {
        for(k = 0; k < 10; k++)
            printf("%d ", wbuf[i][j][k]);
        printf("\n");
    }
    printf("\n");
}
printf("\nrbuf:\n");
for(i = 0; i < 5; i++) {
    for(j = 0; j < 6; j++) {
        for(k = 0; k < 10; k++)
            printf("%d ", rbuf[i][j][k]);
        printf("\n");
    }
    printf("\n");
}*/
            /* Verify the read data */
            for(i = 0; i < 5; i++)
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        if(wbuf[i][j][k] != rbuf[i][j][k]) {
                            H5_FAILED();
                            HDprintf("    data verification failed\n");
                            PART_ERROR(multi_dset_open)
                        } /* end if */

            /* Close the datasets */
            for(i = 0; i < 5; i++)
                if(H5Dclose(dset_id[i]) < 0)
                    PART_TEST_ERROR(multi_dset_open)

            PASSED();
        } PART_END(multi_dset_open);

        PART_BEGIN(multi_dset_close) {
            TESTING_2("closing datasets between I/O")

            /* Loop over datasets */
            for(i = 0; i < 5; i++) {
                /* Set dataset name */
                sprintf(dset_name, "dset%d", i);

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id, dset_name, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_dset_close)

                /* Udpate wbuf */
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        wbuf[i][j][k] += 5 * 6* 10;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[0], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        wbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++) {
                /* Set dataset name */
                sprintf(dset_name, "dset%d", i);

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id, dset_name, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_dset_close)

                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id[0], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        rbuf[i], es_id) < 0)
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
/*printf("\nwbuf:\n");
for(i = 0; i < 5; i++) {
    for(j = 0; j < 6; j++) {
        for(k = 0; k < 10; k++)
            printf("%d ", wbuf[i][j][k]);
        printf("\n");
    }
    printf("\n");
}
printf("\nrbuf:\n");
for(i = 0; i < 5; i++) {
    for(j = 0; j < 6; j++) {
        for(k = 0; k < 10; k++)
            printf("%d ", rbuf[i][j][k]);
        printf("\n");
    }
    printf("\n");
}*/
            /* Verify the read data */
            for(i = 0; i < 5; i++)
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        if(wbuf[i][j][k] != rbuf[i][j][k]) {
                            H5_FAILED();
                            HDprintf("    data verification failed\n");
                            PART_ERROR(multi_dset_close)
                        } /* end if */

            PASSED();
        } PART_END(multi_dset_close);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if(H5Fclose_async(file_id, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        for(i = 0; i < 5; i++)
            H5Dclose(dset_id[i]);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_multi_dataset_io() */


/*
 * Create multiple files, each with a single dataset, write to them and read
 * from them
 */
static int
test_multi_file_dataset_io(void)
{
    hid_t file_id[5] = {H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID};
    hid_t dset_id[5] = {H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID, H5I_INVALID_HID};
    hid_t space_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {6, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    char file_name[32];
    int wbuf[5][6][10];
    int rbuf[5][6][10];
    int i, j, k;

    TESTING_MULTIPART("multi file dataset I/O")

    TESTING_2("test setup")

    /* Create dataspace */
    if((space_id = H5Screate_simple(2, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(multi_file_dset_open) {
            TESTING_2("keeping files and datasets open")

            /* Loop over files */
            for(i = 0; i < 5; i++) {
                /* Set file name */
                sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);

                /* Create file asynchronously */
                if((file_id[i] = H5Fcreate_async(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)
                if(i > max_printf_file)
                    max_printf_file = i;

                /* Create the dataset asynchronously */
                if((dset_id[i] = H5Dcreate_async(file_id[i], "dset", H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

                /* Initialize wbuf.  Must use a new slice of wbuf for each dset
                 * since we can't overwrite the buffers until I/O is done. */
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        wbuf[i][j][k] = 6* 10 * i + 10 * j + k;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[i], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        wbuf[i], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)
            } /* end for */

            /* Loop over files */
            for(i = 0; i < 5; i++) {
                /* Flush the dataset asynchronously.  This will effectively work as a
                 * barrier, guaranteeing the read takes place after the write. */
                if(H5Oflush_async(dset_id[i], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id[i], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        rbuf[i], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)
            } /* end for */

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_dset_open)
            if(op_failed)
                PART_TEST_ERROR(multi_file_dset_open)

            /* Verify the read data */
            for(i = 0; i < 5; i++)
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        if(wbuf[i][j][k] != rbuf[i][j][k]) {
                            H5_FAILED();
                            HDprintf("    data verification failed\n");
                            PART_ERROR(multi_file_dset_open)
                        } /* end if */

            /* Close the datasets */
            for(i = 0; i < 5; i++)
                if(H5Dclose(dset_id[i]) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

            PASSED();
        } PART_END(multi_file_dset_open);

        PART_BEGIN(multi_file_dset_dclose) {
            TESTING_2("closing datasets between I/O")

            /* Loop over files */
            for(i = 0; i < 5; i++) {
                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[i], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

                /* Udpate wbuf */
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        wbuf[i][j][k] += 5 * 6 * 10;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[0], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        wbuf[i], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

                /* Close the dataset asynchronously */
                if(H5Dclose_async(dset_id[0], es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)
            } /* end for */

            /* Loop over files */
            for(i = 0; i < 5; i++) {
                /* Flush the file asynchronously.  This will effectively work as a
                 * barrier, guaranteeing the read takes place after the write. */
                if(H5Fflush_async(file_id[i], H5F_SCOPE_LOCAL, es_id) < 0)
                    PART_TEST_ERROR(multi_file_dset_open)

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[i], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id[0], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        rbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++)
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        if(wbuf[i][j][k] != rbuf[i][j][k]) {
                            H5_FAILED();
                            HDprintf("    data verification failed\n");
                            PART_ERROR(multi_file_dset_dclose)
                        } /* end if */

            /* Close the files */
            for(i = 0; i < 5; i++)
                if(H5Fclose(file_id[i]) < 0)
                    PART_TEST_ERROR(multi_file_dset_dclose)

            PASSED();
        } PART_END(multi_file_dset_dclose);

        PART_BEGIN(multi_file_dset_fclose) {
            TESTING_2("closing files between I/O")

            /* Loop over files */
            for(i = 0; i < 5; i++) {
                /* Set file name */
                sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);

                /* Open the file asynchronously */
                if((file_id[0] = H5Fopen_async(file_name, H5F_ACC_RDWR, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[0], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Udpate wbuf */
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        wbuf[i][j][k] += 5 * 6 * 10;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id[0], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        wbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++) {
                /* Set file name */
                sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);

                /* Open the file asynchronously */
                if((file_id[0] = H5Fopen_async(file_name, H5F_ACC_RDONLY, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Open the dataset asynchronously */
                if((dset_id[0] = H5Dopen_async(file_id[0], "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_dset_fclose)

                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id[0], H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        rbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++)
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        if(wbuf[i][j][k] != rbuf[i][j][k]) {
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

    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        for(i = 0; i < 5; i++) {
            H5Dclose(dset_id[i]);
            H5Fclose(file_id[i]);
        } /* end for */
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_multi_file_dataset_io() */


/*
 * Create multiple files, each with a single group and dataset, write to them
 * and read from them
 */
static int
test_multi_file_grp_dset_io(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t grp_id = H5I_INVALID_HID;
    hid_t dset_id = H5I_INVALID_HID;
    hid_t space_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {6, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    char file_name[32];
    int wbuf[5][6][10];
    int rbuf[5][6][10];
    int i, j, k;

    TESTING_MULTIPART("multi file dataset I/O with groups")

    TESTING_2("test setup")

    /* Create dataspace */
    if((space_id = H5Screate_simple(2, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(multi_file_grp_dset_no_kick) {
            TESTING_2("without intermediate calls to H5ESwait()")

            /* Loop over files */
            for(i = 0; i < 5; i++) {
                /* Set file name */
                sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);

                /* Create file asynchronously */
                if((file_id = H5Fcreate_async(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)
                if(i > max_printf_file)
                    max_printf_file = i;

                /* Create the group asynchronously */
                if((grp_id = H5Gcreate_async(file_id, "grp", H5P_DEFAULT,
                        H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Create the dataset asynchronously */
                if((dset_id = H5Dcreate_async(grp_id, "dset", H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Initialize wbuf.  Must use a new slice of wbuf for each dset
                 * since we can't overwrite the buffers until I/O is done. */
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        wbuf[i][j][k] = 6* 10 * i + 10 * j + k;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        wbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++) {
                /* Set file name */
                sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);

                /* Open the file asynchronously */
                if((file_id = H5Fopen_async(file_name, H5F_ACC_RDONLY, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Open the group asynchronously */
                if((grp_id = H5Gopen_async(file_id, "grp", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Open the dataset asynchronously */
                if((dset_id = H5Dopen_async(grp_id, "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_no_kick)

                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        rbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++)
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        if(wbuf[i][j][k] != rbuf[i][j][k]) {
                            H5_FAILED();
                            HDprintf("    data verification failed\n");
                            PART_ERROR(multi_file_grp_dset_no_kick)
                        } /* end if */

            PASSED();
        } PART_END(multi_file_grp_dset_no_kick);

        PART_BEGIN(multi_file_grp_dset_kick) {
            TESTING_2("with intermediate calls to H5ESwait() (0 timeout)")

            /* Loop over files */
            for(i = 0; i < 5; i++) {
                /* Set file name */
                sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);

                /* Create file asynchronously */
                if((file_id = H5Fcreate_async(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)
                if(i > max_printf_file)
                    max_printf_file = i;

                /* Create the group asynchronously */
                if((grp_id = H5Gcreate_async(file_id, "grp", H5P_DEFAULT,
                        H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Create the dataset asynchronously */
                if((dset_id = H5Dcreate_async(grp_id, "dset", H5T_NATIVE_INT, space_id,
                        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Udpate wbuf */
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        wbuf[i][j][k] += 5 * 6 * 10;

                /* Write the dataset asynchronously */
                if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        wbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++) {
                /* Set file name */
                sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);

                /* Open the file asynchronously */
                if((file_id = H5Fopen_async(file_name, H5F_ACC_RDONLY, H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Open the group asynchronously */
                if((grp_id = H5Gopen_async(file_id, "grp", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Open the dataset asynchronously */
                if((dset_id = H5Dopen_async(grp_id, "dset", H5P_DEFAULT, es_id)) < 0)
                    PART_TEST_ERROR(multi_file_grp_dset_kick)

                /* Read the dataset asynchronously */
                if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                        rbuf[i], es_id) < 0)
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
            for(i = 0; i < 5; i++)
                for(j = 0; j < 6; j++)
                    for(k = 0; k < 10; k++)
                        if(wbuf[i][j][k] != rbuf[i][j][k]) {
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

    if(H5Sclose(space_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        H5Gclose(grp_id);
        H5Dclose(dset_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_multi_file_grp_dset_io() */


/*
 * Create file and dataset, write to dataset
 */
static int
test_set_extent(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t dset_id = H5I_INVALID_HID;
    hid_t fspace_id = H5I_INVALID_HID;
    hid_t fspace_out = H5I_INVALID_HID;
    hid_t mspace_id = H5I_INVALID_HID;
    hid_t dcpl_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {1, 10};
    hsize_t mdims[2] = {6, 10};
    hsize_t cdims[2] = {2, 3};
    hsize_t start[2] = {0, 0};
    hsize_t count[2] = {1, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    htri_t tri_ret;
    int wbuf[6][10];
    int rbuf[6][10];
    int i, j;

    TESTING("extending dataset")

    /* Create file dataspace */
    if((fspace_id = H5Screate_simple(2, dims, mdims)) < 0)
        TEST_ERROR

    /* Create memory dataspace */
    if((mspace_id = H5Screate_simple(1, &dims[1], NULL)) < 0)
        TEST_ERROR

    /* Create DCPL */
    if((dcpl_id = H5Pcreate(H5P_DATASET_CREATE)) < 0)
        TEST_ERROR

    /* Set chunking */
    if(H5Pset_chunk(dcpl_id, 2, cdims) < 0)
        TEST_ERROR

    /* Initialize wbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            wbuf[i][j] = 10 * i + j;

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Create file asynchronously */
    if((file_id = H5Fcreate_async(ASYNC_VOL_TEST_FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Create the dataset asynchronously */
    if((dset_id = H5Dcreate_async(file_id, "dset", H5T_NATIVE_INT, fspace_id,
            H5P_DEFAULT, dcpl_id, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Extend the first dataset from 1 to 6, 1 at a time */
    for(i = 0; i < 6; i++) {
        /* No need to extend on the first iteration */
        if(i) {
            /* Extend datapace */
            dims[0] = (hsize_t)(i + 1);
            if(H5Sset_extent_simple(fspace_id, 2, dims, mdims) < 0)
                TEST_ERROR

            /* Extend dataset asynchronously */
            if(H5Dset_extent_async(dset_id, dims, es_id) < 0)
                TEST_ERROR

            /* Select hyperslab in file space to match new region */
            start[0] = (hsize_t)i;
            if(H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, start, NULL, count, NULL) < 0)
                TEST_ERROR
        } /* end if */

        /* Get dataset dataspace */
        if((fspace_out = H5Dget_space_async(dset_id, es_id)) < 0)
            TEST_ERROR

        /* Verify extent is correct */
        if((tri_ret = H5Sextent_equal(fspace_id, fspace_out)) < 0)
            TEST_ERROR
        if(!tri_ret)
            FAIL_PUTS_ERROR("    dataspaces are not equal\n");

        /* Close output dataspace */
        if(H5Sclose(fspace_out) < 0)
            TEST_ERROR

        /* Write the dataset slice asynchronously */
        if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, mspace_id, fspace_id, H5P_DEFAULT,
                wbuf[i], es_id) < 0)
            TEST_ERROR
    } /* end for */

    /* Flush the dataset asynchronously.  This will effectively work as a
     * barrier, guaranteeing the read takes place after the write. */
    if(H5Oflush_async(dset_id, es_id) < 0)
        TEST_ERROR

    /* Read the entire dataset asynchronously */
    if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
            rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            if(wbuf[i][j] != rbuf[i][j])
                FAIL_PUTS_ERROR("    data verification failed\n")

    if(H5Dclose(dset_id) < 0)
        TEST_ERROR
    if(H5Fclose(file_id) < 0)
        TEST_ERROR
    if(H5Sclose(mspace_id) < 0)
        TEST_ERROR
    if(H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if(H5Pclose(dcpl_id) < 0)
        TEST_ERROR
    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Sclose(fspace_out);
        H5Pclose(dcpl_id);
        H5Dclose(dset_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_set_extent() */


/*
 * Create file, dataset, and attribute, write to attribute
 */
static int
test_attribute_io(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t dset_id = H5I_INVALID_HID;
    hid_t attr_id = H5I_INVALID_HID;
    hid_t space_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {6, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    int wbuf[6][10];
    int rbuf[6][10];
    int i, j;

    TESTING("attribute I/O")

    /* Create dataspace */
    if((space_id = H5Screate_simple(2, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Open file asynchronously */
    if((file_id = H5Fopen_async(ASYNC_VOL_TEST_FILE, H5F_ACC_RDWR, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Create the dataset asynchronously */
    if((dset_id = H5Dcreate_async(file_id, "attr_dset", H5T_NATIVE_INT, space_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Create the attribute asynchronously */
    if((attr_id = H5Acreate_async(dset_id, "attr", H5T_NATIVE_INT, space_id,
            H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Initialize wbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            wbuf[i][j] = 10 * i + j;

    /* Write the attribute asynchronously */
    if(H5Awrite_async(attr_id, H5T_NATIVE_INT, wbuf, es_id) < 0)
        TEST_ERROR

    /* Flush the dataset asynchronously.  This will effectively work as a
     * barrier, guaranteeing the read takes place after the write. */
    if(H5Oflush_async(dset_id, es_id) < 0)
        TEST_ERROR

    /* Read the attribute asynchronously */
    if(H5Aread_async(attr_id, H5T_NATIVE_INT, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            if(wbuf[i][j] != rbuf[i][j])
                FAIL_PUTS_ERROR("    data verification failed\n");

    /* Close the attribute asynchronously */
    if(H5Aclose_async(attr_id, es_id) < 0)
        TEST_ERROR

    /* Open the attribute asynchronously */
    if((attr_id = H5Aopen_async(dset_id, "attr", H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Read the attribute asynchronously */
    if(H5Aread_async(attr_id, H5T_NATIVE_INT, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            if(wbuf[i][j] != rbuf[i][j])
                FAIL_PUTS_ERROR("    data verification failed\n");

    /* Close out of order to see if it trips things up */
    if(H5Dclose_async(dset_id, es_id) < 0)
        TEST_ERROR
    if(H5Aclose_async(attr_id, es_id) < 0)
        TEST_ERROR
    if(H5Fclose_async(file_id, es_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        H5Aclose(attr_id);
        H5Dclose(dset_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_attribute_io() */


/*
 * Create file, dataset, and attribute, write to attribute with type conversion
 */
static int
test_attribute_io_tconv(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t attr_id = H5I_INVALID_HID;
    hid_t space_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {6, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    int wbuf[6][10];
    int rbuf[6][10];
    int i, j;

    TESTING("attribute I/O with type conversion")

    /* Create dataspace */
    if((space_id = H5Screate_simple(2, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Open file asynchronously */
    if((file_id = H5Fopen_async(ASYNC_VOL_TEST_FILE, H5F_ACC_RDWR, H5P_DEFAULT,
            es_id)) < 0)
        TEST_ERROR

    /* Create the attribute asynchronously by name */
    if((attr_id = H5Acreate_by_name_async(file_id, "attr_dset", "attr_tconv",
            H5T_STD_U16BE, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT,
            es_id)) < 0)
        TEST_ERROR

    /* Initialize wbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            wbuf[i][j] = 10 * i + j;

    /* Write the attribute asynchronously */
    if(H5Awrite_async(attr_id, H5T_NATIVE_INT, wbuf, es_id) < 0)
        TEST_ERROR

    /* Flush the dataset asynchronously.  This will effectively work as a
     * barrier, guaranteeing the read takes place after the write. */
    if(H5Fflush_async(file_id, H5F_SCOPE_LOCAL, es_id) < 0)
        TEST_ERROR

    /* Read the attribute asynchronously */
    if(H5Aread_async(attr_id, H5T_NATIVE_INT, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            if(wbuf[i][j] != rbuf[i][j])
                FAIL_PUTS_ERROR("    data verification failed\n");

    /* Close the attribute asynchronously */
    if(H5Aclose_async(attr_id, es_id) < 0)
        TEST_ERROR

    /* Open the attribute asynchronously */
    if((attr_id = H5Aopen_by_name_async(file_id, "attr_dset", "attr_tconv",
            H5P_DEFAULT, H5P_DEFAULT, es_id)) < 0)
        TEST_ERROR

    /* Read the attribute asynchronously */
    if(H5Aread_async(attr_id, H5T_NATIVE_INT, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            if(wbuf[i][j] != rbuf[i][j])
                FAIL_PUTS_ERROR("    data verification failed\n");

    /* Close */
    if(H5Aclose_async(attr_id, es_id) < 0)
        TEST_ERROR
    if(H5Fclose_async(file_id, es_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        H5Aclose(attr_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_attribute_io_tconv() */


/*
 * Create file, dataset, and attribute, write to attribute with compound type
 * conversion
 */
typedef struct tattr_cmpd_t {
    int a;
    int b;
} tattr_cmpd_t;

static int
test_attribute_io_compound(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t attr_id = H5I_INVALID_HID;
    hid_t space_id = H5I_INVALID_HID;
    hid_t mtype_id = H5I_INVALID_HID;
    hid_t ftype_id = H5I_INVALID_HID;
    hid_t mtypea_id = H5I_INVALID_HID;
    hid_t mtypeb_id = H5I_INVALID_HID;
    hid_t es_id = H5I_INVALID_HID;
    hsize_t dims[2] = {6, 10};
    size_t num_in_progress;
    hbool_t op_failed;
    tattr_cmpd_t wbuf[6][10];
    tattr_cmpd_t rbuf[6][10];
    tattr_cmpd_t fbuf[6][10];
    int i, j;

    TESTING("attribute I/O with compound type conversion")

    /* Create datatype */
    if((mtype_id = H5Tcreate (H5T_COMPOUND, sizeof(tattr_cmpd_t))) < 0)
        TEST_ERROR
    if(H5Tinsert(mtype_id, "a_name", HOFFSET(tattr_cmpd_t, a), H5T_NATIVE_INT) < 0)
        TEST_ERROR
    if(H5Tinsert(mtype_id, "b_name", HOFFSET(tattr_cmpd_t, b), H5T_NATIVE_INT) < 0)
        TEST_ERROR

    if((mtypea_id = H5Tcreate (H5T_COMPOUND, sizeof(tattr_cmpd_t))) < 0)
        TEST_ERROR
    if(H5Tinsert(mtypea_id, "a_name", HOFFSET(tattr_cmpd_t, a), H5T_NATIVE_INT) < 0)
        TEST_ERROR

    if((mtypeb_id = H5Tcreate (H5T_COMPOUND, sizeof(tattr_cmpd_t))) < 0)
        TEST_ERROR
    if(H5Tinsert(mtypeb_id, "b_name", HOFFSET(tattr_cmpd_t, b), H5T_NATIVE_INT) < 0)
        TEST_ERROR

    if((ftype_id = H5Tcreate (H5T_COMPOUND, 2 + 8)) < 0)
        TEST_ERROR
    if(H5Tinsert(ftype_id, "a_name", 0, H5T_STD_U16BE) < 0)
        TEST_ERROR
    if(H5Tinsert(ftype_id, "b_name", 2, H5T_STD_I64LE) < 0)
        TEST_ERROR

    /* Create dataspace */
    if((space_id = H5Screate_simple(2, dims, NULL)) < 0)
        TEST_ERROR

    /* Create event stack */
    if((es_id = H5EScreate()) <  0)
        TEST_ERROR

    /* Open file asynchronously */
    if((file_id = H5Fopen_async(ASYNC_VOL_TEST_FILE, H5F_ACC_RDWR, H5P_DEFAULT,
            es_id)) < 0)
        TEST_ERROR

    /* Create the attribute asynchronously by name */
    if((attr_id = H5Acreate_by_name_async(file_id, "attr_dset", "attr_cmpd",
            ftype_id, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT,
            es_id)) < 0)
        TEST_ERROR

    /* Initialize wbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            wbuf[i][j].a = 2 * (10 * i + j);
            wbuf[i][j].b = 2 * (10 * i + j) + 1;
        } /* end for */

    /* Write the attribute asynchronously */
    if(H5Awrite_async(attr_id, mtype_id, wbuf, es_id) < 0)
        TEST_ERROR

    /* Update fbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            fbuf[i][j].a = wbuf[i][j].a;
            fbuf[i][j].b = wbuf[i][j].b;
        } /* end for */

    /* Flush the dataset asynchronously.  This will effectively work as a
     * barrier, guaranteeing the read takes place after the write. */
    if(H5Fflush_async(file_id, H5F_SCOPE_LOCAL, es_id) < 0)
        TEST_ERROR

    /* Read the attribute asynchronously */
    if(H5Aread_async(attr_id, mtype_id, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            if(rbuf[i][j].a != fbuf[i][j].a)
                FAIL_PUTS_ERROR("    data verification failed\n");
            if(rbuf[i][j].b != fbuf[i][j].b)
                FAIL_PUTS_ERROR("    data verification failed\n");
        } /* end for */

    /* Clear the read buffer */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            rbuf[i][j].a = -2;
            rbuf[i][j].b = -2;
        } /* end for */

    /* Read the attribute asynchronously (element a only) */
    if(H5Aread_async(attr_id, mtypea_id, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            if(rbuf[i][j].a != fbuf[i][j].a)
                FAIL_PUTS_ERROR("    data verification failed\n");
            if(rbuf[i][j].b != -2)
                FAIL_PUTS_ERROR("    data verification failed\n");
        } /* end for */

    /* Clear the read buffer */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            rbuf[i][j].a = -2;
            rbuf[i][j].b = -2;
        } /* end for */

    /* Read the attribute asynchronously (element b only) */
    if(H5Aread_async(attr_id, mtypeb_id, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            if(rbuf[i][j].a != -2)
                FAIL_PUTS_ERROR("    data verification failed\n");
            if(rbuf[i][j].b != fbuf[i][j].b)
                FAIL_PUTS_ERROR("    data verification failed\n");
        } /* end for */

    /* Update wbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            wbuf[i][j].a += 2 * 6 * 10;
            wbuf[i][j].b += 2 * 6 * 10;
        } /* end for */

    /* Write the attribute asynchronously (element a only) */
    if(H5Awrite_async(attr_id, mtypea_id, wbuf, es_id) < 0)
        TEST_ERROR

    /* Update fbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            fbuf[i][j].a = wbuf[i][j].a;

    /* Flush the dataset asynchronously.  This will effectively work as a
     * barrier, guaranteeing the read takes place after the write. */
    if(H5Fflush_async(file_id, H5F_SCOPE_LOCAL, es_id) < 0)
        TEST_ERROR

    /* Clear the read buffer */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            rbuf[i][j].a = -2;
            rbuf[i][j].b = -2;
        } /* end for */

    /* Read the attribute asynchronously */
    if(H5Aread_async(attr_id, mtype_id, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            if(rbuf[i][j].a != fbuf[i][j].a)
                FAIL_PUTS_ERROR("    data verification failed\n");
            if(rbuf[i][j].b != fbuf[i][j].b)
                FAIL_PUTS_ERROR("    data verification failed\n");
        } /* end for */

    /* Update wbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            wbuf[i][j].a += 2 * 6 * 10;
            wbuf[i][j].b += 2 * 6 * 10;
        } /* end for */

    /* Write the attribute asynchronously (element b only) */
    if(H5Awrite_async(attr_id, mtypeb_id, wbuf, es_id) < 0)
        TEST_ERROR

    /* Update fbuf */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++)
            fbuf[i][j].b = wbuf[i][j].b;

    /* Flush the dataset asynchronously.  This will effectively work as a
     * barrier, guaranteeing the read takes place after the write. */
    if(H5Fflush_async(file_id, H5F_SCOPE_LOCAL, es_id) < 0)
        TEST_ERROR

    /* Clear the read buffer */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            rbuf[i][j].a = -2;
            rbuf[i][j].b = -2;
        } /* end for */

    /* Read the attribute asynchronously */
    if(H5Aread_async(attr_id, mtype_id, rbuf, es_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    /* Verify the read data */
    for(i = 0; i < 6; i++)
        for(j = 0; j < 10; j++) {
            if(rbuf[i][j].a != fbuf[i][j].a)
                FAIL_PUTS_ERROR("    data verification failed\n");
            if(rbuf[i][j].b != fbuf[i][j].b)
                FAIL_PUTS_ERROR("    data verification failed\n");
        } /* end for */

    /* Close */
    if(H5Aclose_async(attr_id, es_id) < 0)
        TEST_ERROR
    if(H5Fclose_async(file_id, es_id) < 0)
        TEST_ERROR
    if(H5Sclose(space_id) < 0)
        TEST_ERROR

    /* Wait for the event stack to complete */
    if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
        TEST_ERROR
    if(op_failed)
        TEST_ERROR

    if(H5ESclose(es_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        H5Aclose(attr_id);
        H5Fclose(file_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed);
        H5ESclose(es_id);
    } H5E_END_TRY;

    return 1;
} /* end test_attribute_io_compound() */


/*
 * Cleanup temporary test files
 */
static void
cleanup_files(void)
{
    char file_name[32];
    int i;

    H5Fdelete(ASYNC_VOL_TEST_FILE, H5P_DEFAULT);
    for(i = 0; i <= max_printf_file; i++) {
        sprintf(file_name, ASYNC_VOL_TEST_FILE_PRINTF, i);
        H5Fdelete(file_name, H5P_DEFAULT);
    } /* end for */
}


int
vol_async_test(void)
{
    size_t i;
    int    nerrors;

    HDprintf("**********************************************\n");
    HDprintf("*                                            *\n");
    HDprintf("*             VOL Async Tests                *\n");
    HDprintf("*                                            *\n");
    HDprintf("**********************************************\n\n");

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(async_tests); i++) {
        nerrors += (*async_tests[i])() ? 1 : 0;
    }

    HDprintf("\n");

    HDprintf("Cleaning up testing files\n");
    cleanup_files();

    return nerrors;
}

#else /* _H5ESpublic_H */

int
vol_async_test(void)
{
    HDprintf("**********************************************\n");
    HDprintf("*                                            *\n");
    HDprintf("*             VOL Async Tests                *\n");
    HDprintf("*                                            *\n");
    HDprintf("**********************************************\n\n");

    HDprintf("SKIPPED due to no async support in HDF5 library\n");

    return 0;
}

#endif /* _H5ESpublic_H */

