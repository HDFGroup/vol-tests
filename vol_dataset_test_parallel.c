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

#include "vol_dataset_test_parallel.h"

static int test_write_dataset_data_verification(void);
static int test_write_dataset_one_proc_0_selection(void);
static int test_write_dataset_one_proc_none_selection(void);
static int test_write_dataset_one_proc_all_selection(void);
static int test_write_dataset_hyper_file_all_mem(void);
static int test_write_dataset_all_file_hyper_mem(void);
static int test_write_dataset_point_file_all_mem(void);
static int test_write_dataset_all_file_point_mem(void);
static int test_write_dataset_hyper_file_point_mem(void);
static int test_write_dataset_point_file_hyper_mem(void);
static int test_read_dataset_one_proc_0_selection(void);
static int test_read_dataset_one_proc_none_selection(void);
static int test_read_dataset_one_proc_all_selection(void);
static int test_read_dataset_hyper_file_all_mem(void);
static int test_read_dataset_all_file_hyper_mem(void);
static int test_read_dataset_point_file_all_mem(void);
static int test_read_dataset_all_file_point_mem(void);
static int test_read_dataset_hyper_file_point_mem(void);
static int test_read_dataset_point_file_hyper_mem(void);

/*
 * The array of parallel dataset tests to be performed.
 */
static int (*par_dataset_tests[])(void) = {
    test_write_dataset_data_verification,
    test_write_dataset_one_proc_0_selection,
    test_write_dataset_one_proc_none_selection,
    test_write_dataset_one_proc_all_selection,
    test_write_dataset_hyper_file_all_mem,
    test_write_dataset_all_file_hyper_mem,
    test_write_dataset_point_file_all_mem,
    test_write_dataset_all_file_point_mem,
    test_write_dataset_hyper_file_point_mem,
    test_write_dataset_point_file_hyper_mem,
    test_read_dataset_one_proc_0_selection,
    test_read_dataset_one_proc_none_selection,
    test_read_dataset_one_proc_all_selection,
    test_read_dataset_hyper_file_all_mem,
    test_read_dataset_all_file_hyper_mem,
    test_read_dataset_point_file_all_mem,
    test_read_dataset_all_file_point_mem,
    test_read_dataset_hyper_file_point_mem,
    test_read_dataset_point_file_hyper_mem,
};

/*
 * A test to ensure that data is read back correctly from
 * a dataset after it has been written in paralle. The test
 * covers simple examples of using H5S_ALL selections,
 * hyperslab selections and point selections.
 */
