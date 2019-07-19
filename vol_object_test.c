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

#include "vol_object_test.h"

/*
 * XXX: Implement tests for H5Olink. Pull tests from other interface tests for H5Xcreate_anon.
 */

/*
 * XXX: Difficult to implement right now.
 */
#define NO_REF_TESTS

static int test_open_object(void);
static int test_open_object_invalid_params(void);
static int test_object_exists(void);
static int test_object_exists_invalid_params(void);
static int test_get_object_info(void);
static int test_get_object_info_invalid_params(void);
static int test_link_object(void);
static int test_link_object_invalid_params(void);
static int test_incr_decr_object_refcount(void);
static int test_incr_decr_object_refcount_invalid_params(void);
static int test_copy_object(void);
static int test_copy_object_invalid_params(void);
static int test_object_comments(void);
static int test_object_comments_invalid_params(void);
static int test_object_visit(void);
static int test_object_visit_invalid_params(void);
static int test_close_object(void);
static int test_close_object_invalid_params(void);
static int test_close_invalid_objects(void);
static int test_flush_object(void);
static int test_flush_object_invalid_params(void);
static int test_refresh_object(void);
static int test_refresh_object_invalid_params(void);

#ifndef NO_REF_TESTS
static int test_create_obj_ref(void);
static int test_dereference_reference(void);
static int test_get_ref_type(void);
static int test_get_ref_name(void);
static int test_get_region(void);
static int test_write_dataset_w_obj_refs(void);
static int test_read_dataset_w_obj_refs(void);
static int test_write_dataset_w_obj_refs_empty_data(void);
#endif

static herr_t object_visit_callback(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data);
static herr_t object_visit_callback2(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data);

/*
 * The array of object tests to be performed.
 */
static int (*object_tests[])(void) = {
        test_open_object,
        test_open_object_invalid_params,
        test_object_exists,
        test_object_exists_invalid_params,
        test_get_object_info,
        test_get_object_info_invalid_params,
        test_link_object,
        test_link_object_invalid_params,
        test_incr_decr_object_refcount,
        test_incr_decr_object_refcount_invalid_params,
        test_copy_object,
        test_copy_object_invalid_params,
        test_object_comments,
        test_object_comments_invalid_params,
        test_object_visit,
        test_object_visit_invalid_params,
        test_close_object,
        test_close_object_invalid_params,
        test_close_invalid_objects,
        test_flush_object,
        test_flush_object_invalid_params,
        test_refresh_object,
        test_refresh_object_invalid_params,
#ifndef NO_REF_TESTS
        test_create_obj_ref,
        test_dereference_reference,
        test_get_ref_type,
        test_get_ref_name,
        test_get_region,
        test_write_dataset_w_obj_refs,
        test_read_dataset_w_obj_refs,
        test_write_dataset_w_obj_refs_empty_data,
#endif
};

/*
 * A test to check that various objects (group, dataset, datatype)
 * can be opened by using H5Oopen, H5Oopen_by_idx and H5Oopen_by_addr.
 *
 * XXX: create separate objects for each test part.
 */
