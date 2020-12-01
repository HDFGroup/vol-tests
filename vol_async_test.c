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

/*
 * The array of async tests to be performed.
 */
static int (*async_tests[])(void) = {
        test_one_dataset_io,
        test_multi_dataset_io,
        test_multi_file_dataset_io,
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
#if 0 /* Doesn't work with H5Dclose_async - change to H5Dflush?  Either way needs work */
        PART_BEGIN(single_dset_dclose_async) {
            TESTING_2("synchronization using H5Dclose_async()")

            /* Update wbuf */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    wbuf[i][j] += 6 * 10;

            /* Write the dataset asynchronously */
            if(H5Dwrite_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    wbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose_async)

            /* Close the dataset asynchronously.  This will effectively work as a
             * barrier, guaranteeing the read takes place after the write. */
            if(H5Dclose_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose_async)

            /* Re-open the dataset asynchronously */
            if((dset_id = H5Dopen_async(file_id, "dset", H5P_DEFAULT, es_id)) < 0)
                PART_TEST_ERROR(single_dset_dclose_async)

            /* Read the dataset asynchronously */
            if(H5Dread_async(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                    rbuf, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose_async)

            /* Close the dataset asynchronously */
            if(H5Dclose_async(dset_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose_async)

            /* Close the file asynchronously */
            if(H5Fclose_async(file_id, es_id) < 0)
                PART_TEST_ERROR(single_dset_dclose_async)

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(single_dset_dclose_async)
            if(op_failed)
                PART_TEST_ERROR(single_dset_dclose_async)

            /* Verify the read data */
            for(i = 0; i < 6; i++)
                for(j = 0; j < 10; j++)
                    if(wbuf[i][j] != rbuf[i][j]) {
                        H5_FAILED();
                        HDprintf("    data verification failed\n");
                        PART_ERROR(single_dset_dclose_async)
                    } /* end if */

            PASSED();
        } PART_END(single_dset_dclose_async);
#endif
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

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_dset_open)
            if(op_failed)
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

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_dset_close)
            if(op_failed)
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
                    TEST_ERROR
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

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_dset_open)
            if(op_failed)
                PART_TEST_ERROR(multi_file_dset_open)

            /* Loop over files */
            for(i = 0; i < 5; i++) {
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

            /* Wait for the event stack to complete */
            if(H5ESwait(es_id, H5ES_WAIT_FOREVER, &num_in_progress, &op_failed) < 0)
                PART_TEST_ERROR(multi_file_dset_dclose)
            if(op_failed)
                PART_TEST_ERROR(multi_file_dset_dclose)

            /* Loop over files */
            for(i = 0; i < 5; i++) {
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