#define DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK 3
#define DATASET_WRITE_DATA_VERIFY_TEST_NUM_POINTS 10
#define DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME "dataset_write_data_verification_test"
#define DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1 "dataset_write_data_verification_all"
#define DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2 "dataset_write_data_verification_hyperslab"
#define DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3 "dataset_write_data_verification_points"
static int
test_write_dataset_data_verification(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    hsize_t   start[DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK];
    hsize_t   stride[DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK];
    hsize_t   count[DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK];
    hsize_t   block[DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK];
    hsize_t  *points = NULL;
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING_MULTIPART("verification of dataset data using H5Dwrite then H5Dread");

    TESTING_2("test setup")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1);
        goto error;
    }
    H5E_BEGIN_TRY {
        H5Dclose(dset_id);
    } H5E_END_TRY;
    dset_id = H5I_INVALID_HID;

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2);
        goto error;
    }
    H5E_BEGIN_TRY {
        H5Dclose(dset_id);
    } H5E_END_TRY;
    dset_id = H5I_INVALID_HID;

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3);
        goto error;
    }
    H5E_BEGIN_TRY {
        H5Dclose(dset_id);
    } H5E_END_TRY;
    dset_id = H5I_INVALID_HID;

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Dwrite_all_read) {
            hbool_t op_failed = FALSE;

            TESTING_2("H5Dwrite using H5S_ALL then H5Dread")

            if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1);
                PART_ERROR(H5Dwrite_all_read);
            }

            /*
             * Write data to dataset on rank 0 only. All ranks will read the data back.
             */
            if (MAINPROCESS) {
                for (i = 0, data_size = 1; i < DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK; i++)
                    data_size *= dims[i];
                data_size *= DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE;

                if (NULL != (write_buf = HDmalloc(data_size))) {
                    for (i = 0; i < data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE; i++)
                        ((int *) write_buf)[i] = (int) i;

                    if (H5Dwrite(dset_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, write_buf) < 0)
                        op_failed = TRUE;
                }
                else
                    op_failed = TRUE;

                if (write_buf) {
                    HDfree(write_buf);
                    write_buf = NULL;
                }
            }

            if (MPI_SUCCESS != MPI_Allreduce(MPI_IN_PLACE, &op_failed, 1, MPI_C_BOOL, MPI_LAND, MPI_COMM_WORLD)) {
                H5_FAILED();
                HDprintf("    couldn't determine if dataset write on rank 0 succeeded\n");
                PART_ERROR(H5Dwrite_all_read);
            }

            if (op_failed == TRUE) {
                H5_FAILED();
                HDprintf("    dataset write on rank 0 failed!\n");
                PART_ERROR(H5Dwrite_all_read);
            }

            if (fspace_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Sclose(fspace_id);
                } H5E_END_TRY;
                fspace_id = H5I_INVALID_HID;
            }
            if (dset_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Dclose(dset_id);
                } H5E_END_TRY;
                dset_id = H5I_INVALID_HID;
            }

            /*
             * Close and re-open the file to ensure that the data gets written.
             */
            if (H5Gclose(group_id) < 0) {
                H5_FAILED();
                HDprintf("    failed to close test's container group\n");
                PART_ERROR(H5Dwrite_all_read);
            }
            if (H5Gclose(container_group) < 0) {
                H5_FAILED();
                HDprintf("    failed to close container group\n");
                PART_ERROR(H5Dwrite_all_read);
            }
            if (H5Fclose(file_id) < 0) {
                H5_FAILED();
                HDprintf("    failed to close file for data flushing\n");
                PART_ERROR(H5Dwrite_all_read);
            }
            if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
                PART_ERROR(H5Dwrite_all_read);
            }
            if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
                PART_ERROR(H5Dwrite_all_read);
            }
            if ((group_id = H5Gopen2(container_group, DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME);
                PART_ERROR(H5Dwrite_all_read);
            }

            if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1);
                PART_ERROR(H5Dwrite_all_read);
            }

            if ((fspace_id = H5Dget_space(dset_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataset dataspace\n");
                PART_ERROR(H5Dwrite_all_read);
            }

            if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataspace num points\n");
                PART_ERROR(H5Dwrite_all_read);
            }

            if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE))) {
                H5_FAILED();
                HDprintf("    couldn't allocate buffer for dataset read\n");
                PART_ERROR(H5Dwrite_all_read);
            }

            if (H5Dread(dset_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
                H5_FAILED();
                HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME1);
                PART_ERROR(H5Dwrite_all_read);
            }

            for (i = 0; i < (hsize_t) space_npoints; i++)
                if (((int *) read_buf)[i] != (int) i) {
                    H5_FAILED();
                    HDprintf("    H5S_ALL selection data verification failed\n");
                    PART_ERROR(H5Dwrite_all_read);
                }

            if (read_buf) {
                HDfree(read_buf);
                read_buf = NULL;
            }

            PASSED();
        } PART_END(H5Dwrite_all_read);

        if (write_buf) {
            HDfree(write_buf);
            write_buf = NULL;
        }
        if (read_buf) {
            HDfree(read_buf);
            read_buf = NULL;
        }
        if (fspace_id >= 0) {
            H5E_BEGIN_TRY {
                H5Sclose(fspace_id);
            } H5E_END_TRY;
            fspace_id = H5I_INVALID_HID;
        }
        if (dset_id >= 0) {
            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;
            dset_id = H5I_INVALID_HID;
        }

        PART_BEGIN(H5Dwrite_hyperslab_read) {
            TESTING_2("H5Dwrite using hyperslab selection then H5Dread")

            for (i = 1, data_size = 1; i < DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK; i++)
                data_size *= dims[i];
            data_size *= DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE;

            if (NULL == (write_buf = HDmalloc(data_size))) {
                H5_FAILED();
                HDprintf("    couldn't allocate buffer for dataset write\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            for (i = 0; i < data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE; i++)
                ((int *) write_buf)[i] = mpi_rank;

            /* Each MPI rank writes to a single row in the second dimension
             * and the entirety of the following dimensions. The combined
             * selections from all MPI ranks spans the first dimension.
             */
            for (i = 0; i < DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK; i++) {
                if (i == 0) {
                    start[i] = mpi_rank;
                    block[i] = 1;
                }
                else {
                    start[i] = 0;
                    block[i] = dims[i];
                }

                stride[i] = 1;
                count[i] = 1;
            }

            if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2);
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if ((fspace_id = H5Dget_space(dset_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataset dataspace\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if (H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
                H5_FAILED();
                HDprintf("    couldn't select hyperslab for dataset write\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            {
                hsize_t mdims[] = { data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE };

                if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create memory dataspace\n");
                    PART_ERROR(H5Dwrite_hyperslab_read);
                }
            }

            if (H5Dwrite(dset_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE, mspace_id, fspace_id, H5P_DEFAULT, write_buf) < 0) {
                H5_FAILED();
                HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2);
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if (write_buf) {
                HDfree(write_buf);
                write_buf = NULL;
            }
            if (mspace_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Sclose(mspace_id);
                } H5E_END_TRY;
                mspace_id = H5I_INVALID_HID;
            }
            if (fspace_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Sclose(fspace_id);
                } H5E_END_TRY;
                fspace_id = H5I_INVALID_HID;
            }
            if (dset_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Dclose(dset_id);
                } H5E_END_TRY;
                dset_id = H5I_INVALID_HID;
            }

            /*
             * Close and re-open the file to ensure that the data gets written.
             */
            if (H5Gclose(group_id) < 0) {
                H5_FAILED();
                HDprintf("    failed to close test's container group\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }
            if (H5Gclose(container_group) < 0) {
                H5_FAILED();
                HDprintf("    failed to close container group\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }
            if (H5Fclose(file_id) < 0) {
                H5_FAILED();
                HDprintf("    failed to close file for data flushing\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }
            if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
                PART_ERROR(H5Dwrite_hyperslab_read);
            }
            if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
                PART_ERROR(H5Dwrite_hyperslab_read);
            }
            if ((group_id = H5Gopen2(container_group, DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME);
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2);
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if ((fspace_id = H5Dget_space(dset_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataset dataspace\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataspace num points\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE))) {
                H5_FAILED();
                HDprintf("    couldn't allocate buffer for dataset read\n");
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            if (H5Dread(dset_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
                H5_FAILED();
                HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME2);
                PART_ERROR(H5Dwrite_hyperslab_read);
            }

            for (i = 0; i < (size_t) mpi_size; i++) {
                size_t j;

                for (j = 0; j < data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE; j++) {
                    if (((int *) read_buf)[j + (i * (data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE))] != (int) i) {
                        H5_FAILED();
                        HDprintf("    hyperslab selection data verification failed\n");
                        PART_ERROR(H5Dwrite_hyperslab_read);
                    }
                }
            }

            if (read_buf) {
                HDfree(read_buf);
                read_buf = NULL;
            }

            PASSED();
        } PART_END(H5Dwrite_hyperslab_read);

        if (write_buf) {
            HDfree(write_buf);
            write_buf = NULL;
        }
        if (read_buf) {
            HDfree(read_buf);
            read_buf = NULL;
        }
        if (fspace_id >= 0) {
            H5E_BEGIN_TRY {
                H5Sclose(fspace_id);
            } H5E_END_TRY;
            fspace_id = H5I_INVALID_HID;
        }
        if (mspace_id >= 0) {
            H5E_BEGIN_TRY {
                H5Sclose(mspace_id);
            } H5E_END_TRY;
            mspace_id = H5I_INVALID_HID;
        }
        if (dset_id >= 0) {
            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;
            dset_id = H5I_INVALID_HID;
        }

        PART_BEGIN(H5Dwrite_point_sel_read) {
            TESTING_2("H5Dwrite using point selection then H5Dread")

            for (i = 1, data_size = 1; i < DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK; i++)
                data_size *= dims[i];
            data_size *= DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE;

            if (NULL == (write_buf = HDmalloc(data_size))) {
                H5_FAILED();
                HDprintf("    couldn't allocate buffer for dataset write\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            /* Use different data than the previous test to ensure that the data actually changed. */
            for (i = 0; i < data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE; i++)
                ((int *) write_buf)[i] = mpi_size - mpi_rank;

            if (NULL == (points = HDmalloc(DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK * (data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE) * sizeof(hsize_t)))) {
                H5_FAILED();
                HDprintf("    couldn't allocate buffer for point selection\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            /* Each MPI rank writes to a single row in the second dimension
             * and the entirety of the following dimensions. The combined
             * selections from all MPI ranks spans the first dimension.
             */
            for (i = 0; i < data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE; i++) {
                int j;

                for (j = 0; j < DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK; j++) {
                    int idx = (i * DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK) + j;

                    if (j == 0)
                        points[idx] = mpi_rank;
                    else if (j != DATASET_WRITE_DATA_VERIFY_TEST_SPACE_RANK - 1)
                        points[idx] = i / dims[j + 1];
                    else
                        points[idx] = i % dims[j];
                }
            }

            if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3);
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if ((fspace_id = H5Dget_space(dset_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataset dataspace\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if (H5Sselect_elements(fspace_id, H5S_SELECT_SET, data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE, points) < 0) {
                H5_FAILED();
                HDprintf("    couldn't select elements in dataspace\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            {
                hsize_t mdims[] = { data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE };

                if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create memory dataspace\n");
                    PART_ERROR(H5Dwrite_point_sel_read);
                }
            }

            if (H5Dwrite(dset_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE, mspace_id, fspace_id, H5P_DEFAULT, write_buf) < 0) {
                H5_FAILED();
                HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3);
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if (write_buf) {
                HDfree(write_buf);
                write_buf = NULL;
            }
            if (mspace_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Sclose(mspace_id);
                } H5E_END_TRY;
                mspace_id = H5I_INVALID_HID;
            }
            if (fspace_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Sclose(fspace_id);
                } H5E_END_TRY;
                fspace_id = H5I_INVALID_HID;
            }
            if (dset_id >= 0) {
                H5E_BEGIN_TRY {
                    H5Dclose(dset_id);
                } H5E_END_TRY;
                dset_id = H5I_INVALID_HID;
            }

            /*
             * Close and re-open the file to ensure that the data gets written.
             */
            if (H5Gclose(group_id) < 0) {
                H5_FAILED();
                HDprintf("    failed to close test's container group\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }
            if (H5Gclose(container_group) < 0) {
                H5_FAILED();
                HDprintf("    failed to close container group\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }
            if (H5Fclose(file_id) < 0) {
                H5_FAILED();
                HDprintf("    failed to close file for data flushing\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }
            if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
                PART_ERROR(H5Dwrite_point_sel_read);
            }
            if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
                PART_ERROR(H5Dwrite_point_sel_read);
            }
            if ((group_id = H5Gopen2(container_group, DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_GROUP_NAME);
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3);
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if ((fspace_id = H5Dget_space(dset_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataset dataspace\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get dataspace num points\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE))) {
                H5_FAILED();
                HDprintf("    couldn't allocate buffer for dataset read\n");
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            if (H5Dread(dset_id, DATASET_WRITE_DATA_VERIFY_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
                H5_FAILED();
                HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_DATA_VERIFY_TEST_DSET_NAME3);
                PART_ERROR(H5Dwrite_point_sel_read);
            }

            for (i = 0; i < (size_t) mpi_size; i++) {
                size_t j;

                for (j = 0; j < data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE; j++) {
                    if (((int *) read_buf)[j + (i * (data_size / DATASET_WRITE_DATA_VERIFY_TEST_DTYPE_SIZE))] != (mpi_size - (int) i)) {
                        H5_FAILED();
                        HDprintf("    point selection data verification failed\n");
                        PART_ERROR(H5Dwrite_point_sel_read);
                    }
                }
            }

            if (read_buf) {
                HDfree(read_buf);
                read_buf = NULL;
            }

            PASSED();
        } PART_END(H5Dwrite_point_sel_read);

        if (write_buf) {
            HDfree(write_buf);
            write_buf = NULL;
        }
        if (read_buf) {
            HDfree(read_buf);
            read_buf = NULL;
        }
        if (points) {
            HDfree(points);
            points = NULL;
        }
        if (fspace_id >= 0) {
            H5E_BEGIN_TRY {
                H5Sclose(fspace_id);
            } H5E_END_TRY;
            fspace_id = H5I_INVALID_HID;
        }
        if (mspace_id >= 0) {
            H5E_BEGIN_TRY {
                H5Sclose(mspace_id);
            } H5E_END_TRY;
            mspace_id = H5I_INVALID_HID;
        }
        if (dset_id >= 0) {
            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;
            dset_id = H5I_INVALID_HID;
        }
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }

    if (points) {
        HDfree(points);
        points = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (read_buf) HDfree(read_buf);
        if (points) HDfree(points);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be written to by having
 * one of the MPI ranks select 0 rows in a hyperslab selection.
 */
#define DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK 2
#define DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_ONE_PROC_0_SEL_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_ONE_PROC_0_SEL_TEST_GROUP_NAME "one_rank_0_sel_write_test"
#define DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_NAME  "one_rank_0_sel_dset"
static int
test_write_dataset_one_proc_0_selection(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    hsize_t   start[DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK];
    hsize_t   stride[DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK];
    hsize_t   count[DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK];
    hsize_t   block[DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK];
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING("write to dataset with one rank selecting 0 rows")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_ONE_PROC_0_SEL_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_ONE_PROC_0_SEL_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_NAME, DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_NAME);
        goto error;
    }

    for (i = 1, data_size = 1; i < DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= DATASET_WRITE_ONE_PROC_0_SEL_TEST_DTYPE_SIZE;

    if (!MAINPROCESS) {
        if (NULL == (write_buf = HDmalloc(data_size))) {
            H5_FAILED();
            HDprintf("    couldn't allocate buffer for dataset write\n");
            goto error;
        }

        for (i = 0; i < data_size / DATASET_WRITE_ONE_PROC_0_SEL_TEST_DTYPE_SIZE; i++)
            ((int *) write_buf)[i] = mpi_rank;
    }

    for (i = 0; i < DATASET_WRITE_ONE_PROC_0_SEL_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = mpi_rank;
            block[i] = MAINPROCESS ? 0 : 1;
        }
        else {
            start[i] = 0;
            block[i] = MAINPROCESS ? 0 : dims[i];
        }

        stride[i] = 1;
        count[i] = MAINPROCESS ? 0 : 1;
    }

    if (H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    {
        hsize_t mdims[] = { data_size / DATASET_WRITE_ONE_PROC_0_SEL_TEST_DTYPE_SIZE };

        if (MAINPROCESS)
            mdims[0] = 0;

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    if (H5Dwrite(dset_id, DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_DTYPE, mspace_id, fspace_id, H5P_DEFAULT, write_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_NAME);
        goto error;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }
    if (mspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(mspace_id);
        } H5E_END_TRY;
        mspace_id = H5I_INVALID_HID;
    }
    if (fspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(fspace_id);
        } H5E_END_TRY;
        fspace_id = H5I_INVALID_HID;
    }
    if (dset_id >= 0) {
        H5E_BEGIN_TRY {
            H5Dclose(dset_id);
        } H5E_END_TRY;
        dset_id = H5I_INVALID_HID;
    }

    /*
     * Close and re-open the file to ensure that the data gets written.
     */
    if (H5Gclose(group_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close test's container group\n");
        goto error;
    }
    if (H5Gclose(container_group) < 0) {
        H5_FAILED();
        HDprintf("    failed to close container group\n");
        goto error;
    }
    if (H5Fclose(file_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close file for data flushing\n");
        goto error;
    }
    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }
    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }
    if ((group_id = H5Gopen2(container_group, DATASET_WRITE_ONE_PROC_0_SEL_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_ONE_PROC_0_SEL_TEST_GROUP_NAME);
        goto error;
    }

    if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_NAME);
        goto error;
    }

    if ((fspace_id = H5Dget_space(dset_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataset dataspace\n");
        goto error;
    }

    if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataspace num points\n");
        goto error;
    }

    if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_ONE_PROC_0_SEL_TEST_DTYPE_SIZE))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        goto error;
    }

    if (H5Dread(dset_id, DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_ONE_PROC_0_SEL_TEST_DSET_NAME);
        goto error;
    }

    for (i = 0; i < (size_t) mpi_size; i++) {
        size_t j;

        if (i != 0) {
            for (j = 0; j < data_size / DATASET_WRITE_ONE_PROC_0_SEL_TEST_DTYPE_SIZE; j++) {
                if (((int *) read_buf)[j + (i * (data_size / DATASET_WRITE_ONE_PROC_0_SEL_TEST_DTYPE_SIZE))] != (int) i) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    goto error;
                }
            }
        }
    }

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be written to by having
 * one of the MPI ranks call H5Sselect_none.
 */
#define DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK 2
#define DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_GROUP_NAME "one_rank_none_sel_write_test"
#define DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_NAME  "one_rank_none_sel_dset"
static int
test_write_dataset_one_proc_none_selection(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    hsize_t   start[DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK];
    hsize_t   stride[DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK];
    hsize_t   count[DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK];
    hsize_t   block[DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK];
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING("write to dataset with one rank using 'none' selection")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_NAME, DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_NAME);
        goto error;
    }

    for (i = 1, data_size = 1; i < DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DTYPE_SIZE;

    if (!MAINPROCESS) {
        if (NULL == (write_buf = HDmalloc(data_size))) {
            H5_FAILED();
            HDprintf("    couldn't allocate buffer for dataset write\n");
            goto error;
        }

        for (i = 0; i < data_size / DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DTYPE_SIZE; i++)
            ((int *) write_buf)[i] = mpi_rank;
    }

    for (i = 0; i < DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (MAINPROCESS) {
        if (H5Sselect_none(fspace_id) < 0) {
            H5_FAILED();
            HDprintf("    couldn't set 'none' selection for dataset write\n");
            goto error;
        }
    }
    else {
        if (H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
            H5_FAILED();
            HDprintf("    couldn't select hyperslab for dataset write\n");
            goto error;
        }
    }

    {
        hsize_t mdims[] = { data_size / DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DTYPE_SIZE };

        if (MAINPROCESS)
            mdims[0] = 0;

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    if (H5Dwrite(dset_id, DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_DTYPE, mspace_id, fspace_id, H5P_DEFAULT, write_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_NAME);
        goto error;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }
    if (mspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(mspace_id);
        } H5E_END_TRY;
        mspace_id = H5I_INVALID_HID;
    }
    if (fspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(fspace_id);
        } H5E_END_TRY;
        fspace_id = H5I_INVALID_HID;
    }
    if (dset_id >= 0) {
        H5E_BEGIN_TRY {
            H5Dclose(dset_id);
        } H5E_END_TRY;
        dset_id = H5I_INVALID_HID;
    }

    /*
     * Close and re-open the file to ensure that the data gets written.
     */
    if (H5Gclose(group_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close test's container group\n");
        goto error;
    }
    if (H5Gclose(container_group) < 0) {
        H5_FAILED();
        HDprintf("    failed to close container group\n");
        goto error;
    }
    if (H5Fclose(file_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close file for data flushing\n");
        goto error;
    }
    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }
    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }
    if ((group_id = H5Gopen2(container_group, DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_GROUP_NAME);
        goto error;
    }

    if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_NAME);
        goto error;
    }

    if ((fspace_id = H5Dget_space(dset_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataset dataspace\n");
        goto error;
    }

    if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataspace num points\n");
        goto error;
    }

    if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DTYPE_SIZE))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        goto error;
    }

    if (H5Dread(dset_id, DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DSET_NAME);
        goto error;
    }

    for (i = 0; i < (size_t) mpi_size; i++) {
        size_t j;

        if (i != 0) {
            for (j = 0; j < data_size / DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DTYPE_SIZE; j++) {
                if (((int *) read_buf)[j + (i * (data_size / DATASET_WRITE_ONE_PROC_NONE_SEL_TEST_DTYPE_SIZE))] != (int) i) {
                    H5_FAILED();
                    HDprintf("    data verification failed\n");
                    goto error;
                }
            }
        }
    }

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be written to by having
 * one of the MPI ranks use an ALL selection, while the other
 * ranks write nothing.
 */
#define DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_SPACE_RANK 2
#define DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_GROUP_NAME "one_rank_all_sel_write_test"
#define DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_NAME  "one_rank_all_sel_dset"
static int
test_write_dataset_one_proc_all_selection(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING("write to dataset with one rank using all selection; others none selection")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_NAME, DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_NAME);
        goto error;
    }

    for (i = 0, data_size = 1; i < DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DTYPE_SIZE;

    if (MAINPROCESS) {
        if (NULL == (write_buf = HDmalloc(data_size))) {
            H5_FAILED();
            HDprintf("    couldn't allocate buffer for dataset write\n");
            goto error;
        }

        for (i = 0; i < data_size / DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DTYPE_SIZE; i++)
            ((int *) write_buf)[i] = i;
    }

    if (MAINPROCESS) {
        if (H5Sselect_all(fspace_id) < 0) {
            H5_FAILED();
            HDprintf("    couldn't set 'all' selection for dataset write\n");
            goto error;
        }
    }
    else {
        if (H5Sselect_none(fspace_id) < 0) {
            H5_FAILED();
            HDprintf("    couldn't set 'none' selection for dataset write\n");
            goto error;
        }
    }

    {
        hsize_t mdims[] = { data_size / DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DTYPE_SIZE };

        if (!MAINPROCESS)
            mdims[0] = 0;

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    if (H5Dwrite(dset_id, DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_DTYPE, mspace_id, fspace_id, H5P_DEFAULT, write_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_NAME);
        goto error;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }
    if (mspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(mspace_id);
        } H5E_END_TRY;
        mspace_id = H5I_INVALID_HID;
    }
    if (fspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(fspace_id);
        } H5E_END_TRY;
        fspace_id = H5I_INVALID_HID;
    }
    if (dset_id >= 0) {
        H5E_BEGIN_TRY {
            H5Dclose(dset_id);
        } H5E_END_TRY;
        dset_id = H5I_INVALID_HID;
    }

    /*
     * Close and re-open the file to ensure that the data gets written.
     */
    if (H5Gclose(group_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close test's container group\n");
        goto error;
    }
    if (H5Gclose(container_group) < 0) {
        H5_FAILED();
        HDprintf("    failed to close container group\n");
        goto error;
    }
    if (H5Fclose(file_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close file for data flushing\n");
        goto error;
    }
    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }
    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }
    if ((group_id = H5Gopen2(container_group, DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_GROUP_NAME);
        goto error;
    }

    if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_NAME);
        goto error;
    }

    if ((fspace_id = H5Dget_space(dset_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataset dataspace\n");
        goto error;
    }

    if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataspace num points\n");
        goto error;
    }

    if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DTYPE_SIZE))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        goto error;
    }

    if (H5Dread(dset_id, DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DSET_NAME);
        goto error;
    }

    for (i = 0; i < data_size / DATASET_WRITE_ONE_PROC_ALL_SEL_TEST_DTYPE_SIZE; i++) {
        if (((int *) read_buf)[i] != (int) i) {
            H5_FAILED();
            HDprintf("    data verification failed\n");
            goto error;
        }
    }

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be written to by having
 * a hyperslab selection in the file dataspace and an all selection
 * in the memory dataspace.
 */
#define DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK 2
#define DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_GROUP_NAME "hyper_sel_file_all_sel_mem_write_test"
#define DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_NAME  "hyper_sel_file_all_sel_mem_dset"
static int
test_write_dataset_hyper_file_all_mem(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    hsize_t   start[DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK];
    hsize_t   stride[DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK];
    hsize_t   count[DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK];
    hsize_t   block[DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK];
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING("write to dataset with hyperslab sel. for file space; all sel. for memory")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_NAME, DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_NAME);
        goto error;
    }

    for (i = 1, data_size = 1; i < DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DTYPE_SIZE;

    if (NULL == (write_buf = HDmalloc(data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        goto error;
    }

    for (i = 0; i < data_size / DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DTYPE_SIZE; i++)
        ((int *) write_buf)[i] = mpi_rank;

    for (i = 0; i < DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    if (H5Dwrite(dset_id, DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_DTYPE, H5S_ALL, fspace_id, H5P_DEFAULT, write_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_NAME);
        goto error;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }
    if (fspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(fspace_id);
        } H5E_END_TRY;
        fspace_id = H5I_INVALID_HID;
    }
    if (dset_id >= 0) {
        H5E_BEGIN_TRY {
            H5Dclose(dset_id);
        } H5E_END_TRY;
        dset_id = H5I_INVALID_HID;
    }

    /*
     * Close and re-open the file to ensure that the data gets written.
     */
    if (H5Gclose(group_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close test's container group\n");
        goto error;
    }
    if (H5Gclose(container_group) < 0) {
        H5_FAILED();
        HDprintf("    failed to close container group\n");
        goto error;
    }
    if (H5Fclose(file_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close file for data flushing\n");
        goto error;
    }
    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }
    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }
    if ((group_id = H5Gopen2(container_group, DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_GROUP_NAME);
        goto error;
    }

    if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_NAME);
        goto error;
    }

    if ((fspace_id = H5Dget_space(dset_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataset dataspace\n");
        goto error;
    }

    if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataspace num points\n");
        goto error;
    }

    if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DTYPE_SIZE))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        goto error;
    }

    if (H5Dread(dset_id, DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DSET_NAME);
        goto error;
    }

    for (i = 0; i < data_size / DATASET_WRITE_HYPER_FILE_ALL_MEM_TEST_DTYPE_SIZE; i++) {
        if (((int *) read_buf)[i] != (int) i) {
            H5_FAILED();
            HDprintf("    data verification failed\n");
            goto error;
        }
    }

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be written to by having
 * an all selection in the file dataspace and a hyperslab
 * selection in the memory dataspace.
 */
#define DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK 2
#define DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_GROUP_NAME "all_sel_file_hyper_sel_mem_write_test"
#define DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DSET_NAME  "all_sel_file_hyper_sel_mem_dset"
static int
test_write_dataset_all_file_hyper_mem(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    hsize_t   start[DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK];
    hsize_t   stride[DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK];
    hsize_t   count[DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK];
    hsize_t   block[DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK];
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING("write to dataset with all sel. for file space; hyperslab sel. for memory")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DSET_NAME, DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DSET_NAME);
        goto error;
    }

    for (i = 1, data_size = 1; i < DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DTYPE_SIZE;

    if (MAINPROCESS) {
        if (NULL == (write_buf = HDmalloc(data_size))) {
            H5_FAILED();
            HDprintf("    couldn't allocate buffer for dataset write\n");
            goto error;
        }

        for (i = 0; i < data_size / DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DTYPE_SIZE; i++)
            ((int *) write_buf)[i] = mpi_rank;
    }

    {
        hsize_t mdims[] = { data_size / DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DTYPE_SIZE };

        if (!MAINPROCESS)
            mdims[0] = 0;

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }
    }

    for (i = 0; i < DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    if (H5Dwrite(dset_id, DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DSET_DTYPE, mspace_id, H5S_ALL, H5P_DEFAULT, write_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_ALL_FILE_HYPER_MEM_TEST_DSET_NAME);
        goto error;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }
    if (fspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(fspace_id);
        } H5E_END_TRY;
        fspace_id = H5I_INVALID_HID;
    }
    if (dset_id >= 0) {
        H5E_BEGIN_TRY {
            H5Dclose(dset_id);
        } H5E_END_TRY;
        dset_id = H5I_INVALID_HID;
    }




    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be written to by having
 * a point selection in the file dataspace and an all selection
 * in the memory dataspace.
 */
static int
test_write_dataset_point_file_all_mem(void)
{

}

/*
 * A test to ensure that a dataset can be written to by having
 * an all selection in the file dataspace and a point selection
 * in the memory dataspace.
 */
static int
test_write_dataset_all_file_point_mem(void)
{

}

/*
 * A test to ensure that a dataset can be written to by having
 * a hyperslab selection in the file dataspace and a point
 * selection in the memory dataspace.
 */
#define DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK 2
#define DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_GROUP_NAME "hyper_sel_file_point_sel_mem_write_test"
#define DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_NAME  "hyper_sel_file_point_sel_mem_dset"
static int
test_write_dataset_hyper_file_point_mem(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    hsize_t  *points = NULL;
    hsize_t   start[DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK];
    hsize_t   stride[DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK];
    hsize_t   count[DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK];
    hsize_t   block[DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK];
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING("write to dataset with hyperslab sel. for file space; point sel. for memory")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_NAME, DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_NAME);
        goto error;
    }

    for (i = 1, data_size = 1; i < DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE;

    /*
     * Allocate twice the amount of memory needed and leave "holes" in the memory
     * buffer in order to prove that the mapping from point selection <-> hyperslab
     * selection works correctly.
     */
    if (NULL == (write_buf = HDmalloc(2 * data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        goto error;
    }

    for (i = 0; i < 2 * (data_size / DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE); i++) {
        /* Write actual data to even indices */
        if (i % 2 == 0)
            ((int *) write_buf)[i] = mpi_rank;
        else
            ((int *) write_buf)[i] = 0;
    }

    for (i = 0; i < DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_SPACE_RANK; i++) {
        if (i == 0) {
            start[i] = mpi_rank;
            block[i] = 1;
        }
        else {
            start[i] = 0;
            block[i] = dims[i];
        }

        stride[i] = 1;
        count[i] = 1;
    }

    if (H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
        H5_FAILED();
        HDprintf("    couldn't select hyperslab for dataset write\n");
        goto error;
    }

    {
        hsize_t mdims[] = { 2 * (data_size / DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE) };
        int j;

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }

        if (NULL == (points = HDmalloc((data_size / DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE) * sizeof(hsize_t)))) {
            H5_FAILED();
            HDprintf("    couldn't allocate buffer for point selection\n");
            goto error;
        }

        /* Select every other point in the 1-dimensional memory dataspace */
        for (i = 0, j = 0; i < 2 * (data_size / DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE); i++) {
            if (i % 2 == 0)
                points[j++] = (hsize_t) i;
        }

        if (H5Sselect_elements(mspace_id, H5S_SELECT_SET, data_size / DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE, points) < 0) {
            H5_FAILED();
            HDprintf("    couldn't set point selection for dataset write\n");
            goto error;
        }
    }

    if (H5Dwrite(dset_id, DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_DTYPE, mspace_id, fspace_id, H5P_DEFAULT, write_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_NAME);
        goto error;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }
    if (points) {
        HDfree(points);
        points = NULL;
    }
    if (mspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(mspace_id);
        } H5E_END_TRY;
        mspace_id = H5I_INVALID_HID;
    }
    if (fspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(fspace_id);
        } H5E_END_TRY;
        fspace_id = H5I_INVALID_HID;
    }
    if (dset_id >= 0) {
        H5E_BEGIN_TRY {
            H5Dclose(dset_id);
        } H5E_END_TRY;
        dset_id = H5I_INVALID_HID;
    }

    /*
     * Close and re-open the file to ensure that the data gets written.
     */
    if (H5Gclose(group_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close test's container group\n");
        goto error;
    }
    if (H5Gclose(container_group) < 0) {
        H5_FAILED();
        HDprintf("    failed to close container group\n");
        goto error;
    }
    if (H5Fclose(file_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close file for data flushing\n");
        goto error;
    }
    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }
    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }
    if ((group_id = H5Gopen2(container_group, DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_GROUP_NAME);
        goto error;
    }

    if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_NAME);
        goto error;
    }

    if ((fspace_id = H5Dget_space(dset_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataset dataspace\n");
        goto error;
    }

    if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataspace num points\n");
        goto error;
    }

    if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        goto error;
    }

    if (H5Dread(dset_id, DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DSET_NAME);
        goto error;
    }

    for (i = 0; i < (size_t) mpi_size; i++) {
        size_t j;

        for (j = 0; j < data_size / DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE; j++) {
            if (((int *) read_buf)[j + (i * (data_size / DATASET_WRITE_HYPER_FILE_POINT_MEM_TEST_DTYPE_SIZE))] != (int) i) {
                H5_FAILED();
                HDprintf("    data verification failed\n");
                goto error;
            }
        }
    }

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (points) HDfree(points);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be written to by having
 * a point selection in the file dataspace and a hyperslab
 * selection in the memory dataspace.
 */
#define DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK 2
#define DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_DTYPE H5T_NATIVE_INT
#define DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE sizeof(int)
#define DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_GROUP_NAME "point_sel_file_hyper_sel_mem_write_test"
#define DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_NAME  "point_sel_file_hyper_sel_mem_dset"
static int
test_write_dataset_point_file_hyper_mem(void)
{
    hssize_t  space_npoints;
    hsize_t  *dims = NULL;
    hsize_t  *points = NULL;
    size_t    i, data_size;
    hid_t     file_id = H5I_INVALID_HID;
    hid_t     fapl_id = H5I_INVALID_HID;
    hid_t     container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t     dset_id = H5I_INVALID_HID;
    hid_t     fspace_id = H5I_INVALID_HID;
    hid_t     mspace_id = H5I_INVALID_HID;
    void     *write_buf = NULL;
    void     *read_buf = NULL;

    TESTING("write to dataset with point sel. for file space; hyperslab sel. for memory")

    if ((fapl_id = create_mpio_fapl(MPI_COMM_WORLD, MPI_INFO_NULL)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_GROUP_NAME);
        goto error;
    }

    if (NULL == (dims = HDmalloc(DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK * sizeof(hsize_t))))
        TEST_ERROR
    for (i = 0; i < DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK; i++) {
        if (i == 0)
            dims[i] = mpi_size;
        else
            dims[i] = (rand() % MAX_DIM_SIZE) + 1;
    }

    if ((fspace_id = H5Screate_simple(DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK, dims, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_NAME, DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_DTYPE,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_NAME);
        goto error;
    }

    for (i = 1, data_size = 1; i < DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK; i++)
        data_size *= dims[i];
    data_size *= DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE;

    /*
     * Allocate twice the amount of memory needed and leave "holes" in the memory
     * buffer in order to prove that the mapping from hyperslab selection <-> point
     * selection works correctly.
     */
    if (NULL == (write_buf = HDmalloc(2 * data_size))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset write\n");
        goto error;
    }

    for (i = 0; i < 2 * (data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE); i++) {
        /* Write actual data to even indices */
        if (i % 2 == 0)
            ((int *) write_buf)[i] = mpi_rank;
        else
            ((int *) write_buf)[i] = 0;
    }

    if (NULL == (points = HDmalloc((data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE) * DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK * sizeof(hsize_t)))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for point selection\n");
        goto error;
    }

    for (i = 0; i < data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE; i++) {
        int j;

        for (j = 0; j < DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK; j++) {
            int idx = (i * DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK) + j;

            if (j == 0)
                points[idx] = mpi_rank;
            else if (j != DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_SPACE_RANK - 1)
                points[idx] = i / dims[j + 1];
            else
                points[idx] = i % dims[j];
        }
    }

    if (H5Sselect_elements(fspace_id, H5S_SELECT_SET, data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE, points) < 0) {
        H5_FAILED();
        HDprintf("    couldn't set point selection for dataset write\n");
        goto error;
    }

    {
        hsize_t start[1] = { 0 };
        hsize_t stride[1] = { 2 };
        hsize_t count[1] = { data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE };
        hsize_t block[1] = { 1 };
        hsize_t mdims[] = { 2 * (data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE) };

        if ((mspace_id = H5Screate_simple(1, mdims, NULL)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't create memory dataspace\n");
            goto error;
        }

        if (H5Sselect_hyperslab(mspace_id, H5S_SELECT_SET, start, stride, count, block) < 0) {
            H5_FAILED();
            HDprintf("    couldn't set hyperslab selection for dataset write\n");
            goto error;
        }
    }

    if (H5Dwrite(dset_id, DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_DTYPE, mspace_id, fspace_id, H5P_DEFAULT, write_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset '%s'\n", DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_NAME);
        goto error;
    }

    if (write_buf) {
        HDfree(write_buf);
        write_buf = NULL;
    }
    if (points) {
        HDfree(points);
        points = NULL;
    }
    if (mspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(mspace_id);
        } H5E_END_TRY;
        mspace_id = H5I_INVALID_HID;
    }
    if (fspace_id >= 0) {
        H5E_BEGIN_TRY {
            H5Sclose(fspace_id);
        } H5E_END_TRY;
        fspace_id = H5I_INVALID_HID;
    }
    if (dset_id >= 0) {
        H5E_BEGIN_TRY {
            H5Dclose(dset_id);
        } H5E_END_TRY;
        dset_id = H5I_INVALID_HID;
    }

    /*
     * Close and re-open the file to ensure that the data gets written.
     */
    if (H5Gclose(group_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close test's container group\n");
        goto error;
    }
    if (H5Gclose(container_group) < 0) {
        H5_FAILED();
        HDprintf("    failed to close container group\n");
        goto error;
    }
    if (H5Fclose(file_id) < 0) {
        H5_FAILED();
        HDprintf("    failed to close file for data flushing\n");
        goto error;
    }
    if ((file_id = H5Fopen(vol_test_parallel_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't re-open file '%s'\n", vol_test_parallel_filename);
        goto error;
    }
    if ((container_group = H5Gopen2(file_id, DATASET_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", DATASET_TEST_GROUP_NAME);
        goto error;
    }
    if ((group_id = H5Gopen2(container_group, DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container sub-group '%s'\n", DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_GROUP_NAME);
        goto error;
    }

    if ((dset_id = H5Dopen2(group_id, DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open dataset '%s'\n", DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_NAME);
        goto error;
    }

    if ((fspace_id = H5Dget_space(dset_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataset dataspace\n");
        goto error;
    }

    if ((space_npoints = H5Sget_simple_extent_npoints(fspace_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get dataspace num points\n");
        goto error;
    }

    if (NULL == (read_buf = HDmalloc((hsize_t) space_npoints * DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE))) {
        H5_FAILED();
        HDprintf("    couldn't allocate buffer for dataset read\n");
        goto error;
    }

    if (H5Dread(dset_id, DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_DTYPE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_buf) < 0) {
        H5_FAILED();
        HDprintf("    couldn't read from dataset '%s'\n", DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DSET_NAME);
        goto error;
    }

    for (i = 0; i < (size_t) mpi_size; i++) {
        size_t j;

        for (j = 0; j < data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE; j++) {
            if (((int *) read_buf)[j + (i * (data_size / DATASET_WRITE_POINT_FILE_HYPER_MEM_TEST_DTYPE_SIZE))] != (int) i) {
                H5_FAILED();
                HDprintf("    data verification failed\n");
                goto error;
            }
        }
    }

    if (read_buf) {
        HDfree(read_buf);
        read_buf = NULL;
    }

    if (dims) {
        HDfree(dims);
        dims = NULL;
    }

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        if (read_buf) HDfree(read_buf);
        if (write_buf) HDfree(write_buf);
        if (points) HDfree(points);
        if (dims) HDfree(dims);
        H5Sclose(mspace_id);
        H5Sclose(fspace_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to ensure that a dataset can be read from by having
 * one of the MPI ranks select 0 rows in a hyperslab selection.
 */
static int
test_read_dataset_one_proc_0_selection(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * one of the MPI ranks call H5Sselect_none.
 */
static int
test_read_dataset_one_proc_none_selection(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * one of the MPI ranks use an ALL selection, while the other
 * ranks write nothing.
 */
static int
test_read_dataset_one_proc_all_selection(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * a hyperslab selection in the file dataspace and an all
 * selection in the memory dataspace.
 */
static int
test_read_dataset_hyper_file_all_mem(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * an all selection in the file dataspace and a hyperslab
 * selection in the memory dataspace.
 */
static int
test_read_dataset_all_file_hyper_mem(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * a point selection in the file dataspace and an all selection
 * in the memory dataspace.
 */
static int
test_read_dataset_point_file_all_mem(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * an all selection in the file dataspace and a point selection
 * in the memory dataspace.
 */
static int
test_read_dataset_all_file_point_mem(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * a hyperslab selection in the file dataspace and a point
 * selection in the memory dataspace.
 */
static int
test_read_dataset_hyper_file_point_mem(void)
{

}

/*
 * A test to ensure that a dataset can be read from by having
 * a point selection in the file dataspace and a hyperslab
 * selection in the memory dataspace.
 */
static int
test_read_dataset_point_file_hyper_mem(void)
{

}

int
vol_dataset_test_parallel(void)
{
    size_t i;
    int    nerrors;

    if (MAINPROCESS) {
        HDprintf("**********************************************\n");
        HDprintf("*                                            *\n");
        HDprintf("*         VOL Parallel Dataset Tests         *\n");
        HDprintf("*                                            *\n");
        HDprintf("**********************************************\n\n");
    }

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(par_dataset_tests); i++) {
        nerrors += (*par_dataset_tests[i])() ? 1 : 0;

        if (MPI_SUCCESS != (MPI_Barrier(MPI_COMM_WORLD))) {
            if (MAINPROCESS)
                HDprintf("    MPI_Barrier() failed!\n");
        }
    }

    if (MAINPROCESS)
        HDprintf("\n");

    return nerrors;
}