static int
test_open_object(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t group_id2 = H5I_INVALID_HID;
    hid_t dset_id = H5I_INVALID_HID;
    hid_t dset_dtype = H5I_INVALID_HID;
    hid_t type_id = H5I_INVALID_HID;
    hid_t fspace_id = H5I_INVALID_HID;

    TESTING_MULTIPART("object opening");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_OPEN_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", OBJECT_OPEN_TEST_GROUP_NAME);
        goto error;
    }

    if ((fspace_id = generate_random_dataspace(OBJECT_OPEN_TEST_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Oopen_group) {
            TESTING_2("H5Oopen on a group")

            if ((group_id2 = H5Gcreate2(group_id, OBJECT_OPEN_TEST_GRP_NAME,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create group '%s'\n", OBJECT_OPEN_TEST_GRP_NAME);
                PART_ERROR(H5Oopen_group);
            }

            H5E_BEGIN_TRY {
                H5Gclose(group_id2);
            } H5E_END_TRY;

            if ((group_id2 = H5Oopen(group_id, OBJECT_OPEN_TEST_GRP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open group '%s' with H5Oopen\n", OBJECT_OPEN_TEST_GRP_NAME);
                PART_ERROR(H5Oopen_group);
            }

            if (H5Iget_type(group_id2) != H5I_GROUP) {
	        H5_FAILED();
	        HDprintf("    ID is not a group\n");
                PART_ERROR(H5Oopen_group);
            }

            if (H5Gclose(group_id2) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close group opened with H5Oopen\n");
                PART_ERROR(H5Oopen_group);
            }

            PASSED();
        } PART_END(H5Oopen_group);

        PART_BEGIN(H5Oopen_dset) {
            TESTING_2("H5Oopen on a dataset")

            if ((dset_id = H5Dcreate2(group_id, OBJECT_OPEN_TEST_DSET_NAME, dset_dtype,
                    fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create dataset '%s'\n", OBJECT_OPEN_TEST_DSET_NAME);
                PART_ERROR(H5Oopen_dset);
            }

            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;

            if ((dset_id = H5Oopen(group_id, OBJECT_OPEN_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s' with H5Oopen\n", OBJECT_OPEN_TEST_DSET_NAME);
                PART_ERROR(H5Oopen_dset);
            }

            if (H5Iget_type(dset_id) != H5I_DATASET) {
	        H5_FAILED();
	        HDprintf("    ID is not a dataset\n");
                PART_ERROR(H5Oopen_dset);
            }

            if (H5Dclose(dset_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close dataset opened with H5Oopen\n");
                PART_ERROR(H5Oopen_dset);
            }

            PASSED();
        } PART_END(H5Oopen_dset);

        PART_BEGIN(H5Oopen_dtype) {
            TESTING_2("H5Oopen on a committed datatype")

            if ((type_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create datatype '%s'\n", OBJECT_OPEN_TEST_TYPE_NAME);
                PART_ERROR(H5Oopen_dtype);
            }

            if (H5Tcommit2(group_id, OBJECT_OPEN_TEST_TYPE_NAME, type_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't commit datatype '%s'\n", OBJECT_OPEN_TEST_TYPE_NAME);
                PART_ERROR(H5Oopen_dtype);
            }

            H5E_BEGIN_TRY {
                H5Tclose(type_id);
            } H5E_END_TRY;

            if ((type_id = H5Oopen(group_id, OBJECT_OPEN_TEST_TYPE_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open datatype '%s' with H5Oopen\n", OBJECT_OPEN_TEST_TYPE_NAME);
                PART_ERROR(H5Oopen_dtype);
            }

            if (H5Iget_type(type_id) != H5I_DATATYPE) {
	        H5_FAILED();
	        HDprintf("    ID is not a dataset\n");
                PART_ERROR(H5Oopen_dtype);
            }

            if (H5Tclose(type_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close committed datatype opened with H5Oopen\n");
                PART_ERROR(H5Oopen_dtype);
            }

            PASSED();
        } PART_END(H5Oopen_dtype);

        if (group_id2 >= 0) {
            H5E_BEGIN_TRY {
                H5Gclose(group_id2);
            } H5E_END_TRY;
            group_id = H5I_INVALID_HID;
        }
        if (dset_id >= 0) {
            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;
            dset_id = H5I_INVALID_HID;
        }
        if (type_id >= 0) {
            H5E_BEGIN_TRY {
                H5Tclose(type_id);
            } H5E_END_TRY;
            type_id = H5I_INVALID_HID;
        }

        PART_BEGIN(H5Oopen_by_idx_group) {
            TESTING_2("H5Oopen_by_idx on a group")

            if ((group_id2 = H5Oopen_by_idx(container_group, OBJECT_OPEN_TEST_GROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, 1, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open group '%s' with H5Oopen_by_idx\n", OBJECT_OPEN_TEST_GRP_NAME);
                PART_ERROR(H5Oopen_by_idx_group);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_group);

        PART_BEGIN(H5Oopen_by_idx_dset) {
            TESTING_2("H5Oopen_by_idx on a dataset")

            if ((dset_id = H5Oopen_by_idx(container_group, OBJECT_OPEN_TEST_GROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, 0, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s' with H5Oopen_by_idx\n", OBJECT_OPEN_TEST_DSET_NAME);
                PART_ERROR(H5Oopen_by_idx_dset);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_dset);

        PART_BEGIN(H5Oopen_by_idx_dtype) {
            TESTING_2("H5Oopen_by_idx on a committed datatype")

            if ((type_id = H5Oopen_by_idx(container_group, OBJECT_OPEN_TEST_GROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, 2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open committed datatype '%s' with H5Oopen_by_idx\n", OBJECT_OPEN_TEST_TYPE_NAME);
                PART_ERROR(H5Oopen_by_idx_dtype);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_dtype);

        if (group_id2 >= 0) {
            H5E_BEGIN_TRY {
                H5Gclose(group_id2);
            } H5E_END_TRY;
            group_id = H5I_INVALID_HID;
        }
        if (dset_id >= 0) {
            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;
            dset_id = H5I_INVALID_HID;
        }
        if (type_id >= 0) {
            H5E_BEGIN_TRY {
                H5Tclose(type_id);
            } H5E_END_TRY;
            type_id = H5I_INVALID_HID;
        }

        PART_BEGIN(H5Oopen_by_addr_group) {
            TESTING_2("H5Oopen_by_addr on a group")

            if ((group_id2 = H5Oopen_by_addr(file_id, 0)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open group '%s' with H5Oopen_by_addr\n", OBJECT_OPEN_TEST_GRP_NAME);
                PART_ERROR(H5Oopen_by_addr_group);
            }

            PASSED();
        } PART_END(H5Oopen_by_addr_group);

        PART_BEGIN(H5Oopen_by_addr_dset) {
            TESTING_2("H5Oopen_by_addr on a dataset")

            if ((dset_id = H5Oopen_by_addr(file_id, 0)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s' with H5Oopen_by_addr\n", OBJECT_OPEN_TEST_DSET_NAME);
                PART_ERROR(H5Oopen_by_addr_dset);
            }

            PASSED();
        } PART_END(H5Oopen_by_addr_dset);

        PART_BEGIN(H5Oopen_by_addr_dtype) {
            TESTING_2("H5Oopen_by_addr on a committed datatype")

            if ((type_id = H5Oopen_by_addr(file_id, 0)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open committed datatype '%s' with H5Oopen_by_addr\n", OBJECT_OPEN_TEST_TYPE_NAME);
                PART_ERROR(H5Oopen_by_addr_dtype);
            }

            PASSED();
        } PART_END(H5Oopen_by_addr_dtype);

        if (group_id2 >= 0) {
            H5E_BEGIN_TRY {
                H5Gclose(group_id2);
            } H5E_END_TRY;
            group_id = H5I_INVALID_HID;
        }
        if (dset_id >= 0) {
            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;
            dset_id = H5I_INVALID_HID;
        }
        if (type_id >= 0) {
            H5E_BEGIN_TRY {
                H5Tclose(type_id);
            } H5E_END_TRY;
            type_id = H5I_INVALID_HID;
        }
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(fspace_id);
        H5Tclose(dset_dtype);
        H5Tclose(type_id);
        H5Dclose(dset_id);
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that various objects (group, dataset, datatype)
 * can't be opened when H5Oopen, H5Oopen_by_idx and H5Oopen_by_addr
 * are passed invalid parameters.
 */
static int
test_open_object_invalid_params(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t group_id2 = H5I_INVALID_HID;

    TESTING_MULTIPART("object opening with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id2 = H5Gcreate2(group_id, OBJECT_OPEN_INVALID_PARAMS_TEST_GRP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", OBJECT_OPEN_INVALID_PARAMS_TEST_GRP_NAME);
        goto error;
    }

    if (H5Gclose(group_id2) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Oopen_invalid_loc_id) {
            TESTING_2("H5Oopen with an invalid location ID")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen(H5I_INVALID_HID, OBJECT_OPEN_INVALID_PARAMS_TEST_GRP_NAME, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen succeeded with an invalid location ID!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Oopen_invalid_loc_id);

        PART_BEGIN(H5Oopen_invalid_obj_name) {
            TESTING_2("H5Oopen with an invalid object name")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen(group_id, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen succeeded with a NULL object name!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_invalid_obj_name);
            }

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen(group_id, "", H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen succeeded with an invalid object name of ''!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_invalid_obj_name);
            }

            PASSED();
        } PART_END(H5Oopen_invalid_obj_name);

        PART_BEGIN(H5Oopen_invalid_lapl) {
            TESTING_2("H5Oopen with an invalid LAPL")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen(group_id, OBJECT_OPEN_INVALID_PARAMS_TEST_GRP_NAME, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen succeeded with an invalid LAPL!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Oopen_invalid_lapl);

        PART_BEGIN(H5Oopen_by_idx_invalid_loc_id) {
            TESTING_2("H5Oopen_by_idx with an invalid location ID")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(H5I_INVALID_HID, OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with an invalid location ID!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_invalid_loc_id);

        PART_BEGIN(H5Oopen_by_idx_invalid_grp_name) {
            TESTING_2("H5Oopen_by_idx with an invalid group name")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(container_group, NULL, H5_INDEX_NAME, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with a NULL group name!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_grp_name);
            }

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(container_group, "", H5_INDEX_NAME, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with an invalid group name of ''!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_grp_name);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_invalid_grp_name);

        PART_BEGIN(H5Oopen_by_idx_invalid_index_type) {
            TESTING_2("H5Oopen_by_idx with an invalid index type")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(container_group, OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME,
                        H5_INDEX_UNKNOWN, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(container_group, OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME,
                        H5_INDEX_N, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with invalid index type H5_INDEX_N!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_invalid_index_type);

        PART_BEGIN(H5Oopen_by_idx_invalid_iter_order) {
            TESTING_2("H5Oopen_by_idx with an invalid iteration order")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(container_group, OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_UNKNOWN, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with an invalid iteration ordering H5_ITER_UNKNOWN!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(container_group, OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_N, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with an invalid iteration ordering H5_ITER_N!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_invalid_iter_order);

        PART_BEGIN(H5Oopen_by_idx_invalid_lapl) {
            TESTING_2("H5Oopen_by_idx with an invalid LAPL")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_idx(container_group, OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_INC, 0, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_idx succeeded with an invalid LAPL!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_idx_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Oopen_by_idx_invalid_lapl);

        PART_BEGIN(H5Oopen_by_addr_invalid_loc_id) {
            TESTING_2("H5Oopen_by_addr with an invalid location ID")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_addr(H5I_INVALID_HID, 0);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_addr succeeded with an invalid location ID!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_addr_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Oopen_by_addr_invalid_loc_id);

        PART_BEGIN(H5Oopen_by_addr_invalid_addr) {
            TESTING_2("H5Oopen_by_addr with an invalid address")

            H5E_BEGIN_TRY {
                group_id2 = H5Oopen_by_addr(file_id, 0);
            } H5E_END_TRY;

            if (group_id2 >= 0) {
                H5_FAILED();
                HDprintf("    H5Oopen_by_addr succeeded with an invalid address!\n");
                H5Gclose(group_id2);
                PART_ERROR(H5Oopen_by_addr_invalid_addr);
            }

            PASSED();
        } PART_END(H5Oopen_by_addr_invalid_addr);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test for H5Oexists_by_name.
 */
static int
test_object_exists(void)
{
    htri_t object_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  group_id2 = H5I_INVALID_HID;
    hid_t  dset_id = H5I_INVALID_HID;
    hid_t  dtype_id = H5I_INVALID_HID;
    hid_t  fspace_id = H5I_INVALID_HID;
    hid_t  dset_dtype = H5I_INVALID_HID;

    TESTING_MULTIPART("object existence");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_EXISTS_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", OBJECT_EXISTS_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((fspace_id = generate_random_dataspace(OBJECT_EXISTS_TEST_DSET_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    PASSED();

    /*
     * NOTE: H5Oexists_by_name for hard links should always succeed.
     *       H5Oexists_by_name for a soft link may fail if the link doesn't resolve.
     */
    BEGIN_MULTIPART {
        PART_BEGIN(H5Oexists_by_name_group) {
            TESTING_2("H5Oexists_by_name on a group")

            if ((group_id2 = H5Gcreate2(group_id, OBJECT_EXISTS_TEST_GRP_NAME,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create group '%s'\n", OBJECT_EXISTS_TEST_GRP_NAME);
                PART_ERROR(H5Oexists_by_name_group);
            }

            if ((object_exists = H5Oexists_by_name(group_id, OBJECT_EXISTS_TEST_GRP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if object '%s' exists\n", OBJECT_EXISTS_TEST_GRP_NAME);
                PART_ERROR(H5Oexists_by_name_group);
            }

            if (!object_exists) {
                H5_FAILED();
                HDprintf("    object '%s' didn't exist!\n", OBJECT_EXISTS_TEST_GRP_NAME);
                PART_ERROR(H5Oexists_by_name_group);
            }

            if (H5Gclose(group_id2) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close group\n");
                PART_ERROR(H5Oexists_by_name_group);
            }

            PASSED();
        } PART_END(H5Oexists_by_name_group);

        PART_BEGIN(H5Oexists_by_name_dset) {
            TESTING_2("H5Oexists_by_name on a dataset")

            if ((dset_id = H5Dcreate2(group_id, OBJECT_EXISTS_TEST_DSET_NAME, dset_dtype, fspace_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create dataset '%s'\n", OBJECT_EXISTS_TEST_DSET_NAME);
                PART_ERROR(H5Oexists_by_name_dset);
            }

            if ((object_exists = H5Oexists_by_name(group_id, OBJECT_EXISTS_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if object '%s' exists\n", OBJECT_EXISTS_TEST_DSET_NAME);
                PART_ERROR(H5Oexists_by_name_dset);
            }

            if (!object_exists) {
                H5_FAILED();
                HDprintf("    object '%s' didn't exist!\n", OBJECT_EXISTS_TEST_DSET_NAME);
                PART_ERROR(H5Oexists_by_name_dset);
            }

            if (H5Dclose(dset_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close dataset\n");
                PART_ERROR(H5Oexists_by_name_dset);
            }

            PASSED();
        } PART_END(H5Oexists_by_name_dset);

        PART_BEGIN(H5Oexists_by_name_dtype) {
            TESTING_2("H5Oexists_by_name on a committed datatype")

            if ((dtype_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create datatype '%s'\n", OBJECT_EXISTS_TEST_TYPE_NAME);
                PART_ERROR(H5Oexists_by_name_dtype);
            }

            if (H5Tcommit2(group_id, OBJECT_EXISTS_TEST_TYPE_NAME, dtype_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't commit datatype '%s'\n", OBJECT_EXISTS_TEST_TYPE_NAME);
                PART_ERROR(H5Oexists_by_name_dtype);
            }

            if ((object_exists = H5Oexists_by_name(group_id, OBJECT_EXISTS_TEST_TYPE_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if object '%s' exists\n", OBJECT_EXISTS_TEST_TYPE_NAME);
                PART_ERROR(H5Oexists_by_name_dtype);
            }

            if (!object_exists) {
                H5_FAILED();
                HDprintf("    object '%s' didn't exist!\n", OBJECT_EXISTS_TEST_TYPE_NAME);
                PART_ERROR(H5Oexists_by_name_dtype);
            }

            if (H5Tclose(dtype_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close datatype\n");
                PART_ERROR(H5Oexists_by_name_dtype);
            }

            PASSED();
        } PART_END(H5Oexists_by_name_dtype);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(fspace_id);
        H5Tclose(dset_dtype);
        H5Tclose(dtype_id);
        H5Dclose(dset_id);
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that H5Oexists_by_name fails
 * when it is passed invalid parameters.
 */
static int
test_object_exists_invalid_params(void)
{
    htri_t object_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  group_id2 = H5I_INVALID_HID;

    TESTING_MULTIPART("object existence with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_EXISTS_INVALID_PARAMS_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", OBJECT_EXISTS_INVALID_PARAMS_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((group_id2 = H5Gcreate2(group_id, OBJECT_EXISTS_INVALID_PARAMS_TEST_GRP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", OBJECT_EXISTS_INVALID_PARAMS_TEST_GRP_NAME);
        goto error;
    }

    if (H5Gclose(group_id2) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Oexists_by_name_invalid_loc_id) {
            TESTING_2("H5Oexists_by_name with an invalid location ID")

            H5E_BEGIN_TRY {
                object_exists = H5Oexists_by_name(H5I_INVALID_HID, OBJECT_EXISTS_INVALID_PARAMS_TEST_GRP_NAME, H5P_DEFAULT);
            } H5E_END_TRY;

            if (object_exists >= 0) {
                H5_FAILED();
                HDprintf("    H5Oexists_by_name succeeded with an invalid location ID!\n");
                PART_ERROR(H5Oexists_by_name_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Oexists_by_name_invalid_loc_id);

        PART_BEGIN(H5Oexists_by_name_invalid_obj_name) {
            TESTING_2("H5Oexists_by_name with an invalid object name")

            H5E_BEGIN_TRY {
                object_exists = H5Oexists_by_name(group_id, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (object_exists >= 0) {
                H5_FAILED();
                HDprintf("    H5Oexists_by_name succeeded with a NULL object name!\n");
                PART_ERROR(H5Oexists_by_name_invalid_obj_name);
            }

            H5E_BEGIN_TRY {
                object_exists = H5Oexists_by_name(group_id, "", H5P_DEFAULT);
            } H5E_END_TRY;

            if (object_exists >= 0) {
                H5_FAILED();
                HDprintf("    H5Oexists_by_name succeeded with an invalid object name of ''!\n");
                PART_ERROR(H5Oexists_by_name_invalid_obj_name);
            }

            PASSED();
        } PART_END(H5Oexists_by_name_invalid_obj_name);

        PART_BEGIN(H5Oexists_by_name_invalid_lapl) {
            TESTING_2("H5Oexists_by_name with an invalid LAPL")

            H5E_BEGIN_TRY {
                object_exists = H5Oexists_by_name(group_id, OBJECT_EXISTS_INVALID_PARAMS_TEST_GRP_NAME, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (object_exists >= 0) {
                H5_FAILED();
                HDprintf("    H5Oexists_by_name succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Oexists_by_name_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Oexists_by_name_invalid_lapl);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test for H5Oget_info(_by_name/_by_idx).
 */
static int
test_get_object_info(void)
{
    TESTING("object info retrieval")

    SKIPPED();

    return 0;
}

/*
 * A test to check that an object's info can't be retrieved
 * when H5Oget_info(_by_name/_by_idx) are passed invalid
 * parameters.
 */
static int
test_get_object_info_invalid_params(void)
{
    TESTING("object info retrieval with invalid parameters")

    SKIPPED();

    return 0;
}

/*
 * A test for H5Olink.
 */
static int
test_link_object(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t group_id2 = H5I_INVALID_HID;
    hid_t dset_id = H5I_INVALID_HID;
    hid_t dset_dtype = H5I_INVALID_HID;
    hid_t type_id = H5I_INVALID_HID;
    hid_t fspace_id = H5I_INVALID_HID;

    TESTING_MULTIPART("object linking");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_LINK_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", OBJECT_LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((fspace_id = generate_random_dataspace(OBJECT_LINK_TEST_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Olink_group) {
            TESTING_2("H5Olink an anonymous group")

            if ((group_id2 = H5Gcreate_anon(group_id, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create an anonymous group\n");
                PART_ERROR(H5Olink_group);
            }

            if (H5Olink(group_id2, group_id, OBJECT_LINK_TEST_GROUP_NAME2, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't link the anonymous group\n");
                PART_ERROR(H5Olink_group);
            }

            PASSED();
        } PART_END(H5Olink_group);

        PART_BEGIN(H5Olink_dataset) {
            TESTING_2("H5Olink an anonymous dataset")

            if ((dset_id = H5Dcreate_anon(group_id, dset_dtype, fspace_id, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create an anonymous dataset\n");
                PART_ERROR(H5Olink_dataset);
            }

            if (H5Olink(dset_id, group_id, OBJECT_LINK_TEST_DSET_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't link the anonymous dataset\n");
                PART_ERROR(H5Olink_dataset);
            }

            PASSED();
        } PART_END(H5Olink_dataset);

        PART_BEGIN(H5Olink_datatype) {
            TESTING_2("H5Olink an anonymous datatype")

            if (H5Tcommit_anon(group_id, dset_dtype, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create an anonymous datatype\n");
                PART_ERROR(H5Olink_datatype);
            }

            if (H5Olink(dset_dtype, group_id, OBJECT_LINK_TEST_DTYPE_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't link the anonymous datatype\n");
                PART_ERROR(H5Olink_datatype);
            }

            PASSED();
        } PART_END(H5Olink_datatype);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id2) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(fspace_id);
        H5Tclose(dset_dtype);
        H5Tclose(type_id);
        H5Dclose(dset_id);
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that an object can't be linked into
 * the file structure when H5Olink is passed invalid
 * parameters.
 */
static int
test_link_object_invalid_params(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t group_id2 = H5I_INVALID_HID;
    herr_t status;

    TESTING_MULTIPART("object linking with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_LINK_INVALID_PARAMS_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", OBJECT_LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id2 = H5Gcreate_anon(group_id, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create an anonymous group\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Olink_invalid_object_id) {
            TESTING_2("H5Olink with invalid object ID")

            H5E_BEGIN_TRY {
                status = H5Olink(H5I_INVALID_HID, group_id, OBJECT_LINK_TEST_GROUP_NAME2, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Olink linked to a invalid object ID\n");
                PART_ERROR(H5Olink_invalid_location);
            }

            PASSED();
        } PART_END(H5Olink_invalid_object_id);

        PART_BEGIN(H5Olink_invalid_location) {
            TESTING_2("H5Olink with invalid location ID")

            H5E_BEGIN_TRY {
                status = H5Olink(group_id2, H5I_INVALID_HID, OBJECT_LINK_TEST_GROUP_NAME2, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Olink linked to a invalid location ID\n");
                PART_ERROR(H5Olink_invalid_location);
            }

            PASSED();
        } PART_END(H5Olink_invalid_location);

        PART_BEGIN(H5Olink_invalid_name) {
            TESTING_2("H5Olink with invalid name")

            H5E_BEGIN_TRY {
                status = H5Olink(group_id2, group_id, NULL, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Olink linked with NULL as the name\n");
                PART_ERROR(H5Olink_invalid_name);
            }

            H5E_BEGIN_TRY {
                status = H5Olink(group_id2, group_id, "", H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Olink linked with NULL as the name\n");
                PART_ERROR(H5Olink_invalid_name);
            }

            PASSED();
        } PART_END(H5Olink_invalid_name);

        PART_BEGIN(H5Olink_invalid_plist) {
            TESTING_2("H5Olink with invalid property list")

            H5E_BEGIN_TRY {
                status = H5Olink(group_id2, group_id, OBJECT_LINK_TEST_GROUP_NAME2, H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Olink linked to invalid property list\n");
                PART_ERROR(H5Olink_invalid_plist);
            }

            H5E_BEGIN_TRY {
                status = H5Olink(group_id2, group_id, OBJECT_LINK_TEST_GROUP_NAME2, H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Olink linked to invalid property list\n");
                PART_ERROR(H5Olink_invalid_plist);
            }

            PASSED();
        } PART_END(H5Olink_invalid_plist);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id2) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;

}

/*
 * A test for H5Oincr_refcount/H5Odecr_refcount.
 */
static int
test_incr_decr_object_refcount(void)
{
    htri_t object_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  group_id2 = H5I_INVALID_HID;
    hid_t  dset_id = H5I_INVALID_HID;
    hid_t  fspace_id = H5I_INVALID_HID;
    hid_t  dset_dtype = H5I_INVALID_HID;
    H5O_info_t  oinfo;                      /* Object info struct */

    TESTING_MULTIPART("increment/decrement the reference count of object");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_REF_COUNT_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", OBJECT_REF_COUNT_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((fspace_id = generate_random_dataspace(OBJECT_REF_COUNT_TEST_DSET_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Oincr_decr_refcount_group) {
            TESTING_2("H5Oincr_refcount/H5Odecr_refcount on a group")

            if ((group_id2 = H5Gcreate2(group_id, OBJECT_REF_COUNT_TEST_GRP_NAME,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create group '%s'\n", OBJECT_REF_COUNT_TEST_GRP_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            /* Increment the reference count */
            if (H5Oincr_refcount(group_id2) < 0) {
                H5_FAILED();
                HDprintf("    couldn't increment reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_GRP_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            /* Verify that reference count is 2 now */
            if (H5Oget_info_by_name2(group_id, OBJECT_REF_COUNT_TEST_GRP_NAME, &oinfo, H5O_INFO_BASIC, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_GRP_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (oinfo.rc != 2) {
                H5_FAILED();
                HDprintf("    the reference count for the group '%s' isn't 2: %d\n", OBJECT_REF_COUNT_TEST_GRP_NAME, oinfo.rc);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            /* Decrement the reference count */
            if (H5Odecr_refcount(group_id2) < 0) {
                H5_FAILED();
                HDprintf("    couldn't decrement reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_GRP_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            /* Verify that reference count is 1 now */
            if (H5Oget_info_by_name2(group_id, OBJECT_REF_COUNT_TEST_GRP_NAME, &oinfo, H5O_INFO_BASIC, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_GRP_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (oinfo.rc != 1) {
                H5_FAILED();
                HDprintf("    the reference count for the group '%s' isn't 1: %d\n", OBJECT_REF_COUNT_TEST_GRP_NAME, oinfo.rc);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (H5Gclose(group_id2) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close group\n");
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            PASSED();
        } PART_END(H5Oincr_decr_refcount_group);

        PART_BEGIN(H5Oincr_decr_refcount_dset) {
            TESTING_2("H5Oincr_refcount/H5Odecr_refcount on a dataset")

            if ((dset_id = H5Dcreate2(group_id, OBJECT_REF_COUNT_TEST_DSET_NAME, dset_dtype, fspace_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create dataset '%s'\n", OBJECT_REF_COUNT_TEST_DSET_NAME);
                PART_ERROR(H5Oincr_decr_refcount_dset);
            }

            /* Increment the reference count */
            if (H5Oincr_refcount(dset_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't increment reference count for the dataset '%s' \n", OBJECT_REF_COUNT_TEST_DSET_NAME);
                PART_ERROR(H5Oincr_decr_refcount_dset);
            }

            /* Verify that reference count is 2 now */
            if (H5Oget_info_by_name2(group_id, OBJECT_REF_COUNT_TEST_DSET_NAME, &oinfo, H5O_INFO_BASIC, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_GRP_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (oinfo.rc != 2) {
                H5_FAILED();
                HDprintf("    the reference count for the group '%s' isn't 2: %d\n", OBJECT_REF_COUNT_TEST_DSET_NAME, oinfo.rc);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            /* Decrement the reference count */
            if (H5Odecr_refcount(dset_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't decrement reference count for the dataset '%s' \n", OBJECT_REF_COUNT_TEST_DSET_NAME);
                PART_ERROR(H5Oincr_decr_refcount_dset);
            }

            /* Verify that reference count is 1 now */
            if (H5Oget_info_by_name2(group_id, OBJECT_REF_COUNT_TEST_DSET_NAME, &oinfo, H5O_INFO_BASIC, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_DSET_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (oinfo.rc != 1) {
                H5_FAILED();
                HDprintf("    the reference count for the group '%s' isn't 1: %d\n", OBJECT_REF_COUNT_TEST_DSET_NAME, oinfo.rc);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (H5Dclose(dset_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close dataset\n");
                PART_ERROR(H5Oincr_decr_refcount_dset);
            }

            PASSED();
        } PART_END(H5Oincr_decr_refcount_dset);

        PART_BEGIN(H5Oincr/decr_refcount_dtype) {
            TESTING_2("H5Oincr_refcount/H5Odecr_refcount on a committed datatype")

            if (H5Tcommit2(group_id, OBJECT_REF_COUNT_TEST_TYPE_NAME, dset_dtype,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't commit datatype '%s'\n", OBJECT_REF_COUNT_TEST_TYPE_NAME);
                PART_ERROR(H5Oincr_decr_refcount_dtype);
            }

            /* Increment the reference count */
            if (H5Oincr_refcount(dset_dtype) < 0) {
                H5_FAILED();
                HDprintf("    couldn't increment reference count for the datatype '%s' \n", OBJECT_REF_COUNT_TEST_TYPE_NAME);
                PART_ERROR(H5Oincr_decr_refcount_dset);
            }

            /* Verify that reference count is 2 now */
            if (H5Oget_info_by_name2(group_id, OBJECT_REF_COUNT_TEST_TYPE_NAME, &oinfo, H5O_INFO_BASIC, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_TYPE_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (oinfo.rc != 2) {
                H5_FAILED();
                HDprintf("    the reference count for the group '%s' isn't 2: %d\n", OBJECT_REF_COUNT_TEST_TYPE_NAME, oinfo.rc);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            /* Decrement the reference count */
            if (H5Odecr_refcount(dset_dtype) < 0) {
                H5_FAILED();
                HDprintf("    couldn't decrement reference count for the datatype '%s' \n", OBJECT_REF_COUNT_TEST_TYPE_NAME);
                PART_ERROR(H5Oincr_decr_refcount_dset);
            }

            /* Verify that reference count is 1 now */
            if (H5Oget_info_by_name2(group_id, OBJECT_REF_COUNT_TEST_TYPE_NAME, &oinfo, H5O_INFO_BASIC, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get reference count for the group '%s' \n", OBJECT_REF_COUNT_TEST_TYPE_NAME);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (oinfo.rc != 1) {
                H5_FAILED();
                HDprintf("    the reference count for the group '%s' isn't 1: %d\n", OBJECT_REF_COUNT_TEST_TYPE_NAME, oinfo.rc);
                PART_ERROR(H5Oincr_decr_refcount_group);
            }

            if (H5Tclose(dset_dtype) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close datatype\n");
                PART_ERROR(H5Oincr_decr_refcount_dtype);
            }

            PASSED();
        } PART_END(H5Oincr_decr_refcount_dtype);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(fspace_id);
        H5Tclose(dset_dtype);
        H5Dclose(dset_id);
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
} /* test_incr_decr_object_refcount */

/*
 * A test to check that H5Oincr_refcount/H5Odecr_refcount
 * fail when passed invalid parameters.
 */
static int
test_incr_decr_object_refcount_invalid_params(void)
{
    herr_t status;

    TESTING_MULTIPART("object reference count incr./decr. with an invalid parameter");

    BEGIN_MULTIPART {
        PART_BEGIN(H5Oincr_refcount_invalid_param) {
            TESTING_2("H5Oincr_refcount with invalid object ID")

            H5E_BEGIN_TRY {
                status = H5Oincr_refcount(H5I_INVALID_HID);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    incremented the reference count for an invalid object ID\n");
                PART_ERROR(H5Oincr_refcount_invalid_param);
            }

            PASSED();
        } PART_END(H5Oincr_refcount_invalid_param);

        PART_BEGIN(H5Odecr_refcount_invalid_param) {
            TESTING_2("H5Odecr_refcount with invalid object ID")

            H5E_BEGIN_TRY {
                status = H5Odecr_refcount(H5I_INVALID_HID);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    decremented the reference count for an invalid object ID\n");
                PART_ERROR(H5Odecr_refcount_invalid_param);
            }

            PASSED();
        } PART_END(H5Odecr_refcount_invalid_param);
    } END_MULTIPART;

    return 0;

error:
    return 1;
}

/*
 * A test for H5Ocopy.
 */
static int
test_copy_object(void)
{
    htri_t object_link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  group_id2 = H5I_INVALID_HID;
    hid_t  dset_id = H5I_INVALID_HID;
    hid_t  dset_dtype = H5I_INVALID_HID;
    hid_t  dtype_id = H5I_INVALID_HID;
    hid_t  space_id = H5I_INVALID_HID;

    TESTING_MULTIPART("object copying");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_COPY_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", OBJECT_COPY_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((space_id = generate_random_dataspace(OBJECT_COPY_TEST_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Ocopy_group) {
            TESTING_2("H5Ocopy on a group")

            if ((group_id2 = H5Gcreate2(group_id, OBJECT_COPY_TEST_GROUP_NAME,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create group '%s'\n", OBJECT_COPY_TEST_GROUP_NAME);
                PART_ERROR(H5Ocopy_group);
            }

            if (H5Ocopy(group_id, OBJECT_COPY_TEST_GROUP_NAME, group_id, OBJECT_COPY_TEST_GROUP_NAME2,
                    H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to copy object '%s'\n", OBJECT_COPY_TEST_GROUP_NAME);
                PART_ERROR(H5Ocopy_group);
            }

            if ((object_link_exists = H5Lexists(group_id, OBJECT_COPY_TEST_GROUP_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' to copied group exists\n", OBJECT_COPY_TEST_GROUP_NAME2);
                PART_ERROR(H5Ocopy_group);
            }

            if (!object_link_exists) {
                H5_FAILED();
                HDprintf("    link '%s' to copied group didn't exist!\n", OBJECT_COPY_TEST_GROUP_NAME2);
                PART_ERROR(H5Ocopy_group);
            }

            PASSED();
        } PART_END(H5Ocopy_group);

        PART_BEGIN(H5Ocopy_dset) {
            TESTING_2("H5Ocopy on a dataset")

            if ((dset_id = H5Dcreate2(group_id, OBJECT_COPY_TEST_DSET_NAME, dset_dtype, space_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create dataset '%s'\n", OBJECT_COPY_TEST_DSET_NAME);
                PART_ERROR(H5Ocopy_dset);
            }

            if (H5Ocopy(group_id, OBJECT_COPY_TEST_DSET_NAME, group_id, OBJECT_COPY_TEST_DSET_NAME2,
                    H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to copy object '%s'\n", OBJECT_COPY_TEST_DSET_NAME);
                PART_ERROR(H5Ocopy_dset);
            }

            if ((object_link_exists = H5Lexists(group_id, OBJECT_COPY_TEST_DSET_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' to copied dataset exists\n", OBJECT_COPY_TEST_DSET_NAME2);
                PART_ERROR(H5Ocopy_dset);
            }

            if (!object_link_exists) {
                H5_FAILED();
                HDprintf("    link '%s' to copied dataset didn't exist!\n", OBJECT_COPY_TEST_DSET_NAME2);
                PART_ERROR(H5Ocopy_dset);
            }

            PASSED();
        } PART_END(H5Ocopy_dset);

        PART_BEGIN(H5Ocopy_dtype) {
            TESTING_2("H5Ocopy on a committed datatype")

            if ((dtype_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create datatype '%s'\n", OBJECT_COPY_TEST_TYPE_NAME);
                PART_ERROR(H5Ocopy_dtype);
            }

            if (H5Tcommit2(group_id, OBJECT_COPY_TEST_TYPE_NAME, dtype_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't commit datatype '%s'\n", OBJECT_COPY_TEST_TYPE_NAME);
                PART_ERROR(H5Ocopy_dtype);
            }

            if (H5Ocopy(group_id, OBJECT_COPY_TEST_TYPE_NAME, group_id, OBJECT_COPY_TEST_TYPE_NAME2,
                    H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to copy object '%s'\n", OBJECT_COPY_TEST_TYPE_NAME);
                PART_ERROR(H5Ocopy_dtype);
            }

            if ((object_link_exists = H5Lexists(group_id, OBJECT_COPY_TEST_TYPE_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' to copied datatype exists\n", OBJECT_COPY_TEST_TYPE_NAME2);
                PART_ERROR(H5Ocopy_dtype);
            }

            if (!object_link_exists) {
                H5_FAILED();
                HDprintf("    link '%s' to copied datatype didn't exist!\n", OBJECT_COPY_TEST_TYPE_NAME2);
                PART_ERROR(H5Ocopy_dtype);
            }

            PASSED();
        } PART_END(H5Ocopy_dtype);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(space_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Tclose(dtype_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id2) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(space_id);
        H5Tclose(dset_dtype);
        H5Tclose(dtype_id);
        H5Dclose(dset_id);
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that H5Ocopy fails when it
 * is passed invalid parameters.
 */
static int
test_copy_object_invalid_params(void)
{
    herr_t err_ret = -1;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  group_id2 = H5I_INVALID_HID;

    TESTING_MULTIPART("object copying with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_COPY_INVALID_PARAMS_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", OBJECT_COPY_INVALID_PARAMS_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((group_id2 = H5Gcreate2(group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Ocopy_invalid_src_loc_id) {
            TESTING_2("H5Ocopy with an invalid source location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(H5I_INVALID_HID, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME, group_id,
                        OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME2, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with an invalid source location ID!\n");
                PART_ERROR(H5Ocopy_invalid_src_loc_id);
            }

            PASSED();
        } PART_END(H5Ocopy_invalid_src_loc_id);

        PART_BEGIN(H5Ocopy_invalid_src_obj_name) {
            TESTING_2("H5Ocopy with an invalid source object name")

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(group_id, NULL, group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME2,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with a NULL source object name!\n");
                PART_ERROR(H5Ocopy_invalid_src_obj_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(group_id, "", group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME2,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with an invalid source object name of ''!\n");
                PART_ERROR(H5Ocopy_invalid_src_obj_name);
            }

            PASSED();
        } PART_END(H5Ocopy_invalid_src_obj_name);

        PART_BEGIN(H5Ocopy_invalid_dst_loc_id) {
            TESTING_2("H5Ocopy with an invalid destination location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME, H5I_INVALID_HID,
                        OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME2, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with an invalid destination location ID!\n");
                PART_ERROR(H5Ocopy_invalid_dst_loc_id);
            }

            PASSED();
        } PART_END(H5Ocopy_invalid_dst_loc_id);

        PART_BEGIN(H5Ocopy_invalid_dst_obj_name) {
            TESTING_2("H5Ocopy with an invalid destination object name")

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME, group_id,
                        NULL, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with a NULL destination object name!\n");
                PART_ERROR(H5Ocopy_invalid_dst_obj_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME, group_id,
                        "", H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with an invalid destination object name of ''!\n");
                PART_ERROR(H5Ocopy_invalid_dst_obj_name);
            }

            PASSED();
        } PART_END(H5Ocopy_invalid_dst_obj_name);

        PART_BEGIN(H5Ocopy_invalid_ocpypl) {
            TESTING_2("H5Ocopy with an invalid OcpyPL")

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME, group_id,
                        OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME2, H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with an invalid OcpyPL!\n");
                PART_ERROR(H5Ocopy_invalid_ocpypl);
            }

            PASSED();
        } PART_END(H5Ocopy_invalid_ocpypl);

        PART_BEGIN(H5Ocopy_invalid_lcpl) {
            TESTING_2("H5Ocopy with an invalid LCPL")

            H5E_BEGIN_TRY {
                err_ret = H5Ocopy(group_id, OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME, group_id,
                        OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME2, H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ocopy succeeded with an invalid LCPL!\n");
                PART_ERROR(H5Ocopy_invalid_lcpl);
            }

            PASSED();
        } PART_END(H5Ocopy_invalid_lcpl);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Gclose(group_id2) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test for H5Oset_comment(_by_name)/H5Oget_comment(_by_name).
 */
static int
test_object_comments(void)
{
    TESTING("object comments")

    SKIPPED();

    return 0;
}

/*
 * A test to check that H5Oset_comment(_by_name)/H5Oget_comment(_by_name)
 * fail when passed invalid parameters.
 */
static int
test_object_comments_invalid_params(void)
{
    TESTING("object comment ")

    SKIPPED();

    return 0;
}

/*
 * A test for H5Ovisit(_by_name).
 */
static int
test_object_visit(void)
{
    size_t i;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  group_id2 = H5I_INVALID_HID;
    hid_t  type_id = H5I_INVALID_HID;
    hid_t  dset_id = H5I_INVALID_HID;
    hid_t  dset_dtype = H5I_INVALID_HID;
    hid_t  fspace_id = H5I_INVALID_HID;

    TESTING_MULTIPART("object visiting");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_VISIT_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", OBJECT_VISIT_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((fspace_id = generate_random_dataspace(OBJECT_VISIT_TEST_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    if ((type_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype '%s'\n", OBJECT_VISIT_TEST_TYPE_NAME);
        goto error;
    }

    if ((group_id2 = H5Gcreate2(group_id, OBJECT_VISIT_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", OBJECT_VISIT_TEST_GROUP_NAME);
        goto error;
    }

    if ((dset_id = H5Dcreate2(group_id, OBJECT_VISIT_TEST_DSET_NAME, dset_dtype,
            fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset '%s'\n", OBJECT_VISIT_TEST_DSET_NAME);
        goto error;
    }

    if (H5Tcommit2(group_id, OBJECT_VISIT_TEST_TYPE_NAME, type_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't commit datatype '%s'\n", OBJECT_VISIT_TEST_TYPE_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        /*
         * NOTE: Pass a counter to the iteration callback to try to match up the
         * expected objects with a given step throughout all of the following
         * iterations. This is to try and check that the objects are indeed being
         * returned in the correct order.
         */
        i = 0;

        PART_BEGIN(H5Ovisit_obj_name_increasing) {
            TESTING_2("H5Ovisit by object name in increasing order")

            if (H5Ovisit2(group_id, H5_INDEX_NAME, H5_ITER_INC, object_visit_callback, &i, H5O_INFO_ALL) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit by object name in increasing order failed\n");
                PART_ERROR(H5Ovisit_obj_name_increasing);
            }

            PASSED();
        } PART_END(H5Ovisit_obj_name_increasing);

        PART_BEGIN(H5Ovisit_obj_name_decreasing) {
            TESTING_2("H5Ovisit by object name in decreasing order")

            if (H5Ovisit2(group_id, H5_INDEX_NAME, H5_ITER_DEC, object_visit_callback, &i, H5O_INFO_ALL) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit by object name in decreasing order failed\n");
                PART_ERROR(H5Ovisit_obj_name_decreasing);
            }

            PASSED();
        } PART_END(H5Ovisit_obj_name_decreasing);

        PART_BEGIN(H5Ovisit_create_order_increasing) {
            TESTING_2("H5Ovisit by creation order in increasing order")

            if (H5Ovisit2(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, object_visit_callback, &i, H5O_INFO_ALL) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit by creation order in increasing order failed\n");
                PART_ERROR(H5Ovisit_create_order_increasing);
            }

            PASSED();
        } PART_END(H5Ovisit_create_order_increasing);

        PART_BEGIN(H5Ovisit_create_order_decreasing) {
            TESTING_2("H5Ovisit by creation order in decreasing order")

            if (H5Ovisit2(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, object_visit_callback, &i, H5O_INFO_ALL) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit by creation order in decreasing order failed\n");
                PART_ERROR(H5Ovisit_create_order_decreasing);
            }

            PASSED();
        } PART_END(H5Ovisit_create_order_decreasing);

        /*
         * Make sure to reset the special counter.
         */
        i = 0;

        PART_BEGIN(H5Ovisit_by_name_obj_name_increasing) {
            TESTING_2("H5Ovisit_by_name by object name in increasing order")

            if (H5Ovisit_by_name2(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, object_visit_callback, &i, H5O_INFO_ALL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name by object name in increasing order failed\n");
                PART_ERROR(H5Ovisit_by_name_obj_name_increasing);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_obj_name_increasing);

        PART_BEGIN(H5Ovisit_by_name_obj_name_decreasing) {
            TESTING_2("H5Ovisit_by_name by object name in decreasing order")

            if (H5Ovisit_by_name2(group_id, ".", H5_INDEX_NAME, H5_ITER_DEC, object_visit_callback, &i, H5O_INFO_ALL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name by object name in decreasing order failed\n");
                PART_ERROR(H5Ovisit_by_name_obj_name_decreasing);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_obj_name_decreasing);

        PART_BEGIN(H5Ovisit_by_name_create_order_increasing) {
            TESTING_2("H5Ovisit_by_name by creation order in increasing order")

            if (H5Ovisit_by_name2(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, object_visit_callback, &i, H5O_INFO_ALL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name by creation order in increasing order failed\n");
                PART_ERROR(H5Ovisit_by_name_create_order_increasing);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_create_order_increasing);

        PART_BEGIN(H5Ovisit_by_name_create_order_decreasing) {
            TESTING_2("H5Ovisit_by_name by creation order in decreasing order")

            if (H5Ovisit_by_name2(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_DEC, object_visit_callback, &i, H5O_INFO_ALL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name by creation order in decreasing order failed\n");
                PART_ERROR(H5Ovisit_by_name_create_order_decreasing);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_create_order_decreasing);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Tclose(type_id) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id2) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(fspace_id);
        H5Tclose(dset_dtype);
        H5Tclose(type_id);
        H5Dclose(dset_id);
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that H5Ovisit(_by_name) fails when
 * it is passed invalid parameters.
 */
static int
test_object_visit_invalid_params(void)
{
    herr_t err_ret = -1;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  group_id2 = H5I_INVALID_HID;

    TESTING_MULTIPART("object visiting with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", OBJECT_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((group_id2 = H5Gcreate2(group_id, OBJECT_VISIT_INVALID_PARAMS_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", OBJECT_VISIT_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Ovisit_invalid_obj_id) {
            TESTING_2("H5Ovisit with an invalid object ID")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit2(H5I_INVALID_HID, H5_INDEX_NAME, H5_ITER_INC, object_visit_callback2, NULL, H5O_INFO_ALL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit succeeded with an invalid object ID!\n");
                PART_ERROR(H5Ovisit_invalid_obj_id);
            }

            PASSED();
        } PART_END(H5Ovisit_invalid_obj_id);

        PART_BEGIN(H5Ovisit_invalid_index_type) {
            TESTING_2("H5Ovisit with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit2(group_id, H5_INDEX_UNKNOWN, H5_ITER_INC, object_visit_callback2, NULL, H5O_INFO_ALL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Ovisit_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit2(group_id, H5_INDEX_N, H5_ITER_INC, object_visit_callback2, NULL, H5O_INFO_ALL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Ovisit_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Ovisit_invalid_index_type);

        PART_BEGIN(H5Ovisit_invalid_iter_order) {
            TESTING_2("H5Ovisit with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit2(group_id, H5_INDEX_NAME, H5_ITER_UNKNOWN, object_visit_callback2, NULL, H5O_INFO_ALL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Ovisit_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit2(group_id, H5_INDEX_NAME, H5_ITER_N, object_visit_callback2, NULL, H5O_INFO_ALL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Ovisit_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Ovisit_invalid_iter_order);

        PART_BEGIN(H5Ovisit_by_name_invalid_loc_id) {
            TESTING_2("H5Ovisit_by_name with an invalid location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(H5I_INVALID_HID, ".", H5_INDEX_NAME, H5_ITER_N, object_visit_callback2, NULL, H5O_INFO_ALL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with an invalid location ID!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_invalid_loc_id);

        PART_BEGIN(H5Ovisit_by_name_invalid_obj_name) {
            TESTING_2("H5Ovisit_by_name with an invalid object name")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(group_id, NULL, H5_INDEX_NAME, H5_ITER_N, object_visit_callback2, NULL, H5O_INFO_ALL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with a NULL object name!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_obj_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(group_id, "", H5_INDEX_NAME, H5_ITER_N, object_visit_callback2, NULL, H5O_INFO_ALL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with an invalid object name of ''!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_obj_name);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_invalid_obj_name);

        PART_BEGIN(H5Ovisit_by_name_invalid_index_type) {
            TESTING_2("H5Ovisit_by_name with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(group_id, ".", H5_INDEX_UNKNOWN, H5_ITER_N, object_visit_callback2, NULL, H5O_INFO_ALL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(group_id, ".", H5_INDEX_N, H5_ITER_N, object_visit_callback2, NULL, H5O_INFO_ALL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_invalid_index_type);

        PART_BEGIN(H5Ovisit_by_name_invalid_iter_order) {
            TESTING_2("H5Ovisit_by_name with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(group_id, ".", H5_INDEX_NAME, H5_ITER_UNKNOWN, object_visit_callback2, NULL, H5O_INFO_ALL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(group_id, ".", H5_INDEX_NAME, H5_ITER_N, object_visit_callback2, NULL, H5O_INFO_ALL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_invalid_iter_order);

        PART_BEGIN(H5Ovisit_by_name_invalid_lapl) {
            TESTING_2("H5Ovisit_by_name with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Ovisit_by_name2(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, object_visit_callback2, NULL, H5O_INFO_ALL, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ovisit_by_name succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Ovisit_by_name_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Ovisit_by_name_invalid_lapl);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Gclose(group_id2) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test for H5Oclose.
 */
static int
test_close_object(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t group_id2 = H5I_INVALID_HID;
    hid_t dtype_id = H5I_INVALID_HID;
    hid_t dset_id = H5I_INVALID_HID;
    hid_t dset_dtype = H5I_INVALID_HID;
    hid_t fspace_id = H5I_INVALID_HID;

    TESTING_MULTIPART("H5Oclose");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_CLOSE_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", OBJECT_CLOSE_TEST_GROUP_NAME);
        goto error;
    }

    if ((fspace_id = generate_random_dataspace(OBJECT_CLOSE_TEST_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Oclose_group) {
            TESTING_2("H5Oclose on a group")

            if ((group_id2 = H5Gcreate2(group_id, OBJECT_CLOSE_TEST_GRP_NAME,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create group '%s'\n", OBJECT_CLOSE_TEST_GRP_NAME);
                PART_ERROR(H5Oclose_group);
            }

            H5E_BEGIN_TRY {
                H5Gclose(group_id2);
            } H5E_END_TRY;

            if ((group_id2 = H5Oopen(group_id, OBJECT_CLOSE_TEST_GRP_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open group '%s' with H5Oopen\n", OBJECT_CLOSE_TEST_GRP_NAME);
                PART_ERROR(H5Oclose_group);
            }

            if (H5Oclose(group_id2) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close group '%s' with H5Oclose\n", OBJECT_CLOSE_TEST_GRP_NAME);
                PART_ERROR(H5Oclose_group);
            }

            PASSED();
        } PART_END(H5Oclose_group);

        PART_BEGIN(H5Oclose_dset) {
            TESTING_2("H5Oclose on a dataset")

            if ((dset_id = H5Dcreate2(group_id, OBJECT_CLOSE_TEST_DSET_NAME, dset_dtype,
                    fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create dataset '%s'\n", OBJECT_CLOSE_TEST_DSET_NAME);
                PART_ERROR(H5Oclose_dset);
            }

            H5E_BEGIN_TRY {
                H5Dclose(dset_id);
            } H5E_END_TRY;

            if ((dset_id = H5Oopen(group_id, OBJECT_CLOSE_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open dataset '%s' with H5Oopen\n", OBJECT_CLOSE_TEST_DSET_NAME);
                PART_ERROR(H5Oclose_dset);
            }

            if (H5Oclose(dset_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close dataset '%s' with H5Oclose\n", OBJECT_CLOSE_TEST_DSET_NAME);
                PART_ERROR(H5Oclose_dset);
            }

            PASSED();
        } PART_END(H5Oclose_dset);

        PART_BEGIN(H5Oclose_dtype) {
            TESTING_2("H5Oclose on a committed datatype")

            if ((dtype_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create datatype '%s'\n", OBJECT_CLOSE_TEST_TYPE_NAME);
                PART_ERROR(H5Oclose_dtype);
            }

            if (H5Tcommit2(group_id, OBJECT_CLOSE_TEST_TYPE_NAME, dtype_id,
                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't commit datatype '%s'\n", OBJECT_CLOSE_TEST_TYPE_NAME);
                PART_ERROR(H5Oclose_dtype);
            }

            H5E_BEGIN_TRY {
                H5Tclose(dtype_id);
            } H5E_END_TRY;

            if ((dtype_id = H5Oopen(group_id, OBJECT_CLOSE_TEST_TYPE_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't open datatype '%s' with H5Oopen\n", OBJECT_CLOSE_TEST_TYPE_NAME);
                PART_ERROR(H5Oclose_dtype);
            }

            if (H5Oclose(dtype_id) < 0) {
                H5_FAILED();
                HDprintf("    couldn't close datatype '%s' with H5Oclose\n", OBJECT_CLOSE_TEST_TYPE_NAME);
                PART_ERROR(H5Oclose_dtype);
            }

            PASSED();
        } PART_END(H5Oclose_dtype);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Sclose(fspace_id);
        H5Tclose(dset_dtype);
        H5Tclose(dtype_id);
        H5Dclose(dset_id);
        H5Gclose(group_id2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that H5Oclose fails when it
 * is passed invalid parameters.
 */
static int
test_close_object_invalid_params(void)
{
    herr_t err_ret = -1;
    hid_t  file_id = H5I_INVALID_HID;

    TESTING("H5Oclose with an invalid object ID")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    H5E_BEGIN_TRY {
        err_ret = H5Oclose(H5I_INVALID_HID);
    } H5E_END_TRY;

    if (err_ret >= 0) {
        H5_FAILED();
        HDprintf("    H5Oclose succeeded with an invalid object ID!\n");
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that various objects (file, dataspace, property list,
 * and attribute) can't be closed with H5Oclose.
 */
static int
test_close_invalid_objects(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t attr_dtype = H5I_INVALID_HID;
    hid_t attr_space_id = H5I_INVALID_HID;
    hid_t fapl_id = H5I_INVALID_HID;
    hid_t attr_id = H5I_INVALID_HID;
    herr_t status;

    TESTING_MULTIPART("H5Oclose invalid objects");

    TESTING_2("test setup")

    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", OBJECT_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJECT_CLOSE_INVALID_TEST_GROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", OBJECT_OPEN_TEST_GROUP_NAME);
        goto error;
    }

    if ((attr_space_id = generate_random_dataspace(OBJECT_CLOSE_INVALID_TEST_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((attr_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    if ((attr_id = H5Acreate2(group_id, OBJECT_CLOSE_INVALID_TEST_ATTRIBUTE_NAME, attr_dtype, attr_space_id, H5P_DEFAULT, H5P_DEFAULT)) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Oclose_file) {
            TESTING_2("H5Oclose with an invalid object - file")

            H5E_BEGIN_TRY {
                status = H5Oclose(file_id);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Oclose succeeded with an invalid object (file)!\n");
                PART_ERROR(H5Oclose_file);
            }

            PASSED();
        } PART_END(H5Oclose_file);

        PART_BEGIN(H5Oclose_plist) {
            TESTING_2("H5Oclose with an invalid object - property list")

            H5E_BEGIN_TRY {
                status = H5Oclose(fapl_id);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Oclose succeeded with an invalid object (property list)!\n");
                PART_ERROR(H5Oclose_plist);
            }

            PASSED();
        } PART_END(H5Oclose_plist);

        PART_BEGIN(H5Oclose_dspace) {
            TESTING_2("H5Oclose with an invalid object - data space")

            H5E_BEGIN_TRY {
                status = H5Oclose(attr_space_id);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Oclose succeeded with an invalid object (data space)!\n");
                PART_ERROR(H5Oclose_dspace);
            }

            PASSED();
        } PART_END(H5Oclose_dspace);

        PART_BEGIN(H5Oclose_attribute) {
            TESTING_2("H5Oclose with an invalid object - attribute")

            H5E_BEGIN_TRY {
                status = H5Oclose(attr_id);
            } H5E_END_TRY;

            if (status >= 0) {
                H5_FAILED();
                HDprintf("    H5Oclose succeeded with an invalid object (attribute)!\n");
                PART_ERROR(H5Oclose_attribute);
            }

            PASSED();
        } PART_END(H5Oclose_attribute);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Tclose(attr_dtype) < 0)
        TEST_ERROR
    if (H5Aclose(attr_id) < 0)
        TEST_ERROR
    if (H5Sclose(attr_space_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR
    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Tclose(attr_dtype);
        H5Sclose(attr_space_id);
        H5Aclose(attr_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
        H5Pclose(fapl_id);
    } H5E_END_TRY;

    return 1;
} /* test_close_invalid_objects */

/*
 * A test for H5Oflush.
 */
static int
test_flush_object(void)
{
    TESTING("H5Oflush")

    SKIPPED();

    return 0;
}

/*
 * A test to check that H5Oflush fails when
 * it is passed invalid parameters.
 */
static int
test_flush_object_invalid_params(void)
{
    TESTING("H5Oflush with invalid parameters")

    SKIPPED();

    return 0;
}

/*
 * A test for H5Orefresh.
 */
static int
test_refresh_object(void)
{
    TESTING("H5Orefresh")

    SKIPPED();

    return 0;
}

/*
 * A test to check that H5Orefresh fails when
 * it is passed invalid parameters.
 */
static int
test_refresh_object_invalid_params(void)
{
    TESTING("H5Orefresh with invalid parameters")

    SKIPPED();

    return 0;
}

#ifndef NO_REF_TESTS
static int
test_create_obj_ref(void)
{
    vol_test_obj_ref_t ref;
    hid_t              file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;

    TESTING("create an object reference")

    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file\n");
        goto error;
    }

    if (H5Rcreate((void *) &ref, file_id, "/", H5R_OBJECT, H5I_INVALID_HID) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create obj. ref\n");
        goto error;
    }

    if (H5R_OBJECT != ref.ref_type) TEST_ERROR
    if (H5I_GROUP != ref.ref_obj_type) TEST_ERROR

    if (H5Pclose(fapl_id) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_dereference_reference(void)
{
    TESTING("dereference a reference")

    /* H5Rdereference2 */

    SKIPPED();

    return 0;
}

static int
test_get_ref_type(void)
{
    vol_test_obj_ref_t ref_array[3];
    H5O_type_t         obj_type;
    size_t             i;
    hid_t              file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t              container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t              ref_dset_id = H5I_INVALID_HID;
    hid_t              ref_dtype_id = H5I_INVALID_HID;
    hid_t              ref_dset_dtype = H5I_INVALID_HID;
    hid_t              space_id = H5I_INVALID_HID;

    TESTING("retrieve type of object reference by an object/region reference")

    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file\n");
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJ_REF_GET_TYPE_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group\n");
        goto error;
    }

    if ((space_id = generate_random_dataspace(OBJ_REF_GET_TYPE_TEST_SPACE_RANK, NULL)) < 0)
        TEST_ERROR

    if ((ref_dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    /* Create the dataset and datatype which will be referenced */
    if ((ref_dset_id = H5Dcreate2(group_id, OBJ_REF_GET_TYPE_TEST_DSET_NAME, ref_dset_dtype,
            space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset for referencing\n");
        goto error;
    }

    if ((ref_dtype_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype\n");
        goto error;
    }

    if (H5Tcommit2(group_id, OBJ_REF_GET_TYPE_TEST_TYPE_NAME, ref_dtype_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype for referencing\n");
        goto error;
    }

    {
        /* TODO: Temporary workaround for datatypes */
        if (H5Tclose(ref_dtype_id) < 0)
            TEST_ERROR

        if ((ref_dtype_id = H5Topen2(group_id, OBJ_REF_GET_TYPE_TEST_TYPE_NAME, H5P_DEFAULT)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't open datatype for referencing\n");
            goto error;
        }
    }


    /* Create and check the group reference */
    if (H5Rcreate(&ref_array[0], file_id, "/", H5R_OBJECT, H5I_INVALID_HID) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group object reference\n");
        goto error;
    }

    if (H5Rget_obj_type2(file_id, H5R_OBJECT, &ref_array[0], &obj_type) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get object reference's object type\n");
        goto error;
    }

    if (H5O_TYPE_GROUP != obj_type) {
        H5_FAILED();
        HDprintf("    referenced object was not a group\n");
        goto error;
    }

    /* Create and check the datatype reference */
    if (H5Rcreate(&ref_array[1], group_id, OBJ_REF_GET_TYPE_TEST_TYPE_NAME, H5R_OBJECT, H5I_INVALID_HID) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype object reference\n");
        goto error;
    }

    if (H5Rget_obj_type2(file_id, H5R_OBJECT, &ref_array[1], &obj_type) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get object reference's object type\n");
        goto error;
    }

    if (H5O_TYPE_NAMED_DATATYPE != obj_type) {
        H5_FAILED();
        HDprintf("    referenced object was not a datatype\n");
        goto error;
    }

    /* Create and check the dataset reference */
    if (H5Rcreate(&ref_array[2], group_id, OBJ_REF_GET_TYPE_TEST_DSET_NAME, H5R_OBJECT, H5I_INVALID_HID) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset object reference\n");
        goto error;
    }

    if (H5Rget_obj_type2(file_id, H5R_OBJECT, &ref_array[2], &obj_type) < 0) {
        H5_FAILED();
        HDprintf("    couldn't get object reference's object type\n");
        goto error;
    }

    if (H5O_TYPE_DATASET != obj_type) {
        H5_FAILED();
        HDprintf("    referenced object was not a dataset\n");
        goto error;
    }

    /* TODO: Support for region references in this test */


    if (H5Sclose(space_id) < 0)
        TEST_ERROR
    if (H5Tclose(ref_dset_dtype) < 0)
        TEST_ERROR
    if (H5Tclose(ref_dtype_id) < 0)
        TEST_ERROR
    if (H5Dclose(ref_dset_id) < 0)
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
        H5Sclose(space_id);
        H5Tclose(ref_dset_dtype);
        H5Tclose(ref_dtype_id);
        H5Dclose(ref_dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_get_ref_name(void)
{
    TESTING("get ref. name")

    /* H5Rget_name */

    SKIPPED();

    return 0;
}

static int
test_get_region(void)
{
    TESTING("get region for region reference")

    /* H5Rget_region */

    SKIPPED();

    return 0;
}

static int
test_write_dataset_w_obj_refs(void)
{
    vol_test_obj_ref_t *ref_array = NULL;
    size_t              i, ref_array_size = 0;
    hid_t               file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t               container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t               dset_id = H5I_INVALID_HID, ref_dset_id = H5I_INVALID_HID;
    hid_t               ref_dtype_id = H5I_INVALID_HID;
    hid_t               ref_dset_dtype = H5I_INVALID_HID;
    hid_t               space_id = H5I_INVALID_HID;

    TESTING("write to a dataset w/ object reference type")

    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file\n");
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJ_REF_DATASET_WRITE_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group\n");
        goto error;
    }

    if ((space_id = generate_random_dataspace(OBJ_REF_DATASET_WRITE_TEST_SPACE_RANK, NULL)) < 0)
        TEST_ERROR

    if ((ref_dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    /* Create the dataset and datatype which will be referenced */
    if ((ref_dset_id = H5Dcreate2(group_id, OBJ_REF_DATASET_WRITE_TEST_REF_DSET_NAME, ref_dset_dtype,
            space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset for referencing\n");
        goto error;
    }

    if ((ref_dtype_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype\n");
        goto error;
    }

    if (H5Tcommit2(group_id, OBJ_REF_DATASET_WRITE_TEST_REF_TYPE_NAME, ref_dtype_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype for referencing\n");
        goto error;
    }

    {
        /* TODO: Temporary workaround for datatypes */
        if (H5Tclose(ref_dtype_id) < 0)
            TEST_ERROR

        if ((ref_dtype_id = H5Topen2(group_id, OBJ_REF_DATASET_WRITE_TEST_REF_TYPE_NAME, H5P_DEFAULT)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't open datatype for referencing\n");
            goto error;
        }
    }


    if ((dset_id = H5Dcreate2(group_id, OBJ_REF_DATASET_WRITE_TEST_DSET_NAME, H5T_STD_REF_OBJ,
            space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset\n");
        goto error;
    }

    for (i = 0, ref_array_size = 1; i < OBJ_REF_DATASET_WRITE_TEST_SPACE_RANK; i++)
        ref_array_size *= dims[i];

    if (NULL == (ref_array = (vol_test_obj_ref_t *) HDmalloc(ref_array_size * sizeof(*ref_array))))
        TEST_ERROR

    for (i = 0; i < dims[0]; i++) {
        /* Create a reference to either a group, datatype or dataset */
        switch (rand() % 3) {
            case 0:
                if (H5Rcreate(&ref_array[i], file_id, "/", H5R_OBJECT, H5I_INVALID_HID) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create reference\n");
                    goto error;
                }

                break;

            case 1:
                if (H5Rcreate(&ref_array[i], group_id, OBJ_REF_DATASET_WRITE_TEST_REF_TYPE_NAME, H5R_OBJECT, H5I_INVALID_HID) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create reference\n");
                    goto error;
                }

                break;

            case 2:
                if (H5Rcreate(&ref_array[i], group_id, OBJ_REF_DATASET_WRITE_TEST_REF_DSET_NAME, H5R_OBJECT, H5I_INVALID_HID) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create reference\n");
                    goto error;
                }

                break;

            default:
                TEST_ERROR
        }
    }

    if (H5Dwrite(dset_id, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, ref_array) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset\n");
        goto error;
    }

    if (ref_array) {
        HDfree(ref_array);
        ref_array = NULL;
    }

    if (H5Sclose(space_id) < 0)
        TEST_ERROR
    if (H5Tclose(ref_dset_dtype) < 0)
        TEST_ERROR
    if (H5Tclose(ref_dtype_id) < 0)
        TEST_ERROR
    if (H5Dclose(ref_dset_id) < 0)
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
        if (ref_array) HDfree(ref_array);
        H5Sclose(space_id);
        H5Tclose(ref_dset_dtype);
        H5Tclose(ref_dtype_id);
        H5Dclose(ref_dset_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_read_dataset_w_obj_refs(void)
{
    vol_test_obj_ref_t *ref_array = NULL;
    size_t              i, ref_array_size = 0;
    hid_t               file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t               container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t               dset_id = H5I_INVALID_HID, ref_dset_id = H5I_INVALID_HID;
    hid_t               ref_dtype_id = H5I_INVALID_HID;
    hid_t               ref_dset_dtype = H5I_INVALID_HID;
    hid_t               space_id = H5I_INVALID_HID;

    TESTING("read from a dataset w/ object reference type")

    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file\n");
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJ_REF_DATASET_READ_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group\n");
        goto error;
    }

    if ((space_id = generate_random_dataspace(OBJ_REF_DATASET_READ_TEST_SPACE_RANK, NULL)) < 0)
        TEST_ERROR

    if ((ref_dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    /* Create the dataset and datatype which will be referenced */
    if ((ref_dset_id = H5Dcreate2(group_id, OBJ_REF_DATASET_READ_TEST_REF_DSET_NAME, ref_dset_dtype,
            space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset for referencing\n");
        goto error;
    }

    if ((ref_dtype_id = generate_random_datatype(H5T_NO_CLASS)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype\n");
        goto error;
    }

    if (H5Tcommit2(group_id, OBJ_REF_DATASET_READ_TEST_REF_TYPE_NAME, ref_dtype_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create datatype for referencing\n");
        goto error;
    }

    {
        /* TODO: Temporary workaround for datatypes */
        if (H5Tclose(ref_dtype_id) < 0)
            TEST_ERROR

        if ((ref_dtype_id = H5Topen2(group_id, OBJ_REF_DATASET_READ_TEST_REF_TYPE_NAME, H5P_DEFAULT)) < 0) {
            H5_FAILED();
            HDprintf("    couldn't open datatype for referencing\n");
            goto error;
        }
    }


    if ((dset_id = H5Dcreate2(group_id, OBJ_REF_DATASET_READ_TEST_DSET_NAME, H5T_STD_REF_OBJ,
            space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset\n");
        goto error;
    }

    for (i = 0, ref_array_size = 1; i < OBJ_REF_DATASET_READ_TEST_SPACE_RANK; i++)
        ref_array_size *= dims[i];

    if (NULL == (ref_array = (vol_test_obj_ref_t *) HDmalloc(ref_array_size * sizeof(*ref_array))))
        TEST_ERROR

    for (i = 0; i < dims[0]; i++) {
        /* Create a reference to either a group, datatype or dataset */
        switch (rand() % 3) {
            case 0:
                if (H5Rcreate(&ref_array[i], file_id, "/", H5R_OBJECT, H5I_INVALID_HID) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create reference\n");
                    goto error;
                }

                break;

            case 1:
                if (H5Rcreate(&ref_array[i], group_id, OBJ_REF_DATASET_READ_TEST_REF_TYPE_NAME, H5R_OBJECT, H5I_INVALID_HID) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create reference\n");
                    goto error;
                }

                break;

            case 2:
                if (H5Rcreate(&ref_array[i], group_id, OBJ_REF_DATASET_READ_TEST_REF_DSET_NAME, H5R_OBJECT, H5I_INVALID_HID) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create reference\n");
                    goto error;
                }

                break;

            default:
                TEST_ERROR
        }
    }

    if (H5Dwrite(dset_id, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, ref_array) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset\n");
        goto error;
    }

    /* Now read from the dataset */
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR

    if ((dset_id = H5Dopen2(group_id, OBJ_REF_DATASET_READ_TEST_DSET_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open dataset\n");
        goto error;
    }

    HDmemset(ref_array, 0, ref_array_size * sizeof(*ref_array));

    if (H5Dread(dset_id, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, ref_array) < 0) {
        H5_FAILED();
        HDprintf("    couldn't read from dataset\n");
        goto error;
    }

    for (i = 0; i < dims[0]; i++) {
        /* Check the reference type */
        if (H5R_OBJECT != ref_array[i].ref_type) {
            H5_FAILED();
            HDprintf("    ref type was not H5R_OBJECT\n");
            goto error;
        }

        /* Check the object type referenced */
        if (   H5I_FILE != ref_array[i].ref_obj_type
            && H5I_GROUP != ref_array[i].ref_obj_type
            && H5I_DATATYPE != ref_array[i].ref_obj_type
            && H5I_DATASET != ref_array[i].ref_obj_type
           ) {
            H5_FAILED();
            HDprintf("    ref object type mismatch\n");
            goto error;
        }

        /* Check the URI of the referenced object according to
         * the server spec where each URI is prefixed as
         * 'X-', where X is a character denoting the type
         * of object */
        if (   (ref_array[i].ref_obj_URI[1] != '-')
            || (ref_array[i].ref_obj_URI[0] != 'g'
            &&  ref_array[i].ref_obj_URI[0] != 't'
            &&  ref_array[i].ref_obj_URI[0] != 'd')
           ) {
            H5_FAILED();
            HDprintf("    ref URI mismatch\n");
            goto error;
        }
    }

    if (ref_array) {
        HDfree(ref_array);
        ref_array = NULL;
    }

    if (H5Sclose(space_id) < 0)
        TEST_ERROR
    if (H5Tclose(ref_dset_dtype) < 0)
        TEST_ERROR
    if (H5Tclose(ref_dtype_id) < 0)
        TEST_ERROR
    if (H5Dclose(ref_dset_id) < 0)
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
        if (ref_array) HDfree(ref_array);
        H5Sclose(space_id);
        H5Tclose(ref_dset_dtype);
        H5Tclose(ref_dtype_id);
        H5Dclose(ref_dset_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_write_dataset_w_obj_refs_empty_data(void)
{
    vol_test_obj_ref_t *ref_array = NULL;
    size_t              i, ref_array_size = 0;
    hid_t               file_id = H5I_INVALID_HID, fapl_id = H5I_INVALID_HID;
    hid_t               container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t               dset_id = H5I_INVALID_HID;
    hid_t               space_id = H5I_INVALID_HID;

    TESTING("write to a dataset w/ object reference type and some empty data")

    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, fapl_id)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file\n");
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, OBJECT_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, OBJ_REF_DATASET_EMPTY_WRITE_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group");
        goto error;
    }

    if ((space_id = generate_random_dataspace(OBJ_REF_DATASET_EMPTY_WRITE_TEST_SPACE_RANK, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, OBJ_REF_DATASET_EMPTY_WRITE_TEST_DSET_NAME, H5T_STD_REF_OBJ,
            space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dataset\n");
        goto error;
    }

    for (i = 0, ref_array_size = 1; i < OBJ_REF_DATASET_EMPTY_WRITE_TEST_SPACE_RANK; i++)
        ref_array_size *= dims[i];

    if (NULL == (ref_array = (vol_test_obj_ref_t *) HDcalloc(1, ref_array_size * sizeof(*ref_array))))
        TEST_ERROR

    for (i = 0; i < dims[0]; i++) {
        switch (rand() % 2) {
            case 0:
                if (H5Rcreate(&ref_array[i], file_id, "/", H5R_OBJECT, H5I_INVALID_HID) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't create reference\n");
                    goto error;
                }

                break;

            case 1:
                break;

            default:
                TEST_ERROR
        }
    }

    if (H5Dwrite(dset_id, H5T_STD_REF_OBJ, H5S_ALL, H5S_ALL, H5P_DEFAULT, ref_array) < 0) {
        H5_FAILED();
        HDprintf("    couldn't write to dataset\n");
        goto error;
    }

    if (ref_array) {
        HDfree(ref_array);
        ref_array = NULL;
    }

    if (H5Sclose(space_id) < 0)
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
        H5Sclose(space_id);
        H5Dclose(dset_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Pclose(fapl_id);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}
#endif

/*
 * H5Ovisit callback to simply iterate recursively through all of the objects in a
 * group and check to make sure their names match what is expected.
 */
static herr_t
object_visit_callback(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data)
{
    size_t *i = (size_t *) op_data;
    size_t  counter_val = *((size_t *) op_data);
    herr_t  ret_val = 0;

    UNUSED(o_id);
    UNUSED(object_info);

    if (!HDstrcmp(name, ".") &&
            (counter_val == 0 || counter_val == 4 || counter_val == 8 || counter_val == 12)) {
        goto done;
    }
    if (!HDstrcmp(name, OBJECT_VISIT_TEST_GROUP_NAME) &&
            (counter_val == 2 || counter_val == 6 || counter_val == 9 || counter_val == 15)) {
        goto done;
    }
    else if (!HDstrcmp(name, OBJECT_VISIT_TEST_DSET_NAME) &&
            (counter_val == 1 || counter_val == 7 || counter_val == 10 || counter_val == 14)) {
        goto done;
    }
    else if (!HDstrcmp(name, OBJECT_VISIT_TEST_TYPE_NAME) &&
            (counter_val == 3 || counter_val == 5 || counter_val == 11 || counter_val == 13)) {
        goto done;
    }

    HDprintf("    object name '%s' didn't match known names or came in an incorrect order\n", name);

    ret_val = -1;

done:
    (*i)++;

    return ret_val;
}

/*
 * H5Ovisit callback to simply iterate through all of the objects in a given
 * group.
 */
static herr_t
object_visit_callback2(hid_t o_id, const char *name, const H5O_info_t *object_info, void *op_data)
{
    UNUSED(o_id);
    UNUSED(name);
    UNUSED(object_info);
    UNUSED(op_data);

    return 0;
}

int
vol_object_test(void)
{
    size_t i;
    int    nerrors;

    HDprintf("**********************************************\n");
    HDprintf("*                                            *\n");
    HDprintf("*              VOL Object Tests              *\n");
    HDprintf("*                                            *\n");
    HDprintf("**********************************************\n\n");

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(object_tests); i++) {
        nerrors += (*object_tests[i])() ? 1 : 0;
    }

    HDprintf("\n");

    return nerrors;
}
