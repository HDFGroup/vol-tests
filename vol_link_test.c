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

#include "vol_link_test.h"

static int test_create_hard_link(void);
static int test_create_hard_link_same_loc(void);
static int test_create_hard_link_invalid_params(void);
static int test_create_soft_link_existing_relative(void);
static int test_create_soft_link_existing_absolute(void);
static int test_create_soft_link_dangling_relative(void);
static int test_create_soft_link_dangling_absolute(void);
static int test_create_soft_link_invalid_params(void);
static int test_create_external_link(void);
static int test_create_external_link_dangling(void);
static int test_create_external_link_invalid_params(void);
static int test_create_user_defined_link(void);
static int test_create_user_defined_link_invalid_params(void);
static int test_delete_link(void);
static int test_delete_link_invalid_params(void);
static int test_copy_link(void);
static int test_copy_link_invalid_params(void);
static int test_move_link(void);
static int test_move_link_invalid_params(void);
static int test_get_link_val(void);
static int test_get_link_val_invalid_params(void);
static int test_get_link_info(void);
static int test_get_link_info_invalid_params(void);
static int test_get_link_name(void);
static int test_get_link_name_invalid_params(void);
static int test_link_iterate(void);
static int test_link_iterate_invalid_params(void);
static int test_link_iterate_0_links(void);
static int test_link_visit(void);
static int test_link_visit_cycles(void);
static int test_link_visit_invalid_params(void);
static int test_link_visit_0_links(void);

static herr_t link_iter_callback1(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data);
static herr_t link_iter_callback2(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data);
static herr_t link_iter_callback3(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data);

static herr_t link_visit_callback1(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data);
static herr_t link_visit_callback2(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data);
static herr_t link_visit_callback3(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data);

/*
 * The array of link tests to be performed.
 */
static int (*link_tests[])(void) = {
        test_create_hard_link,
        test_create_hard_link_same_loc,
        test_create_hard_link_invalid_params,
        test_create_soft_link_existing_relative,
        test_create_soft_link_existing_absolute,
        test_create_soft_link_dangling_relative,
        test_create_soft_link_dangling_absolute,
        test_create_soft_link_invalid_params,
        test_create_external_link,
        test_create_external_link_dangling,
        test_create_external_link_invalid_params,
        test_create_user_defined_link,
        test_create_user_defined_link_invalid_params,
        test_delete_link,
        test_delete_link_invalid_params,
        test_copy_link,
        test_copy_link_invalid_params,
        test_move_link,
        test_move_link_invalid_params,
        test_get_link_val,
        test_get_link_val_invalid_params,
        test_get_link_info,
        test_get_link_info_invalid_params,
        test_get_link_name,
        test_get_link_name_invalid_params,
        test_link_iterate,
        test_link_iterate_invalid_params,
        test_link_iterate_0_links,
        test_link_visit,
        test_link_visit_cycles,
        test_link_visit_invalid_params,
        test_link_visit_0_links,
};

/*
 * A test to check that a hard link can be created
 * using H5Lcreate_hard.
 */
static int
test_create_hard_link(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;

    TESTING("hard link creation")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, HARD_LINK_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", HARD_LINK_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_hard(file_id, "/", group_id, HARD_LINK_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create hard link '%s'\n", HARD_LINK_TEST_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, HARD_LINK_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", HARD_LINK_TEST_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that behavior is correct when using
 * the H5L_SAME_LOC macro for H5Lcreate_hard().
 */
static int
test_create_hard_link_same_loc(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;

    TESTING("hard link creation with H5L_SAME_LOC")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, H5L_SAME_LOC_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", H5L_SAME_LOC_TEST_GROUP_NAME);
        goto error;
    }

#if 0 /* Library functionality for this part of the test is broken */
    if (H5Lcreate_hard(H5L_SAME_LOC, ".", group_id, H5L_SAME_LOC_TEST_LINK_NAME1, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first link '%s'\n", H5L_SAME_LOC_TEST_LINK_NAME1);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, H5L_SAME_LOC_TEST_LINK_NAME1, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link exists\n");
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }
#endif

    if (H5Lcreate_hard(group_id, ".", H5L_SAME_LOC, H5L_SAME_LOC_TEST_LINK_NAME2, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second link '%s'\n", H5L_SAME_LOC_TEST_LINK_NAME2);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, H5L_SAME_LOC_TEST_LINK_NAME2, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", H5L_SAME_LOC_TEST_LINK_NAME2);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a hard link can't be created when
 * H5Lcreate_hard is passed invalid parameters.
 */
static int
test_create_hard_link_invalid_params(void)
{
    herr_t err_ret = -1;
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;

    TESTING_MULTIPART("hard link creation with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, HARD_LINK_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", HARD_LINK_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lcreate_hard_invalid_cur_loc_id) {
            TESTING_2("H5Lcreate_hard with an invalid cur_loc_id")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(H5I_INVALID_HID, "/", group_id, HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with an invalid cur_loc_id!\n");
                PART_ERROR(H5Lcreate_hard_invalid_cur_loc_id);
            }

            PASSED();
        } PART_END(H5Lcreate_hard_invalid_cur_loc_id);

        PART_BEGIN(H5Lcreate_hard_invalid_cur_name) {
            TESTING_2("H5Lcreate_hard with an invalid cur_name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(file_id, NULL, group_id, HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with a NULL cur_name!\n");
                PART_ERROR(H5Lcreate_hard_invalid_cur_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(file_id, "", group_id, HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with an invalid cur_name of ''!\n");
                PART_ERROR(H5Lcreate_hard_invalid_cur_name);
            }

            PASSED();
        } PART_END(H5Lcreate_hard_invalid_cur_name);

        PART_BEGIN(H5Lcreate_hard_invalid_new_loc_id) {
            TESTING_2("H5Lcreate_hard with an invalid new_loc_id")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(file_id, "/", H5I_INVALID_HID, HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with an invalid new_loc_id!\n");
                PART_ERROR(H5Lcreate_hard_invalid_new_loc_id);
            }

            PASSED();
        } PART_END(H5Lcreate_hard_invalid_new_loc_id);

        PART_BEGIN(H5Lcreate_hard_invalid_new_name) {
            TESTING_2("H5Lcreate_hard with an invalid new_name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(file_id, "/", group_id, NULL, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with a NULL new_name!\n");
                PART_ERROR(H5Lcreate_hard_invalid_new_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(file_id, "/", group_id, "", H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with an invalid new_name of ''!\n");
                PART_ERROR(H5Lcreate_hard_invalid_new_name);
            }

            PASSED();
        } PART_END(H5Lcreate_hard_invalid_new_name);

        PART_BEGIN(H5Lcreate_hard_invalid_lcpl) {
            TESTING_2("H5Lcreate_hard with an invalid LCPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(file_id, "/", group_id, HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with an invalid LCPL!\n");
                PART_ERROR(H5Lcreate_hard_invalid_lcpl);
            }

            PASSED();
        } PART_END(H5Lcreate_hard_invalid_lcpl);

        PART_BEGIN(H5Lcreate_hard_invalid_lapl) {
            TESTING_2("H5Lcreate_hard with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_hard(file_id, "/", group_id, HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created hard link with an invalid LAPL!\n");
                PART_ERROR(H5Lcreate_hard_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lcreate_hard_invalid_lapl);

        PART_BEGIN(H5Lcreate_hard_invalid_existence) {
            TESTING_2("invalid link existence")

            /* Verify the link hasn't been created */
            if ((link_exists = H5Lexists(group_id, HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' exists\n", HARD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_hard_invalid_existence);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    link existed!\n");
                PART_ERROR(H5Lcreate_hard_invalid_existence);
            }

            PASSED();
        } PART_END(H5Lcreate_hard_invalid_existence);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a soft link, which points to an
 * existing object with a relative path, can be created.
 */
static int
test_create_soft_link_existing_relative(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  object_id = H5I_INVALID_HID;

    TESTING("soft link creation to existing object by relative path")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, SOFT_LINK_EXISTING_RELATIVE_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", SOFT_LINK_EXISTING_RELATIVE_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((object_id = H5Gcreate2(group_id, SOFT_LINK_EXISTING_RELATIVE_TEST_OBJECT_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to create object '%s' for soft link's target\n", SOFT_LINK_EXISTING_RELATIVE_TEST_OBJECT_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
        TEST_ERROR

    if (H5Lcreate_soft(SOFT_LINK_EXISTING_RELATIVE_TEST_OBJECT_NAME, group_id,
            SOFT_LINK_EXISTING_RELATIVE_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", SOFT_LINK_EXISTING_RELATIVE_TEST_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, SOFT_LINK_EXISTING_RELATIVE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", SOFT_LINK_EXISTING_RELATIVE_TEST_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    /*
     * XXX: If H5Oopen is available, use that.
     */
    if ((object_id = H5Gopen2(group_id, SOFT_LINK_EXISTING_RELATIVE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open object '%s' through the soft link\n", SOFT_LINK_EXISTING_RELATIVE_TEST_OBJECT_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
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
        H5Gclose(object_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a soft link, which points to an
 * existing object using an absolute path, can be created.
 */
static int
test_create_soft_link_existing_absolute(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID, root_id = H5I_INVALID_HID;

    TESTING("soft link creation to existing object by absolute path")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, SOFT_LINK_EXISTING_ABSOLUTE_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", SOFT_LINK_EXISTING_ABSOLUTE_TEST_SUBGROUP_NAME);
        goto error;
    }

    if (H5Lcreate_soft("/", group_id, SOFT_LINK_EXISTING_ABSOLUTE_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", SOFT_LINK_EXISTING_ABSOLUTE_TEST_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, SOFT_LINK_EXISTING_ABSOLUTE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", SOFT_LINK_EXISTING_ABSOLUTE_TEST_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    if ((root_id = H5Gopen2(group_id, SOFT_LINK_EXISTING_ABSOLUTE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open object pointed to by soft link '%s'\n", SOFT_LINK_EXISTING_ABSOLUTE_TEST_LINK_NAME);
        goto error;
    }

    if (H5Gclose(root_id) < 0)
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
        H5Gclose(root_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a soft link, which points to
 * an object that doesn't exist by using a relative
 * path, can be created.
 */
static int
test_create_soft_link_dangling_relative(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  object_id = H5I_INVALID_HID;

    TESTING("dangling soft link creation to object by relative path")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, SOFT_LINK_DANGLING_RELATIVE_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", SOFT_LINK_DANGLING_RELATIVE_TEST_SUBGROUP_NAME);
        goto error;
    }

    if (H5Lcreate_soft(SOFT_LINK_DANGLING_RELATIVE_TEST_OBJECT_NAME, group_id,
            SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    /*
     * XXX: If H5Oopen is available, use that.
     */
    H5E_BEGIN_TRY {
        object_id = H5Gopen2(group_id, SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME, H5P_DEFAULT);
    } H5E_END_TRY;

    if (object_id >= 0) {
        H5_FAILED();
        HDprintf("    opened target of dangling link '%s'!\n", SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME);
        H5Gclose(object_id);
        goto error;
    }

    if ((object_id = H5Gcreate2(group_id, SOFT_LINK_DANGLING_RELATIVE_TEST_OBJECT_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to create object '%s' for soft link's target\n", SOFT_LINK_DANGLING_RELATIVE_TEST_OBJECT_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
        TEST_ERROR

    /*
     * XXX: If H5Oopen is available, use that.
     */
    if ((object_id = H5Gopen2(group_id, SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to open object pointed to by soft link '%s'\n", SOFT_LINK_DANGLING_RELATIVE_TEST_LINK_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
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
        H5Gclose(object_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a soft link, which points to an
 * object that doesn't exist by using an absolute path,
 * can be created.
 */
static int
test_create_soft_link_dangling_absolute(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  object_id = H5I_INVALID_HID;

    TESTING("dangling soft link creation to object by absolute path")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, SOFT_LINK_DANGLING_ABSOLUTE_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", SOFT_LINK_DANGLING_ABSOLUTE_TEST_SUBGROUP_NAME);
        goto error;
    }

    if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" SOFT_LINK_DANGLING_ABSOLUTE_TEST_SUBGROUP_NAME "/" SOFT_LINK_DANGLING_ABSOLUTE_TEST_OBJECT_NAME,
            group_id, SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    /*
     * XXX: If H5Oopen is available, use that.
     */
    H5E_BEGIN_TRY {
        object_id = H5Gopen2(group_id, SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME, H5P_DEFAULT);
    } H5E_END_TRY;

    if (object_id >= 0) {
        H5_FAILED();
        HDprintf("    opened target of dangling link '%s'!\n", SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME);
        H5Gclose(object_id);
        goto error;
    }

    if ((object_id = H5Gcreate2(group_id, SOFT_LINK_DANGLING_ABSOLUTE_TEST_OBJECT_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to create object '%s' for soft link's target\n", SOFT_LINK_DANGLING_ABSOLUTE_TEST_OBJECT_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
        TEST_ERROR

    /*
     * XXX: If H5Oopen is available, use that.
     */
    if ((object_id = H5Gopen2(group_id, SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to open object pointed to by soft link '%s'\n", SOFT_LINK_DANGLING_ABSOLUTE_TEST_LINK_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
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
        H5Gclose(object_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a soft link can't be created
 * when H5Lcreate_soft is passed invalid parameters.
 */
static int
test_create_soft_link_invalid_params(void)
{
    herr_t err_ret = -1;
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;

    TESTING_MULTIPART("soft link creation with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, SOFT_LINK_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container sub-group '%s'\n", SOFT_LINK_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lcreate_soft_invalid_link_target) {
            TESTING_2("H5Lcreate_soft with an invalid link target")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_soft(NULL, group_id, SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created soft link '%s' with an invalid link target!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_link_target);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_soft("", group_id, SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created soft link '%s' with an invalid link target!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_link_target);
            }

            PASSED();
        } PART_END(H5Lcreate_soft_invalid_link_target);

        PART_BEGIN(H5Lcreate_soft_invalid_link_loc_id) {
            TESTING_2("H5Lcreate_soft with an invalid link_loc_id")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_soft("/", H5I_INVALID_HID, SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created soft link '%s' with an invalid link_loc_id!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_link_loc_id);
            }

            PASSED();
        } PART_END(H5Lcreate_soft_invalid_link_loc_id);

        PART_BEGIN(H5Lcreate_soft_invalid_link_name) {
            TESTING_2("H5Lcreate_soft with an invalid link name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_soft("/", group_id, NULL, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created soft link '%s' with a NULL link name!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_link_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_soft("/", group_id, "", H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created soft link '%s' with an invalid link name of ''!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_link_name);
            }

            PASSED();
        } PART_END(H5Lcreate_soft_invalid_link_name);

        PART_BEGIN(H5Lcreate_soft_invalid_lcpl) {
            TESTING_2("H5Lcreate_soft with an invalid LCPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_soft("/", group_id, SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created soft link '%s' with an invalid LCPL!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_lcpl);
            }

            PASSED();
        } PART_END(H5Lcreate_soft_invalid_lcpl);

        PART_BEGIN(H5Lcreate_soft_invalid_lapl) {
            TESTING_2("H5Lcreate_soft with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_soft("/", group_id, SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created soft link '%s' with an invalid LAPL!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lcreate_soft_invalid_lapl);

        PART_BEGIN(H5Lcreate_soft_invalid_existence) {
            TESTING_2("invalid link existence")

            /* Verify the link hasn't been created */
            if ((link_exists = H5Lexists(group_id, SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' exists\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_existence);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    link '%s' existed!\n", SOFT_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_soft_invalid_existence);
            }

            PASSED();
        } PART_END(H5Lcreate_soft_invalid_existence);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that an external link can be created
 * using H5Lcreate_external.
 */
static int
test_create_external_link(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  root_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING("external link creation to existing object")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, EXTERNAL_LINK_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", EXTERNAL_LINK_TEST_SUBGROUP_NAME);
        goto error;
    }

    if (H5Lcreate_external(ext_link_filename, "/", group_id,
            EXTERNAL_LINK_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create external link '%s'\n", EXTERNAL_LINK_TEST_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, EXTERNAL_LINK_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", EXTERNAL_LINK_TEST_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    if ((root_id = H5Gopen2(group_id, EXTERNAL_LINK_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open root group of other file using external link '%s'\n", EXTERNAL_LINK_TEST_LINK_NAME);
        goto error;
    }

    if (H5Gclose(root_id) < 0)
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
        H5Gclose(root_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that an external link, which points to an
 * object that doesn't exist by using an absolute path, can
 * be created.
 */
static int
test_create_external_link_dangling(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID, ext_file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  object_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING("dangling external link creation")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((ext_file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, EXTERNAL_LINK_TEST_DANGLING_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", EXTERNAL_LINK_TEST_DANGLING_SUBGROUP_NAME);
        goto error;
    }

    if (H5Lcreate_external(ext_link_filename, "/" EXTERNAL_LINK_TEST_DANGLING_OBJECT_NAME, group_id,
            EXTERNAL_LINK_TEST_DANGLING_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create dangling external link '%s'\n", EXTERNAL_LINK_TEST_DANGLING_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, EXTERNAL_LINK_TEST_DANGLING_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", EXTERNAL_LINK_TEST_DANGLING_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    /*
     * XXX: If H5Oopen is available, use that.
     */
    H5E_BEGIN_TRY {
        object_id = H5Gopen2(group_id, EXTERNAL_LINK_TEST_DANGLING_LINK_NAME, H5P_DEFAULT);
    } H5E_END_TRY;

    if (object_id >= 0) {
        H5_FAILED();
        HDprintf("    opened non-existent object in other file using dangling external link '%s'!\n", EXTERNAL_LINK_TEST_DANGLING_LINK_NAME);
        H5Gclose(object_id);
        goto error;
    }

    if ((object_id = H5Gcreate2(ext_file_id, EXTERNAL_LINK_TEST_DANGLING_OBJECT_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to create object '%s' for external link's target\n", EXTERNAL_LINK_TEST_DANGLING_OBJECT_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
        TEST_ERROR

    /*
     * XXX: If H5Oopen is available, use that.
     */
    if ((object_id = H5Gopen2(group_id, EXTERNAL_LINK_TEST_DANGLING_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to open object pointed to by external link '%s'\n", EXTERNAL_LINK_TEST_DANGLING_LINK_NAME);
        goto error;
    }

    if (H5Gclose(object_id) < 0)
        TEST_ERROR
    if (H5Gclose(group_id) < 0)
        TEST_ERROR
    if (H5Gclose(container_group) < 0)
        TEST_ERROR
    if (H5Fclose(file_id) < 0)
        TEST_ERROR
    if (H5Fclose(ext_file_id) < 0)
        TEST_ERROR

    PASSED();

    return 0;

error:
    H5E_BEGIN_TRY {
        H5Gclose(object_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
        H5Fclose(ext_file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that an external link can't be created
 * when H5Lcreate_external is passed invalid parameters.
 */
static int
test_create_external_link_invalid_params(void)
{
    herr_t err_ret = -1;
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("H5Lcreate_external with invalid parameters");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_INVALID_PARAMS_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, EXTERNAL_LINK_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lcreate_external_invalid_file_name) {
            TESTING_2("H5Lcreate_external with an invalid file name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(NULL, "/", group_id, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using a NULL file name!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_file_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external("", "/", group_id, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using an invalid file name of ''!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_file_name);
            }

            PASSED();
        } PART_END(H5Lcreate_external_invalid_file_name);

        PART_BEGIN(H5Lcreate_external_invalid_ext_obj_name) {
            TESTING_2("H5Lcreate_external with an invalid external object name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(ext_link_filename, NULL, group_id, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using a NULL external object name!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_ext_obj_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(ext_link_filename, "", group_id, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using an invalid external object name of ''!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_ext_obj_name);
            }

            PASSED();
        } PART_END(H5Lcreate_external_invalid_ext_obj_name);

        PART_BEGIN(H5Lcreate_external_invalid_link_loc_id) {
            TESTING_2("H5Lcreate_external with an invalid link_loc_id")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(ext_link_filename, "/", H5I_INVALID_HID, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using an invalid link_loc_id!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_link_loc_id);
            }

            PASSED();
        } PART_END(H5Lcreate_external_invalid_link_loc_id);

        PART_BEGIN(H5Lcreate_external_invalid_link_name) {
            TESTING_2("H5Lcreate_external with an invalid link name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(ext_link_filename, "/", group_id, NULL, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using a NULL link_loc_id!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_link_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(ext_link_filename, "/", group_id, "", H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using an invalid link name of ''!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_link_name);
            }

            PASSED();
        } PART_END(H5Lcreate_external_invalid_link_name);

        PART_BEGIN(H5Lcreate_external_invalid_lcpl) {
            TESTING_2("H5Lcreate_external with an invalid LCPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(ext_link_filename, "/", group_id, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using an invalid LCPL!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_lcpl);
            }

            PASSED();
        } PART_END(H5Lcreate_external_invalid_lcpl);

        PART_BEGIN(H5Lcreate_external_invalid_lapl) {
            TESTING_2("H5Lcreate_external with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_external(ext_link_filename, "/", group_id, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created external link '%s' using an invalid LAPL!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lcreate_external_invalid_lapl);

        PART_BEGIN(H5Lcreate_external_invalid_existence) {
            TESTING_2("invalid link existence")

            /* Verify the link hasn't been created */
            if ((link_exists = H5Lexists(group_id, EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' exists\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_existence);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    link '%s' existed!\n", EXTERNAL_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_external_invalid_existence);
            }

            PASSED();
        } PART_END(H5Lcreate_external_invalid_existence);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a user-defined link can be created.
 */
static int
test_create_user_defined_link(void)
{
    ssize_t udata_size;
    htri_t  link_exists;
    hid_t   file_id = H5I_INVALID_HID;
    hid_t   container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char    udata[UD_LINK_TEST_UDATA_MAX_SIZE];

    TESTING("user-defined link creation")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, UD_LINK_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", UD_LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((udata_size = HDsnprintf(udata, UD_LINK_TEST_UDATA_MAX_SIZE, "udata")) < 0)
        TEST_ERROR

    if (H5Lcreate_ud(group_id, UD_LINK_TEST_LINK_NAME, H5L_TYPE_EXTERNAL, udata, (size_t) udata_size,
            H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create user-defined link '%s'\n", UD_LINK_TEST_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, UD_LINK_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", UD_LINK_TEST_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link '%s' didn't exist!\n", UD_LINK_TEST_LINK_NAME);
        goto error;
    }

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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_create_user_defined_link_invalid_params(void)
{
    ssize_t udata_size;
    htri_t  link_exists;
    herr_t  err_ret = -1;
    hid_t   file_id = H5I_INVALID_HID;
    hid_t   container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char    udata[UD_LINK_INVALID_PARAMS_TEST_UDATA_MAX_SIZE];

    TESTING_MULTIPART("H5Lcreate_ud with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, UD_LINK_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", UD_LINK_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if ((udata_size = HDsnprintf(udata, UD_LINK_INVALID_PARAMS_TEST_UDATA_MAX_SIZE, "udata")) < 0)
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lcreate_ud_invalid_link_loc_id) {
            TESTING_2("H5Lcreate_ud with an invalid link location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_ud(H5I_INVALID_HID, UD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5L_TYPE_EXTERNAL, udata, (size_t) udata_size,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created user-defined link '%s' with an invalid link location ID!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_link_loc_id);
            }

            PASSED();
        } PART_END(H5Lcreate_ud_invalid_link_loc_id);

        PART_BEGIN(H5Lcreate_ud_invalid_link_name) {
            TESTING_2("H5Lcreate_ud with an invalid link name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_ud(group_id, NULL, H5L_TYPE_EXTERNAL, udata, (size_t) udata_size,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created user-defined link '%s' with a NULL link name!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_link_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_ud(group_id, "", H5L_TYPE_EXTERNAL, udata, (size_t) udata_size,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created user-defined link '%s' with an invalid link name of ''!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_link_name);
            }

            PASSED();
        } PART_END(H5Lcreate_ud_invalid_link_name);

        PART_BEGIN(H5Lcreate_ud_invalid_link_type) {
            TESTING_2("H5Lcreate_ud with an invalid link type")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_ud(group_id, UD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5L_TYPE_HARD, udata, (size_t) udata_size,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created user-defined link '%s' with an invalid link type!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_link_type);
            }

            PASSED();
        } PART_END(H5Lcreate_ud_invalid_link_type);

        PART_BEGIN(H5Lcreate_ud_invalid_udata_pointer) {
            TESTING_2("H5Lcreate_ud with an invalid udata pointer")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_ud(group_id, UD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5L_TYPE_EXTERNAL, NULL, (size_t) udata_size,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created user-defined link '%s' with an invalid udata pointer!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_udata_pointer);
            }

            PASSED();
        } PART_END(H5Lcreate_ud_invalid_udata_pointer);

        PART_BEGIN(H5Lcreate_ud_invalid_lcpl) {
            TESTING_2("H5Lcreate_ud with an invalid LCPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_ud(group_id, UD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5L_TYPE_EXTERNAL, udata, (size_t) udata_size,
                        H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created user-defined link '%s' with an invalid LCPL!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_lcpl);
            }

            PASSED();
        } PART_END(H5Lcreate_ud_invalid_lcpl);

        PART_BEGIN(H5Lcreate_ud_invalid_lapl) {
            TESTING_2("H5Lcreate_ud with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcreate_ud(group_id, UD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5L_TYPE_EXTERNAL, udata, (size_t) udata_size,
                        H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    created user-defined link '%s' with an invalid LAPL!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lcreate_ud_invalid_lapl);

        PART_BEGIN(H5Lcreate_ud_invalid_existence) {
            TESTING_2("invalid link existence")

            /* Verify the link hasn't been created */
            if ((link_exists = H5Lexists(group_id, UD_LINK_INVALID_PARAMS_TEST_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' exists\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_existence);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    link '%s' existed!\n", UD_LINK_INVALID_PARAMS_TEST_LINK_NAME);
                PART_ERROR(H5Lcreate_ud_invalid_existence);
            }

            PASSED();
        } PART_END(H5Lcreate_ud_invalid_existence);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a link can be deleted
 * using H5Ldelete and H5Ldelete_by_idx.
 */
static int
test_delete_link(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  gcpl_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link deletion");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create GCPL for link creation order tracking\n");
        goto error;
    }

    if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED) < 0) {
        H5_FAILED();
        HDprintf("    couldn't set link creation order tracking\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_DELETE_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, gcpl_id, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_DELETE_TEST_SUBGROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Ldelete_hard) {
            TESTING_2("H5Ldelete on hard link")

            if (H5Lcreate_hard(group_id, ".", group_id, LINK_DELETE_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create first hard link '%s'\n", LINK_DELETE_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Ldelete_hard);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if first hard link '%s' exists\n", LINK_DELETE_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Ldelete_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    first hard link did not exist\n");
                PART_ERROR(H5Ldelete_hard);
            }

            if (H5Ldelete(group_id, LINK_DELETE_TEST_HARD_LINK_NAME, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't delete hard link '%s' using H5Ldelete\n", LINK_DELETE_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Ldelete_hard);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if first hard link '%s' exists\n", LINK_DELETE_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Ldelete_hard);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    first hard link exists!\n");
                PART_ERROR(H5Ldelete_hard);
            }

            PASSED();
        } PART_END(H5Ldelete_hard);

        PART_BEGIN(H5Ldelete_soft) {
            TESTING_2("H5Ldelete on soft link")

            if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" LINK_DELETE_TEST_SUBGROUP_NAME,
                    group_id, LINK_DELETE_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create first soft link '%s'\n", LINK_DELETE_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Ldelete_soft);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if first soft link '%s' exists\n", LINK_DELETE_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Ldelete_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    first soft link did not exist\n");
                PART_ERROR(H5Ldelete_soft);
            }

            if (H5Ldelete(group_id, LINK_DELETE_TEST_SOFT_LINK_NAME, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't delete soft link '%s' using H5Ldelete\n", LINK_DELETE_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Ldelete_soft);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if first soft link '%s' exists\n", LINK_DELETE_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Ldelete_soft);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    first soft link exists!\n");
                PART_ERROR(H5Ldelete_soft);
            }

            PASSED();
        } PART_END(H5Ldelete_soft);

        PART_BEGIN(H5Ldelete_external) {
            TESTING_2("H5Ldelete on external link")

            if (H5Lcreate_external(ext_link_filename, "/", group_id, LINK_DELETE_TEST_EXTERNAL_LINK_NAME,
                    H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create first external link '%s'\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME);
                PART_ERROR(H5Ldelete_external);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_EXTERNAL_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if first external link '%s' exists\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME);
                PART_ERROR(H5Ldelete_external);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    first external link did not exist\n");
                PART_ERROR(H5Ldelete_external);
            }

            if (H5Ldelete(group_id, LINK_DELETE_TEST_EXTERNAL_LINK_NAME, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't delete external link '%s' using H5Ldelete\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME);
                PART_ERROR(H5Ldelete_external);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_EXTERNAL_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if first external link '%s' exists\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME);
                PART_ERROR(H5Ldelete_external);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    first external link exists!\n");
                PART_ERROR(H5Ldelete_external);
            }

            PASSED();
        } PART_END(H5Ldelete_external);

        PART_BEGIN(H5Ldelete_by_idx_hard) {
            TESTING_2("H5Ldelete_by_idx on hard link")

            if (H5Lcreate_hard(group_id, ".", group_id, LINK_DELETE_TEST_HARD_LINK_NAME2, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create second hard link '%s'\n", LINK_DELETE_TEST_HARD_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_hard);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_HARD_LINK_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if second hard link '%s' exists\n", LINK_DELETE_TEST_HARD_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    second hard link did not exist\n");
                PART_ERROR(H5Ldelete_by_idx_hard);
            }

            if (H5Ldelete_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't delete hard link '%s' using H5Ldelete_by_idx\n", LINK_DELETE_TEST_HARD_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_hard);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_HARD_LINK_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if second hard link '%s' exists\n", LINK_DELETE_TEST_HARD_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_hard);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    second hard link exists!\n");
                PART_ERROR(H5Ldelete_by_idx_hard);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_hard);

        PART_BEGIN(H5Ldelete_by_idx_soft) {
            TESTING_2("H5Ldelete_by_idx on soft link")

            if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" LINK_DELETE_TEST_SUBGROUP_NAME,
                    group_id, LINK_DELETE_TEST_SOFT_LINK_NAME2, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create second soft link '%s'\n", LINK_DELETE_TEST_SOFT_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_soft);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_SOFT_LINK_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if second soft link '%s' exists\n", LINK_DELETE_TEST_SOFT_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    second soft link did not exist\n");
                PART_ERROR(H5Ldelete_by_idx_soft);
            }

            if (H5Ldelete_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't delete soft link '%s' using H5Ldelete_by_idx\n", LINK_DELETE_TEST_SOFT_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_soft);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_SOFT_LINK_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if second soft link '%s' exists\n", LINK_DELETE_TEST_SOFT_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_soft);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    second soft link exists!\n");
                PART_ERROR(H5Ldelete_by_idx_soft);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_soft);

        PART_BEGIN(H5Ldelete_by_idx_external) {
            TESTING_2("H5Ldelete_by_idx on external link")

            if (H5Lcreate_external(ext_link_filename, "/", group_id, LINK_DELETE_TEST_EXTERNAL_LINK_NAME2,
                    H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create second external link '%s'\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_external);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_EXTERNAL_LINK_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if second external link '%s' exists\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_external);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    second external link did not exist\n");
                PART_ERROR(H5Ldelete_by_idx_external);
            }

            if (H5Ldelete_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't delete external link '%s' using H5Ldelete_by_idx\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_external);
            }

            if ((link_exists = H5Lexists(group_id, LINK_DELETE_TEST_EXTERNAL_LINK_NAME2, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if second external link '%s' exists\n", LINK_DELETE_TEST_EXTERNAL_LINK_NAME2);
                PART_ERROR(H5Ldelete_by_idx_external);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    second external link exists!\n");
                PART_ERROR(H5Ldelete_by_idx_external);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_external);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Pclose(gcpl_id) < 0)
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
        H5Pclose(gcpl_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_delete_link_invalid_params(void)
{
    htri_t link_exists;
    herr_t err_ret = -1;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("H5Ldelete with invalid parameters");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_DELETE_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_DELETE_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_hard(group_id, ".", group_id, LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create hard link '%s'\n", LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    /* Verify the links have been created */
    if ((link_exists = H5Lexists(group_id, LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if first hard link '%s' exists\n", LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    first hard link did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Ldelete_invalid_loc_id) {
            TESTING_2("H5Ldelete with an invalid location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete(H5I_INVALID_HID, LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete succeeded with an invalid location ID!\n");
                PART_ERROR(H5Ldelete_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Ldelete_invalid_loc_id);

        PART_BEGIN(H5Ldelete_invalid_link_name) {
            TESTING_2("H5Ldelete with an invalid link name")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete(group_id, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete succeeded with a NULL link name!\n");
                PART_ERROR(H5Ldelete_invalid_link_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete(group_id, "", H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete succeeded with an invalid link name of ''!\n");
                PART_ERROR(H5Ldelete_invalid_link_name);
            }

            PASSED();
        } PART_END(H5Ldelete_invalid_link_name);

        PART_BEGIN(H5Ldelete_invalid_lapl) {
            TESTING_2("H5Ldelete with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete(group_id, LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Ldelete_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Ldelete_invalid_lapl);

        PART_BEGIN(H5Ldelete_by_idx_invalid_loc_id) {
            TESTING_2("H5Ldelete_by_idx with an invalid location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(H5I_INVALID_HID, ".", H5_INDEX_NAME, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with an invalid location ID!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_invalid_loc_id);

        PART_BEGIN(H5Ldelete_by_idx_invalid_grp_name) {
            TESTING_2("H5Ldelete_by_idx with an invalid group name")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(group_id, NULL, H5_INDEX_NAME, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with a NULL group name!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_grp_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(group_id, "", H5_INDEX_NAME, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with an invalid group name of ''!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_grp_name);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_invalid_grp_name);

        PART_BEGIN(H5Ldelete_by_idx_invalid_index_type) {
            TESTING_2("H5Ldelete_by_idx with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(group_id, ".", H5_INDEX_UNKNOWN, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(group_id, ".", H5_INDEX_N, H5_ITER_INC, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_invalid_index_type);

        PART_BEGIN(H5Ldelete_by_idx_invalid_index_order) {
            TESTING_2("H5Ldelete_by_idx with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_UNKNOWN, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_index_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_N, 0, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_index_order);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_invalid_index_order);

        PART_BEGIN(H5Ldelete_by_idx_invalid_lapl) {
            TESTING_2("H5Ldelete_by_idx with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Ldelete_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, 0, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Ldelete_by_idx succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Ldelete_by_idx_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_invalid_lapl);

        PART_BEGIN(H5Ldelete_by_idx_link_existence) {
            TESTING_2("link existence")

            /* Verify that the link hasn't been deleted */
            if ((link_exists = H5Lexists(group_id, LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", LINK_DELETE_INVALID_PARAMS_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Ldelete_by_idx_link_existence);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    hard link didn't exist!\n");
                PART_ERROR(H5Ldelete_by_idx_link_existence);
            }

            PASSED();
        } PART_END(H5Ldelete_by_idx_link_existence);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a link can be copied using H5Lcopy.
 */
static int
test_copy_link(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link copying")

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, COPY_LINK_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", COPY_LINK_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lcopy_hard) {
            TESTING_2("H5Lcopy on hard link")

            /* Try to copy a hard link */
            if (H5Lcreate_hard(group_id, ".", group_id, COPY_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create hard link '%s'\n", COPY_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lcopy_hard);
            }

            /* Verify the link has been created */
            if ((link_exists = H5Lexists(group_id, COPY_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", COPY_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lcopy_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    hard link did not exist\n");
                PART_ERROR(H5Lcopy_hard);
            }

            /* Copy the link */
            if (H5Lcopy(group_id, COPY_LINK_TEST_HARD_LINK_NAME, group_id, COPY_LINK_TEST_HARD_LINK_COPY_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to copy hard link '%s'\n", COPY_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lcopy_hard);
            }

            /* Verify the link has been copied */
            if ((link_exists = H5Lexists(group_id, COPY_LINK_TEST_HARD_LINK_COPY_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link copy '%s' exists\n", COPY_LINK_TEST_HARD_LINK_COPY_NAME);
                PART_ERROR(H5Lcopy_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    hard link copy did not exist\n");
                PART_ERROR(H5Lcopy_hard);
            }

            PASSED();
        } PART_END(H5Lcopy_hard);

        PART_BEGIN(H5Lcopy_soft) {
            TESTING_2("H5Lcopy on soft link")

            /* Try to copy a soft link */
            if (H5Lcreate_soft(COPY_LINK_TEST_SOFT_LINK_TARGET_PATH, group_id, COPY_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create soft link '%s'\n", COPY_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lcopy_soft);
            }

            /* Verify the link has been created */
            if ((link_exists = H5Lexists(group_id, COPY_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if soft link '%s' exists\n", COPY_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lcopy_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    soft link did not exist\n");
                PART_ERROR(H5Lcopy_soft);
            }

            /* Copy the link */
            if (H5Lcopy(group_id, COPY_LINK_TEST_SOFT_LINK_NAME, group_id, COPY_LINK_TEST_SOFT_LINK_COPY_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to copy soft link '%s'\n", COPY_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lcopy_soft);
            }

            /* Verify the link has been copied */
            if ((link_exists = H5Lexists(group_id, COPY_LINK_TEST_SOFT_LINK_COPY_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if soft link '%s' copy exists\n", COPY_LINK_TEST_SOFT_LINK_COPY_NAME);
                PART_ERROR(H5Lcopy_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    soft link copy did not exist\n");
                PART_ERROR(H5Lcopy_soft);
            }

            PASSED();
        } PART_END(H5Lcopy_soft);

        PART_BEGIN(H5Lcopy_external) {
            TESTING_2("H5Lcopy on external link")

            /* Try to copy an external link */
            if (H5Lcreate_external(ext_link_filename, "/", group_id, COPY_LINK_TEST_EXTERNAL_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create external link '%s'\n", COPY_LINK_TEST_EXTERNAL_LINK_NAME);
                PART_ERROR(H5Lcopy_external);
            }

            /* Verify the link has been created */
            if ((link_exists = H5Lexists(group_id, COPY_LINK_TEST_EXTERNAL_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if external link '%s' exists\n", COPY_LINK_TEST_EXTERNAL_LINK_NAME);
                PART_ERROR(H5Lcopy_external);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    external link did not exist\n");
                PART_ERROR(H5Lcopy_external);
            }

            /* Copy the link */
            if (H5Lcopy(group_id, COPY_LINK_TEST_EXTERNAL_LINK_NAME, group_id, COPY_LINK_TEST_EXTERNAL_LINK_COPY_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to copy external link '%s'\n", COPY_LINK_TEST_EXTERNAL_LINK_NAME);
                PART_ERROR(H5Lcopy_external);
            }

            /* Verify the link has been copied */
            if ((link_exists = H5Lexists(group_id, COPY_LINK_TEST_EXTERNAL_LINK_COPY_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if external link copy '%s' exists\n", COPY_LINK_TEST_EXTERNAL_LINK_COPY_NAME);
                PART_ERROR(H5Lcopy_external);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    external link copy did not exist\n");
                PART_ERROR(H5Lcopy_external);
            }

            PASSED();
        } PART_END(H5Lcopy_external);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a link can't be copied
 * when H5Lcopy is passed invalid parameters.
 */
static int
test_copy_link_invalid_params(void)
{
    herr_t err_ret = -1;
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;

    TESTING_MULTIPART("H5Lcopy with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, COPY_LINK_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", COPY_LINK_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_hard(group_id, ".", group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create hard link '%s'\n", COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if hard link '%s' exists\n", COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    hard link did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lcopy_invalid_src_loc_id) {
            TESTING_2("H5Lcopy with an invalid source location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(H5I_INVALID_HID, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, group_id,
                        COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_COPY_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with an invalid source location ID\n");
                PART_ERROR(H5Lcopy_invalid_src_loc_id);
            }

            PASSED();
        } PART_END(H5Lcopy_invalid_src_loc_id);

        PART_BEGIN(H5Lcopy_invalid_src_name) {
            TESTING_2("H5Lcopy with an invalid source name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(group_id, NULL, group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_COPY_NAME,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with a NULL source name\n");
                PART_ERROR(H5Lcopy_invalid_src_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(group_id, "", group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_COPY_NAME,
                        H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with an invalid source name of ''\n");
                PART_ERROR(H5Lcopy_invalid_src_name);
            }

            PASSED();
        } PART_END(H5Lcopy_invalid_src_name);

        PART_BEGIN(H5Lcopy_invalid_dst_loc_id) {
            TESTING_2("H5Lcopy with an invalid destination location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5I_INVALID_HID,
                        COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_COPY_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with an invalid destination location ID\n");
                PART_ERROR(H5Lcopy_invalid_dst_loc_id);
            }

            PASSED();
        } PART_END(H5Lcopy_invalid_dst_loc_id);

        PART_BEGIN(H5Lcopy_invalid_dst_name) {
            TESTING_2("H5Lcopy with an invalid destination name")

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, group_id,
                        NULL, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with a NULL destination name\n");
                PART_ERROR(H5Lcopy_invalid_dst_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, group_id,
                        "", H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with an invalid destination name of ''\n");
                PART_ERROR(H5Lcopy_invalid_dst_name);
            }

            PASSED();
        } PART_END(H5Lcopy_invalid_dst_name);

        PART_BEGIN(H5Lcopy_invalid_lcpl) {
            TESTING_2("H5Lcopy with an invalid LCPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, group_id,
                        COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_COPY_NAME, H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with an invalid LCPL\n");
                PART_ERROR(H5Lcopy_invalid_lcpl);
            }

            PASSED();
        } PART_END(H5Lcopy_invalid_lcpl);

        PART_BEGIN(H5Lcopy_invalid_lapl) {
            TESTING_2("H5Lcopy with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lcopy(group_id, COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, group_id,
                        COPY_LINK_INVALID_PARAMS_TEST_HARD_LINK_COPY_NAME, H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lcopy succeeded with an invalid LAPL\n");
                PART_ERROR(H5Lcopy_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lcopy_invalid_lapl);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a link can be moved with H5Lmove.
 *
 * XXX: external links
 */
static int
test_move_link(void)
{
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;

    TESTING_MULTIPART("link moving")

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, MOVE_LINK_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", MOVE_LINK_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lmove_hard) {
            TESTING_2("H5Lmove on hard link")

            /* Try to move a hard link */
            if (H5Lcreate_hard(group_id, ".", file_id, MOVE_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create hard link '%s'\n", MOVE_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_hard);
            }

            /* Verify the link has been created */
            if ((link_exists = H5Lexists(file_id, MOVE_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", MOVE_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    hard link did not exist\n");
                PART_ERROR(H5Lmove_hard);
            }

            /* Verify the link doesn't currently exist in the target group */
            if ((link_exists = H5Lexists(group_id, MOVE_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", MOVE_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_hard);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    hard link existed in target group before move!\n");
                PART_ERROR(H5Lmove_hard);
            }

            /* Move the link */
            if (H5Lmove(file_id, MOVE_LINK_TEST_HARD_LINK_NAME, group_id, MOVE_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to move link '%s'\n", MOVE_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_hard);
            }

            /* Verify the link has been moved */
            if ((link_exists = H5Lexists(group_id, MOVE_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", MOVE_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    hard link did not exist\n");
                PART_ERROR(H5Lmove_hard);
            }

            /* Verify the old link is gone */
            if ((link_exists = H5Lexists(file_id, MOVE_LINK_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if old hard link '%s' exists\n", MOVE_LINK_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_hard);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    old hard link exists\n");
                PART_ERROR(H5Lmove_hard);
            }

            PASSED();
        } PART_END(H5Lmove_hard);

        PART_BEGIN(H5Lmove_soft) {
            TESTING_2("H5Lmove on soft link")

            /* Try to move a soft link */
            if (H5Lcreate_soft(MOVE_LINK_TEST_SOFT_LINK_TARGET_PATH, file_id, MOVE_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create soft link '%s'\n", MOVE_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lmove_soft);
            }

            /* Verify the link has been created */
            if ((link_exists = H5Lexists(file_id, MOVE_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if soft link '%s' exists\n", MOVE_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lmove_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    soft link did not exist\n");
                PART_ERROR(H5Lmove_soft);
            }

            /* Verify the link doesn't currently exist in the target group */
            if ((link_exists = H5Lexists(group_id, MOVE_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if soft link '%s' exists\n", MOVE_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lmove_soft);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    soft link existed in target group before move!\n");
                PART_ERROR(H5Lmove_soft);
            }

            /* Move the link */
            if (H5Lmove(file_id, MOVE_LINK_TEST_SOFT_LINK_NAME, group_id, MOVE_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to move link '%s'\n", MOVE_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lmove_soft);
            }

            /* Verify the link has been moved */
            if ((link_exists = H5Lexists(group_id, MOVE_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if soft link '%s' exists\n", MOVE_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lmove_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    soft link did not exist\n");
                PART_ERROR(H5Lmove_soft);
            }

            /* Verify the old link is gone */
            if ((link_exists = H5Lexists(file_id, MOVE_LINK_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if old soft link '%s' exists\n", MOVE_LINK_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lmove_soft);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    old soft link exists\n");
                PART_ERROR(H5Lmove_soft);
            }

            PASSED();
        } PART_END(H5Lmove_soft);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_move_link_invalid_params(void)
{
    htri_t link_exists;
    herr_t err_ret = -1;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;

    TESTING_MULTIPART("H5Lmove with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, MOVE_LINK_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create group '%s'\n", MOVE_LINK_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_hard(group_id, ".", group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create hard link '%s'\n", MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if hard link '%s' exists\n", MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    hard link did not exist\n");
        goto error;
    }

    /* Verify the link doesn't currently exist in the target group */
    if ((link_exists = H5Lexists(file_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if hard link '%s' exists\n", MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (link_exists) {
        H5_FAILED();
        HDprintf("    hard link existed in target group before move!\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lmove_invalid_src_loc_id) {
            TESTING_2("H5Lmove with an invalid source location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(H5I_INVALID_HID, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, file_id,
                        MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with an invalid source location ID!\n");
                PART_ERROR(H5Lmove_invalid_src_loc_id);
            }

            PASSED();
        } PART_END(H5Lmove_invalid_src_loc_id);

        PART_BEGIN(H5Lmove_invalid_src_name) {
            TESTING_2("H5Lmove with an invalid source name")

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(group_id, NULL, file_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with a NULL source name!\n");
                PART_ERROR(H5Lmove_invalid_src_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(group_id, "", file_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with an invalid source name of ''!\n");
                PART_ERROR(H5Lmove_invalid_src_name);
            }

            PASSED();
        } PART_END(H5Lmove_invalid_src_name);

        PART_BEGIN(H5Lmove_invalid_dst_loc_id) {
            TESTING_2("H5Lmove with an invalid destination location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5I_INVALID_HID,
                        MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with an invalid destination location ID!\n");
                PART_ERROR(H5Lmove_invalid_dst_loc_id);
            }

            PASSED();
        } PART_END(H5Lmove_invalid_dst_loc_id);

        PART_BEGIN(H5Lmove_invalid_dst_name) {
            TESTING_2("H5Lmove with an invalid destination name")

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, file_id, NULL, H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with a NULL destination name!\n");
                PART_ERROR(H5Lmove_invalid_dst_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, file_id, "", H5P_DEFAULT, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with an invalid destination name of ''!\n");
                PART_ERROR(H5Lmove_invalid_dst_name);
            }

            PASSED();
        } PART_END(H5Lmove_invalid_dst_name);

        PART_BEGIN(H5Lmove_invalid_lcpl) {
            TESTING_2("H5Lmove with an invalid LCPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, file_id,
                        MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5I_INVALID_HID, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with an invalid LCPL!\n");
                PART_ERROR(H5Lmove_invalid_lcpl);
            }

            PASSED();
        } PART_END(H5Lmove_invalid_lcpl);

        PART_BEGIN(H5Lmove_invalid_lapl) {
            TESTING_2("H5Lmove with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lmove(group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, file_id,
                        MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lmove succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Lmove_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lmove_invalid_lapl);

        PART_BEGIN(H5Lmove_existence) {
            TESTING_2("link existence")

            /* Verify the link hasn't been moved */
            if ((link_exists = H5Lexists(group_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_existence);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    hard link didn't exist in source group after invalid move!\n");
                PART_ERROR(H5Lmove_existence);
            }

            if ((link_exists = H5Lexists(file_id, MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", MOVE_LINK_INVALID_PARAMS_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lmove_existence);
            }

            if (link_exists) {
                H5_FAILED();
                HDprintf("    hard link existed in target group after invalid move!\n");
                PART_ERROR(H5Lmove_existence);
            }

            PASSED();
        } PART_END(H5Lmove_existence);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a soft or external link's value can
 * be retrieved by using H5Lget_val and H5Lget_val_by_idx.
 */
static int
test_get_link_val(void)
{
    H5L_info_t  link_info;
    const char *ext_link_filepath;
    const char *ext_link_val;
    unsigned    ext_link_flags;
    htri_t      link_exists;
    size_t      link_val_buf_size = 0;
    char       *link_val_buf = NULL;
    hid_t       file_id = H5I_INVALID_HID;
    hid_t       container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t       gcpl_id = H5I_INVALID_HID;
    char        ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link value retrieval");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create GCPL for link creation order tracking\n");
        goto error;
    }

    if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED) < 0) {
        H5_FAILED();
        HDprintf("    couldn't set link creation order tracking\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, GET_LINK_VAL_TEST_SUBGROUP_NAME,
            H5P_DEFAULT, gcpl_id, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", GET_LINK_VAL_TEST_SUBGROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lget_val_soft) {
            TESTING_2("H5Lget_val on soft link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" GET_LINK_VAL_TEST_SUBGROUP_NAME, group_id,
                    GET_LINK_VAL_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create soft link '%s'\n", GET_LINK_VAL_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lget_val_soft);
            }

            if ((link_exists = H5Lexists(group_id, GET_LINK_VAL_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if link '%s' exists\n", GET_LINK_VAL_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lget_val_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    link did not exist\n");
                PART_ERROR(H5Lget_val_soft);
            }

            if (H5Lget_info(group_id, GET_LINK_VAL_TEST_SOFT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get soft link info\n");
                PART_ERROR(H5Lget_val_soft);
            }

            if (link_info.type != H5L_TYPE_SOFT) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_val_soft);
            }

            link_val_buf_size = link_info.u.val_size;
            if (NULL == (link_val_buf = (char *) HDmalloc(link_val_buf_size)))
                TEST_ERROR

            if (H5Lget_val(group_id, GET_LINK_VAL_TEST_SOFT_LINK_NAME, link_val_buf, link_val_buf_size, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get soft link value\n");
                PART_ERROR(H5Lget_val_soft);
            }

            if (HDstrcmp(link_val_buf, "/" LINK_TEST_GROUP_NAME "/" GET_LINK_VAL_TEST_SUBGROUP_NAME)) {
                H5_FAILED();
                HDprintf("    soft link value did not match\n");
                PART_ERROR(H5Lget_val_soft);
            }

            PASSED();
        } PART_END(H5Lget_val_soft);

        PART_BEGIN(H5Lget_val_external) {
            TESTING_2("H5Lget_val on external link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lcreate_external(ext_link_filename, "/", group_id, GET_LINK_VAL_TEST_EXT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create external link '%s'\n", GET_LINK_VAL_TEST_EXT_LINK_NAME);
                PART_ERROR(H5Lget_val_external);
            }

            if ((link_exists = H5Lexists(group_id, GET_LINK_VAL_TEST_EXT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if external link '%s' exists\n", GET_LINK_VAL_TEST_EXT_LINK_NAME);
                PART_ERROR(H5Lget_val_external);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    external link did not exist\n");
                PART_ERROR(H5Lget_val_external);
            }

            if (H5Lget_info(group_id, GET_LINK_VAL_TEST_EXT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get external link info\n");
                PART_ERROR(H5Lget_val_external);
            }

            if (link_info.type != H5L_TYPE_EXTERNAL) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_val_external);
            }

            while (link_info.u.val_size > link_val_buf_size) {
                char *tmp_realloc;

                link_val_buf_size *= 2;

                if (NULL == (tmp_realloc = (char *) HDrealloc(link_val_buf, link_val_buf_size)))
                    TEST_ERROR
                link_val_buf = tmp_realloc;
            }

            if (H5Lget_val(group_id, GET_LINK_VAL_TEST_EXT_LINK_NAME, link_val_buf, link_val_buf_size, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get external link value\n");
                PART_ERROR(H5Lget_val_external);
            }

            if (H5Lunpack_elink_val(link_val_buf, link_val_buf_size, &ext_link_flags, &ext_link_filepath, &ext_link_val) < 0) {
                H5_FAILED();
                HDprintf("    couldn't unpack external link value buffer\n");
                PART_ERROR(H5Lget_val_external);
            }

            if (HDstrcmp(ext_link_filepath, ext_link_filename)) {
                H5_FAILED();
                HDprintf("    external link target file '%s' did not match expected '%s'\n", ext_link_filepath, ext_link_filename);
                PART_ERROR(H5Lget_val_external);
            }

            if (HDstrcmp(ext_link_val, "/")) {
                H5_FAILED();
                HDprintf("    external link value '%s' did not match expected '%s'\n", ext_link_val, "/");
                PART_ERROR(H5Lget_val_external);
            }

            PASSED();
        } PART_END(H5Lget_val_external);

        PART_BEGIN(H5Lget_val_by_idx_soft) {
            TESTING_2("H5Lget_val_by_idx on soft link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lget_info(group_id, GET_LINK_VAL_TEST_SOFT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get soft link info\n");
                PART_ERROR(H5Lget_val_by_idx_soft);
            }

            if (link_info.type != H5L_TYPE_SOFT) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_val_by_idx_soft);
            }

            while (link_info.u.val_size > link_val_buf_size) {
                char *tmp_realloc;

                link_val_buf_size *= 2;

                if (NULL == (tmp_realloc = (char *) HDrealloc(link_val_buf, link_val_buf_size)))
                    TEST_ERROR
                link_val_buf = tmp_realloc;
            }

            if (H5Lget_val_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get soft link value\n");
                PART_ERROR(H5Lget_val_by_idx_soft);
            }

            if (HDstrcmp(link_val_buf, "/" LINK_TEST_GROUP_NAME "/" GET_LINK_VAL_TEST_SUBGROUP_NAME)) {
                H5_FAILED();
                HDprintf("    soft link value did not match\n");
                PART_ERROR(H5Lget_val_by_idx_soft);
            }

            PASSED();
        } PART_END(H5Lget_val_by_idx_soft);

        PART_BEGIN(H5Lget_val_by_idx_external) {
            TESTING_2("H5Lget_val_by_idx on external link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lget_info(group_id, GET_LINK_VAL_TEST_EXT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get external link info\n");
                PART_ERROR(H5Lget_val_by_idx_external);
            }

            if (link_info.type != H5L_TYPE_EXTERNAL) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_val_by_idx_external);
            }

            while (link_info.u.val_size > link_val_buf_size) {
                char *tmp_realloc;

                link_val_buf_size *= 2;

                if (NULL == (tmp_realloc = (char *) HDrealloc(link_val_buf, link_val_buf_size)))
                    TEST_ERROR
                link_val_buf = tmp_realloc;
            }

            if (H5Lget_val_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 1, link_val_buf, link_val_buf_size, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get external link value\n");
                PART_ERROR(H5Lget_val_by_idx_external);
            }

            {
                const char *link_filename_retrieved = NULL;

                if (H5Lunpack_elink_val(link_val_buf, link_val_buf_size, &ext_link_flags, &link_filename_retrieved, &ext_link_val) < 0) {
                    H5_FAILED();
                    HDprintf("    couldn't unpack external link value buffer\n");
                    PART_ERROR(H5Lget_val_by_idx_external);
                }

                if (HDstrcmp(link_filename_retrieved, ext_link_filename)) {
                    H5_FAILED();
                    HDprintf("    external link target file '%s' did not match expected '%s'\n", ext_link_filename, ext_link_filename);
                    PART_ERROR(H5Lget_val_by_idx_external);
                }

                if (HDstrcmp(ext_link_val, "/")) {
                    H5_FAILED();
                    HDprintf("    external link value '%s' did not match expected '%s'\n", ext_link_val, "/");
                    PART_ERROR(H5Lget_val_by_idx_external);
                }
            }

            PASSED();
        } PART_END(H5Lget_val_by_idx_external);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (link_val_buf) {
        HDfree(link_val_buf);
        link_val_buf = NULL;
    }

    if (H5Pclose(gcpl_id) < 0)
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
        if (link_val_buf) HDfree(link_val_buf);
        H5Pclose(gcpl_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a soft or external link's value can't be
 * retrieved when H5Lget_val(_by_idx) is passed invalid parameters.
 */
static int
test_get_link_val_invalid_params(void)
{
    H5L_info_t  link_info;
    htri_t      link_exists;
    herr_t      err_ret = -1;
    size_t      link_val_buf_size = 0;
    char       *link_val_buf = NULL;
    hid_t       file_id = H5I_INVALID_HID;
    hid_t       container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char        ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link value retrieval with invalid parameters");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, GET_LINK_VAL_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", GET_LINK_VAL_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" GET_LINK_VAL_INVALID_PARAMS_TEST_GROUP_NAME, group_id,
            GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME);
        goto error;
    }

    /* Verify the links have been created */
    if ((link_exists = H5Lexists(group_id, GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lget_val_invalid_loc_id) {
            TESTING_2("H5Lget_val with an invalid location ID")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lget_info(group_id, GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get soft link info\n");
                PART_ERROR(H5Lget_val_invalid_loc_id);
            }

            if (link_info.type != H5L_TYPE_SOFT) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_val_invalid_loc_id);
            }

            link_val_buf_size = link_info.u.val_size;
            if (NULL == (link_val_buf = (char *) HDmalloc(link_val_buf_size)))
                TEST_ERROR

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val(H5I_INVALID_HID, GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val succeeded with an invalid location ID\n");
                PART_ERROR(H5Lget_val_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Lget_val_invalid_loc_id);

        PART_BEGIN(H5Lget_val_invalid_link_name) {
            TESTING_2("H5Lget_val with an invalid link name")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val(group_id, NULL, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val succeeded with a NULL link name\n");
                PART_ERROR(H5Lget_val_invalid_link_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val(group_id, "", link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val succeeded with an invalid link name of ''\n");
                PART_ERROR(H5Lget_val_invalid_link_name);
            }

            PASSED();
        } PART_END(H5Lget_val_invalid_link_name);

        PART_BEGIN(H5Lget_val_invalid_lapl) {
            TESTING_2("H5Lget_val with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val(group_id, GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME, link_val_buf, link_val_buf_size, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val succeeded with an invalid LAPL\n");
                PART_ERROR(H5Lget_val_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lget_val_invalid_lapl);

        PART_BEGIN(H5Lget_val_by_idx_invalid_loc_id) {
            TESTING_2("H5Lget_val_by_idx with an invalid location ID")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lget_info(group_id, GET_LINK_VAL_INVALID_PARAMS_TEST_SOFT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get soft link info\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_loc_id);
            }

            if (link_info.type != H5L_TYPE_SOFT) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_loc_id);
            }

            while (link_info.u.val_size > link_val_buf_size) {
                char *tmp_realloc;

                link_val_buf_size *= 2;

                if (NULL == (tmp_realloc = (char *) HDrealloc(link_val_buf, link_val_buf_size)))
                    TEST_ERROR
                link_val_buf = tmp_realloc;
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(H5I_INVALID_HID, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with an invalid location ID!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Lget_val_by_idx_invalid_loc_id);

        PART_BEGIN(H5Lget_val_by_idx_invalid_grp_name) {
            TESTING_2("H5Lget_val_by_idx with an invalid group name")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(group_id, NULL, H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with a NULL group name!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_grp_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(group_id, "", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with an invalid group name of ''!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_grp_name);
            }

            PASSED();
        } PART_END(H5Lget_val_by_idx_invalid_grp_name);

        PART_BEGIN(H5Lget_val_by_idx_invalid_index_type) {
            TESTING_2("H5Lget_val_by_idx with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(group_id, ".", H5_INDEX_UNKNOWN, H5_ITER_INC, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(group_id, ".", H5_INDEX_N, H5_ITER_INC, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Lget_val_by_idx_invalid_index_type);

        PART_BEGIN(H5Lget_val_by_idx_invalid_iter_order) {
            TESTING_2("H5Lget_val_by_idx with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_UNKNOWN, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_N, 0, link_val_buf, link_val_buf_size, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Lget_val_by_idx_invalid_iter_order);

        PART_BEGIN(H5Lget_val_by_idx_invalid_lapl) {
            TESTING_2("H5Lget_val_by_idx with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_val_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, link_val_buf, link_val_buf_size, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_val_by_idx succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Lget_val_by_idx_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lget_val_by_idx_invalid_lapl);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (link_val_buf) {
        HDfree(link_val_buf);
        link_val_buf = NULL;
    }

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
        if (link_val_buf) HDfree(link_val_buf);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check the functionality of H5Lget_info and
 * H5Lget_info_by_idx.
 *
 * XXX: create different links for _by_idx part of test
 */
static int
test_get_link_info(void)
{
    H5L_info_t link_info;
    htri_t     link_exists;
    hid_t      file_id = H5I_INVALID_HID;
    hid_t      container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t      gcpl_id = H5I_INVALID_HID;
    char       ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link info retrieval");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create GCPL for link creation order tracking\n");
        goto error;
    }

    if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED) < 0) {
        H5_FAILED();
        HDprintf("    couldn't set link creation order tracking\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, GET_LINK_INFO_TEST_GROUP_NAME, H5P_DEFAULT, gcpl_id, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", GET_LINK_INFO_TEST_GROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lget_info_hard) {
            TESTING_2("H5Lget_info on hard link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lcreate_hard(group_id, ".", group_id, GET_LINK_INFO_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create hard link '%s'\n", GET_LINK_INFO_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lget_info_hard);
            }

            if ((link_exists = H5Lexists(group_id, GET_LINK_INFO_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if hard link '%s' exists\n", GET_LINK_INFO_TEST_HARD_LINK_NAME);
                PART_ERROR(H5Lget_info_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    hard link did not exist\n");
                PART_ERROR(H5Lget_info_hard);
            }

            if (H5Lget_info(group_id, GET_LINK_INFO_TEST_HARD_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get hard link info\n");
                PART_ERROR(H5Lget_info_hard);
            }

            if (link_info.type != H5L_TYPE_HARD) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_info_hard);
            }

            PASSED();
        } PART_END(H5Lget_info_hard);

        PART_BEGIN(H5Lget_info_soft) {
            TESTING_2("H5Lget_info on soft link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" GET_LINK_INFO_TEST_GROUP_NAME,
                    group_id, GET_LINK_INFO_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create soft link '%s'\n", GET_LINK_INFO_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lget_info_soft);
            }

            if ((link_exists = H5Lexists(group_id, GET_LINK_INFO_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if soft link '%s' exists\n", GET_LINK_INFO_TEST_SOFT_LINK_NAME);
                PART_ERROR(H5Lget_info_soft);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    soft link did not exist\n");
                PART_ERROR(H5Lget_info_soft);
            }

            if (H5Lget_info(group_id, GET_LINK_INFO_TEST_SOFT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get soft link info\n");
                PART_ERROR(H5Lget_info_soft);
            }

            if (link_info.type != H5L_TYPE_SOFT) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_info_soft);
            }

            PASSED();
        } PART_END(H5Lget_info_soft);

        PART_BEGIN(H5Lget_info_external) {
            TESTING_2("H5Lget_info on external link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lget_info(group_id, GET_LINK_INFO_TEST_EXT_LINK_NAME, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't get external link info\n");
                PART_ERROR(H5Lget_info_external);
            }

            if (link_info.type != H5L_TYPE_EXTERNAL) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_info_external);
            }

            PASSED();
        } PART_END(H5Lget_info_external);

        PART_BEGIN(H5Lget_info_by_idx_hard) {
            TESTING_2("H5Lget_info_by_idx on hard link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lcreate_external(ext_link_filename, "/", group_id, GET_LINK_INFO_TEST_EXT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    couldn't create external link '%s'\n", GET_LINK_INFO_TEST_EXT_LINK_NAME);
                PART_ERROR(H5Lget_info_by_idx_hard);
            }

            if ((link_exists = H5Lexists(group_id, GET_LINK_INFO_TEST_EXT_LINK_NAME, H5P_DEFAULT)) < 0) {
                H5_FAILED();
                HDprintf("    couldn't determine if external link '%s' exists\n", GET_LINK_INFO_TEST_EXT_LINK_NAME);
                PART_ERROR(H5Lget_info_by_idx_hard);
            }

            if (!link_exists) {
                H5_FAILED();
                HDprintf("    external link did not exist\n");
                PART_ERROR(H5Lget_info_by_idx_hard);
            }

            if (H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to retrieve hard link info\n");
                PART_ERROR(H5Lget_info_by_idx_hard);
            }

            if (link_info.type != H5L_TYPE_HARD) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_info_by_idx_hard);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_hard);

        PART_BEGIN(H5Lget_info_by_idx_soft) {
            TESTING_2("H5Lget_info_by_idx on soft link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 1, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to retrieve soft link info\n");
                PART_ERROR(H5Lget_info_by_idx_soft);
            }

            if (link_info.type != H5L_TYPE_SOFT) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_info_by_idx_soft);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_soft);

        PART_BEGIN(H5Lget_info_by_idx_external) {
            TESTING_2("H5Lget_info_by_idx on external link")

            HDmemset(&link_info, 0, sizeof(link_info));

            if (H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 2, &link_info, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    failed to retrieve external link info\n");
                PART_ERROR(H5Lget_info_by_idx_external);
            }

            if (link_info.type != H5L_TYPE_EXTERNAL) {
                H5_FAILED();
                HDprintf("    incorrect link type returned\n");
                PART_ERROR(H5Lget_info_by_idx_external);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_external);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Pclose(gcpl_id) < 0)
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
        H5Pclose(gcpl_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a link's info can't be retrieved
 * when H5Lget_info(_by_idx) is passed invalid parameters.
 */
static int
test_get_link_info_invalid_params(void)
{
    H5L_info_t link_info;
    herr_t     err_ret = -1;
    htri_t     link_exists;
    hid_t      file_id = H5I_INVALID_HID;
    hid_t      container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char       ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link info retrieval with invalid parameters");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, GET_LINK_INFO_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", GET_LINK_INFO_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_hard(group_id, ".", group_id, GET_LINK_INFO_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create hard link '%s'\n", GET_LINK_INFO_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, GET_LINK_INFO_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if hard link '%s' exists\n", GET_LINK_INFO_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    hard link did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lget_info_invalid_loc_id) {
            TESTING_2("H5Lget_info with an invalid location ID")

            HDmemset(&link_info, 0, sizeof(link_info));

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info(H5I_INVALID_HID, GET_LINK_INFO_INVALID_PARAMS_TEST_HARD_LINK_NAME, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info succeeded with an invalid location ID!\n");
                PART_ERROR(H5Lget_info_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Lget_info_invalid_loc_id);

        PART_BEGIN(H5Lget_info_invalid_link_name) {
            TESTING_2("H5Lget_info with an invalid link name")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info(group_id, NULL, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info succeeded with a NULL link name!\n");
                PART_ERROR(H5Lget_info_invalid_link_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info(group_id, "", &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info succeeded with an invalid link name of ''!\n");
                PART_ERROR(H5Lget_info_invalid_link_name);
            }

            PASSED();
        } PART_END(H5Lget_info_invalid_link_name);

        PART_BEGIN(H5Lget_info_invalid_lapl) {
            TESTING_2("H5Lget_info with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info(group_id, GET_LINK_INFO_INVALID_PARAMS_TEST_HARD_LINK_NAME, &link_info, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Lget_info_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lget_info_invalid_lapl);

        HDmemset(&link_info, 0, sizeof(link_info));

        PART_BEGIN(H5Lget_info_by_idx_invalid_loc_id) {
            TESTING_2("H5Lget_info_by_idx with an invalid location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(H5I_INVALID_HID, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with an invalid location ID!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_invalid_loc_id);

        PART_BEGIN(H5Lget_info_by_idx_invalid_grp_name) {
            TESTING_2("H5Lget_info_by_idx with an invalid group name")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(group_id, NULL, H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with a NULL group name!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_grp_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(group_id, "", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with an invalid group name of ''!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_grp_name);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_invalid_grp_name);

        PART_BEGIN(H5Lget_info_by_idx_invalid_index_type) {
            TESTING_2("H5Lget_info_by_idx with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(group_id, ".", H5_INDEX_UNKNOWN, H5_ITER_INC, 0, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(group_id, ".", H5_INDEX_N, H5_ITER_INC, 0, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_invalid_index_type);

        PART_BEGIN(H5Lget_info_by_idx_invalid_iter_order) {
            TESTING_2("H5Lget_info_by_idx with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_UNKNOWN, 0, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_N, 0, &link_info, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_invalid_iter_order);

        PART_BEGIN(H5Lget_info_by_idx_invalid_lapl) {
            TESTING_2("H5Lget_info_by_idx with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lget_info_by_idx(group_id, ".", H5_INDEX_CRT_ORDER, H5_ITER_INC, 0, &link_info, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_info_by_idx succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Lget_info_by_idx_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lget_info_by_idx_invalid_lapl);
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
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a link's name can be correctly
 * retrieved by using H5Lget_name_by_idx.
 *
 * XXX: soft and external links
 */
static int
test_get_link_name(void)
{
    ssize_t  ret;
    htri_t   link_exists;
    size_t   link_name_buf_size = 0;
    hid_t    file_id = H5I_INVALID_HID;
    hid_t    container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char    *link_name_buf = NULL;

    TESTING("link name retrieval")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, GET_LINK_NAME_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", GET_LINK_NAME_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_hard(group_id, ".", group_id, GET_LINK_NAME_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    failed to create hard link '%s'\n", GET_LINK_NAME_TEST_HARD_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, GET_LINK_NAME_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link exists\n");
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    if ((ret = H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, 0, NULL, link_name_buf_size, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to retrieve link name size\n");
        goto error;
    }

    link_name_buf_size = (size_t) ret;
    if (NULL == (link_name_buf = (char *) HDmalloc(link_name_buf_size + 1)))
        TEST_ERROR

    if (H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    failed to retrieve link name\n");
        goto error;
    }

    if (HDstrcmp(link_name_buf, GET_LINK_NAME_TEST_HARD_LINK_NAME)) {
        H5_FAILED();
        HDprintf("    link name '%s' did not match expected '%s'\n", link_name_buf, GET_LINK_NAME_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (link_name_buf) {
        HDfree(link_name_buf);
        link_name_buf = NULL;
    }

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
        if (link_name_buf) HDfree(link_name_buf);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that a link's name can't be retrieved
 * when H5Lget_name_by_idx is passed invalid parameters.
 */
static int
test_get_link_name_invalid_params(void)
{
    ssize_t  ret;
    htri_t   link_exists;
    size_t   link_name_buf_size = 0;
    hid_t    file_id = H5I_INVALID_HID;
    hid_t    container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    char    *link_name_buf = NULL;

    TESTING_MULTIPART("link name retrieval with invalid parameters");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, GET_LINK_NAME_INVALID_PARAMS_TEST_GROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", GET_LINK_NAME_INVALID_PARAMS_TEST_GROUP_NAME);
        goto error;
    }

    if (H5Lcreate_hard(group_id, ".", group_id, GET_LINK_NAME_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    failed to create hard link '%s'\n", GET_LINK_NAME_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    /* Verify the link has been created */
    if ((link_exists = H5Lexists(group_id, GET_LINK_NAME_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link exists\n");
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link did not exist\n");
        goto error;
    }

    if ((ret = H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, 0, NULL, link_name_buf_size, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    failed to retrieve link name size\n");
        goto error;
    }

    link_name_buf_size = (size_t) ret;
    if (NULL == (link_name_buf = (char *) HDmalloc(link_name_buf_size + 1)))
        TEST_ERROR

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lget_name_by_idx_invalid_loc_id) {
            TESTING_2("H5Lget_name_by_idx with an invalid location ID")

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(H5I_INVALID_HID, ".", H5_INDEX_NAME, H5_ITER_INC, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with an invalid location ID!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Lget_name_by_idx_invalid_loc_id);

        PART_BEGIN(H5Lget_name_by_idx_invalid_grp_name) {
            TESTING_2("H5Lget_name_by_idx with an invalid group name")

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(group_id, NULL, H5_INDEX_NAME, H5_ITER_INC, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with a NULL group name!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_grp_name);
            }

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(group_id, "", H5_INDEX_NAME, H5_ITER_INC, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with an invalid group name of ''!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_grp_name);
            }

            PASSED();
        } PART_END(H5Lget_name_by_idx_invalid_grp_name);

        PART_BEGIN(H5Lget_name_by_idx_invalid_index_type) {
            TESTING_2("H5Lget_name_by_idx with an invalid index type")

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(group_id, ".", H5_INDEX_UNKNOWN, H5_ITER_INC, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(group_id, ".", H5_INDEX_N, H5_ITER_INC, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Lget_name_by_idx_invalid_index_type);

        PART_BEGIN(H5Lget_name_by_idx_invalid_iter_order) {
            TESTING_2("H5Lget_name_by_idx with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_UNKNOWN, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_N, 0, link_name_buf, link_name_buf_size + 1, H5P_DEFAULT);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Lget_name_by_idx_invalid_iter_order);

        PART_BEGIN(H5Lget_name_by_idx_invalid_lapl) {
            TESTING_2("H5Lget_name_by_idx with an invalid LAPL")

            H5E_BEGIN_TRY {
                ret = H5Lget_name_by_idx(group_id, ".", H5_INDEX_NAME, H5_ITER_INC, 0, link_name_buf, link_name_buf_size + 1, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lget_name_by_idx succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Lget_name_by_idx_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lget_name_by_idx_invalid_lapl);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (link_name_buf) {
        HDfree(link_name_buf);
        link_name_buf = NULL;
    }

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
        if (link_name_buf) HDfree(link_name_buf);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check the functionality of link
 * iteration using H5Literate. Iteration is done
 * in increasing and decreasing order of both
 * link name and link creation order.
 */
static int
test_link_iterate(void)
{
    hsize_t saved_idx;
    size_t  i;
    htri_t  link_exists;
    hid_t   file_id = H5I_INVALID_HID;
    hid_t   container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t   gcpl_id = H5I_INVALID_HID;
    hid_t   dset_id = H5I_INVALID_HID;
    hid_t   dset_dtype = H5I_INVALID_HID;
    hid_t   dset_dspace = H5I_INVALID_HID;
    int     halted;
    char    ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link iteration");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create GCPL for link creation order tracking\n");
        goto error;
    }

    if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED) < 0) {
        H5_FAILED();
        HDprintf("    couldn't set link creation order tracking\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_ITER_TEST_SUBGROUP_NAME, H5P_DEFAULT, gcpl_id, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_ITER_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    if ((dset_dspace = generate_random_dataspace(LINK_ITER_TEST_DSET_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, LINK_ITER_TEST_HARD_LINK_NAME, dset_dtype, dset_dspace,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create hard link '%s'\n", LINK_ITER_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_SUBGROUP_NAME "/" LINK_ITER_TEST_HARD_LINK_NAME,
            group_id, LINK_ITER_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", LINK_ITER_TEST_SOFT_LINK_NAME);
        goto error;
    }

    if (H5Lcreate_external(ext_link_filename, "/", group_id, LINK_ITER_TEST_EXT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create external link '%s'\n", LINK_ITER_TEST_EXT_LINK_NAME);
        goto error;
    }

    /* Verify the links have been created */
    if ((link_exists = H5Lexists(group_id, LINK_ITER_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", LINK_ITER_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    first link did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(group_id, LINK_ITER_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", LINK_ITER_TEST_SOFT_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    second link did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(group_id, LINK_ITER_TEST_EXT_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", LINK_ITER_TEST_EXT_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    third link did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        /*
         * NOTE: Pass a counter to the iteration callback to try to match up the
         * expected links with a given step throughout all of the following
         * iterations. This is to try and check that the links are indeed being
         * returned in the correct order.
         */
        i = 0;

        PART_BEGIN(H5Literate_link_name_increasing) {
            TESTING_2("H5Literate by link name in increasing order")

            /* Test basic link iteration capability using both index types and both index orders */
            if (H5Literate(group_id, H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type name in increasing order failed\n");
                PART_ERROR(H5Literate_link_name_increasing);
            }

            PASSED();
        } PART_END(H5Literate_link_name_increasing);

        PART_BEGIN(H5Literate_link_name_decreasing) {
            TESTING_2("H5Literate by link name in decreasing order")

            if (H5Literate(group_id, H5_INDEX_NAME, H5_ITER_DEC, NULL, link_iter_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type name in decreasing order failed\n");
                PART_ERROR(H5Literate_link_name_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_link_name_decreasing);

        PART_BEGIN(H5Literate_link_creation_increasing) {
            TESTING_2("H5Literate by creation order in increasing order")

            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, NULL, link_iter_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type creation order in increasing order failed\n");
                PART_ERROR(H5Literate_link_creation_increasing);
            }

            PASSED();
        } PART_END(H5Literate_link_creation_increasing);

        PART_BEGIN(H5Literate_link_creation_decreasing) {
            TESTING_2("H5Literate by creation order in decreasing order")

            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, NULL, link_iter_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Literate_link_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_link_creation_decreasing);

        /*
         * Make sure to reset the special counter.
         */
        i = 0;

        PART_BEGIN(H5Literate_by_name_link_name_increasing) {
            TESTING_2("H5Literate_by_name by link name in increasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type name in increasing order failed\n");
                PART_ERROR(H5Literate_by_name_link_name_increasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_link_name_increasing);

        PART_BEGIN(H5Literate_by_name_link_name_decreasing) {
            TESTING_2("H5Literate_by_name by link name in decreasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_DEC, NULL, link_iter_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type name in decreasing order failed\n");
                PART_ERROR(H5Literate_by_name_link_name_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_link_name_decreasing);

        PART_BEGIN(H5Literate_by_name_creation_increasing) {
            TESTING_2("H5Literate_by_name by creation order in increasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_INC, NULL, link_iter_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type creation order in increasing order failed\n");
                PART_ERROR(H5Literate_by_name_creation_increasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_creation_increasing);

        PART_BEGIN(H5Literate_by_name_creation_decreasing) {
            TESTING_2("H5Literate_by_name by creation order in decreasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_DEC, NULL, link_iter_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Literate_by_name_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_creation_decreasing);

        PART_BEGIN(H5Literate_index_saving_increasing) {
            TESTING_2("H5Literate index-saving capabilities in increasing order")

            /* Test the H5Literate index-saving capabilities */
            saved_idx = 0;
            halted = 0;
            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, &saved_idx, link_iter_callback2, &halted) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate index-saving capability test failed\n");
                PART_ERROR(H5Literate_index_saving_increasing);
            }

            if (saved_idx != 2) {
                H5_FAILED();
                HDprintf("    saved index after iteration was wrong\n");
                PART_ERROR(H5Literate_index_saving_increasing);
            }

            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, &saved_idx, link_iter_callback2, &halted) < 0) {
                H5_FAILED();
                HDprintf("    couldn't finish iterating when beginning from saved index\n");
                PART_ERROR(H5Literate_index_saving_increasing);
            }

            PASSED();
        } PART_END(H5Literate_index_saving_increasing);

        PART_BEGIN(H5Literate_index_saving_decreasing) {
            TESTING_2("H5Literate index-saving capabilities in decreasing order")

            saved_idx = LINK_ITER_TEST_NUM_LINKS - 1;
            halted = 0;

            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, &saved_idx, link_iter_callback2, &halted) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate index-saving capability test failed\n");
                PART_ERROR(H5Literate_index_saving_decreasing);
            }

            if (saved_idx != 2) {
                H5_FAILED();
                HDprintf("    saved index after iteration was wrong\n");
                PART_ERROR(H5Literate_index_saving_decreasing);
            }

            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, &saved_idx, link_iter_callback2, &halted) < 0) {
                H5_FAILED();
                HDprintf("    couldn't finish iterating when beginning from saved index\n");
                PART_ERROR(H5Literate_index_saving_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_index_saving_decreasing);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(dset_dspace) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Pclose(gcpl_id) < 0)
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
        H5Sclose(dset_dspace);
        H5Tclose(dset_dtype);
        H5Dclose(dset_id);
        H5Pclose(gcpl_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_link_iterate_invalid_params(void)
{
    herr_t err_ret = -1;
    size_t i;
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  gcpl_id = H5I_INVALID_HID;
    hid_t  dset_id = H5I_INVALID_HID;
    hid_t  dset_dtype = H5I_INVALID_HID;
    hid_t  dset_dspace = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link iteration with invalid parameters");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fcreate(ext_link_filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create file '%s' for external link to reference\n", ext_link_filename);
        goto error;
    }

    if (H5Fclose(file_id) < 0)
        TEST_ERROR

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create GCPL for link creation order tracking\n");
        goto error;
    }

    if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED) < 0) {
        H5_FAILED();
        HDprintf("    couldn't set link creation order tracking\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME, H5P_DEFAULT, gcpl_id, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    if ((dset_dspace = generate_random_dataspace(LINK_ITER_INVALID_PARAMS_TEST_DSET_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(group_id, LINK_ITER_INVALID_PARAMS_TEST_HARD_LINK_NAME, dset_dtype, dset_dspace,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create hard link '%s'\n", LINK_ITER_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_HARD_LINK_NAME,
            group_id, LINK_ITER_INVALID_PARAMS_TEST_SOFT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", LINK_ITER_INVALID_PARAMS_TEST_SOFT_LINK_NAME);
        goto error;
    }

    if (H5Lcreate_external(ext_link_filename, "/", group_id, LINK_ITER_INVALID_PARAMS_TEST_EXT_LINK_NAME, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create external link '%s'\n", LINK_ITER_INVALID_PARAMS_TEST_EXT_LINK_NAME);
        goto error;
    }

    /* Verify the links have been created */
    if ((link_exists = H5Lexists(group_id, LINK_ITER_INVALID_PARAMS_TEST_HARD_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", LINK_ITER_INVALID_PARAMS_TEST_HARD_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    first link did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(group_id, LINK_ITER_INVALID_PARAMS_TEST_SOFT_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", LINK_ITER_INVALID_PARAMS_TEST_SOFT_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    second link did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(group_id, LINK_ITER_INVALID_PARAMS_TEST_EXT_LINK_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if link '%s' exists\n", LINK_ITER_INVALID_PARAMS_TEST_EXT_LINK_NAME);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    third link did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        i = 0;

        PART_BEGIN(H5Literate_invalid_grp_id) {
            TESTING_2("H5Literate with an invalid group ID")

            H5E_BEGIN_TRY {
                err_ret = H5Literate(H5I_INVALID_HID, H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate succeeded with an invalid group ID!\n");
                PART_ERROR(H5Literate_invalid_grp_id);
            }

            PASSED();
        } PART_END(H5Literate_invalid_grp_id);

        PART_BEGIN(H5Literate_invalid_index_type) {
            TESTING_2("H5Literate with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Literate(group_id, H5_INDEX_UNKNOWN, H5_ITER_INC, NULL, link_iter_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Literate_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Literate(group_id, H5_INDEX_N, H5_ITER_INC, NULL, link_iter_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Literate_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Literate_invalid_index_type);

        PART_BEGIN(H5Literate_invalid_iter_order) {
            TESTING_2("H5Literate with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Literate(group_id, H5_INDEX_NAME, H5_ITER_UNKNOWN, NULL, link_iter_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Literate_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Literate(group_id, H5_INDEX_NAME, H5_ITER_N, NULL, link_iter_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Literate_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Literate_invalid_iter_order);

        PART_BEGIN(H5Literate_by_name_invalid_loc_id) {
            TESTING_2("H5Literate_by_name with an invalid location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(H5I_INVALID_HID, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with an invalid location ID!\n");
                PART_ERROR(H5Literate_by_name_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Literate_by_name_invalid_loc_id);

        PART_BEGIN(H5Literate_by_name_invalid_grp_name) {
            TESTING_2("H5Literate_by_name with an invalid group name")

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(file_id, NULL, H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with a NULL group name!\n");
                PART_ERROR(H5Literate_by_name_invalid_grp_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(file_id, "", H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with an invalid group name of ''!\n");
                PART_ERROR(H5Literate_by_name_invalid_grp_name);
            }

            PASSED();
        } PART_END(H5Literate_by_name_invalid_grp_name);

        PART_BEGIN(H5Literate_by_name_invalid_index_type) {
            TESTING_2("H5Literate_by_name with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_UNKNOWN, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Literate_by_name_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_N, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Literate_by_name_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Literate_by_name_invalid_index_type);

        PART_BEGIN(H5Literate_by_name_invalid_iter_order) {
            TESTING_2("H5Literate_by_name with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_UNKNOWN, NULL, link_iter_callback3, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Literate_by_name_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_N, NULL, link_iter_callback3, &i, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Literate_by_name_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Literate_by_name_invalid_iter_order);

        PART_BEGIN(H5Literate_by_name_invalid_lapl) {
            TESTING_2("H5Literate_by_name with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Literate_by_name_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Literate_by_name_invalid_lapl);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(dset_dspace) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Dclose(dset_id) < 0)
        TEST_ERROR
    if (H5Pclose(gcpl_id) < 0)
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
        H5Sclose(dset_dspace);
        H5Tclose(dset_dtype);
        H5Dclose(dset_id);
        H5Pclose(gcpl_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that link iteration performed on a
 * group with no links in it is not problematic.
 */
static int
test_link_iterate_0_links(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t gcpl_id = H5I_INVALID_HID;

    TESTING_MULTIPART("link iteration on group with 0 links");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((gcpl_id = H5Pcreate(H5P_GROUP_CREATE)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create GCPL for link creation order tracking\n");
        goto error;
    }

    if (H5Pset_link_creation_order(gcpl_id, H5P_CRT_ORDER_TRACKED) < 0) {
        H5_FAILED();
        HDprintf("    couldn't set link creation order tracking\n");
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_ITER_TEST_0_LINKS_SUBGROUP_NAME, H5P_DEFAULT, gcpl_id, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_ITER_TEST_0_LINKS_SUBGROUP_NAME);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Literate_0_links_name_increasing) {
            TESTING_2("H5Literate by link name in increasing order")

            /* Test basic link iteration capability using both index types and both index orders */
            if (H5Literate(group_id, H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type name in increasing order failed\n");
                PART_ERROR(H5Literate_0_links_name_increasing);
            }

            PASSED();
        } PART_END(H5Literate_0_links_name_increasing);

        PART_BEGIN(H5Literate_0_links_name_decreasing) {
            TESTING_2("H5Literate by link name in decreasing order")

            if (H5Literate(group_id, H5_INDEX_NAME, H5_ITER_DEC, NULL, link_iter_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type name in decreasing order failed\n");
                PART_ERROR(H5Literate_0_links_name_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_0_links_name_decreasing);

        PART_BEGIN(H5Literate_0_links_creation_increasing) {
            TESTING_2("H5Literate by creation order in increasing order")

            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, NULL, link_iter_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type creation order in increasing order failed\n");
                PART_ERROR(H5Literate_0_links_creation_increasing);
            }

            PASSED();
        } PART_END(H5Literate_0_links_creation_increasing);

        PART_BEGIN(H5Literate_0_links_creation_decreasing) {
            TESTING_2("H5Literate by creation order in decreasing order")

            if (H5Literate(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, NULL, link_iter_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Literate_0_links_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_0_links_creation_decreasing);

        PART_BEGIN(H5Literate_by_name_0_links_name_increasing) {
            TESTING_2("H5Literate_by_name by link name in increasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type name in increasing order failed\n");
                PART_ERROR(H5Literate_by_name_0_links_name_increasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_0_links_name_increasing);

        PART_BEGIN(H5Literate_by_name_0_links_name_decreasing) {
            TESTING_2("H5Literate_by_name by link name in decreasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_DEC, NULL, link_iter_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type name in decreasing order failed\n");
                PART_ERROR(H5Literate_by_name_0_links_name_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_0_links_name_decreasing);

        PART_BEGIN(H5Literate_by_name_0_links_creation_increasing) {
            TESTING_2("H5Literate_by_name by creation order in increasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_INC, NULL, link_iter_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type creation order in increasing order failed\n");
                PART_ERROR(H5Literate_by_name_0_links_creation_increasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_0_links_creation_increasing);

        PART_BEGIN(H5Literate_by_name_0_links_creation_decreasing) {
            TESTING_2("H5Literate_by_name by creation order in decreasing order")

            if (H5Literate_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_ITER_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_DEC, NULL, link_iter_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Literate_by_name by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Literate_by_name_0_links_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Literate_by_name_0_links_creation_decreasing);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Pclose(gcpl_id) < 0)
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
        H5Pclose(gcpl_id);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check the functionality of recursive
 * link iteration using H5Lvisit where there are no
 * cyclic links. Iteration is done in increasing and
 * decreasing order of both link name and link creation
 * order.
 */
static int
test_link_visit(void)
{
    size_t i;
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  subgroup1 = H5I_INVALID_HID, subgroup2 = H5I_INVALID_HID;
    hid_t  dset_id = H5I_INVALID_HID;
    hid_t  dset_dtype = H5I_INVALID_HID;
    hid_t  fspace_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link visiting without cycles");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME);
        goto error;
    }

    if ((subgroup1 = H5Gcreate2(group_id, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first subgroup '%s'\n", LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2);
        goto error;
    }

    if ((subgroup2 = H5Gcreate2(group_id, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second subgroup '%s'\n", LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3);
        goto error;
    }

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    if ((fspace_id = generate_random_dataspace(LINK_VISIT_TEST_NO_CYCLE_DSET_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(subgroup1, LINK_VISIT_TEST_NO_CYCLE_DSET_NAME, dset_dtype, fspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first dataset '%s'\n", LINK_VISIT_TEST_NO_CYCLE_DSET_NAME);
    }

    if (H5Dclose(dset_id) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(subgroup2, LINK_VISIT_TEST_NO_CYCLE_DSET_NAME2, dset_dtype, fspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second dataset '%s'\n", LINK_VISIT_TEST_NO_CYCLE_DSET_NAME);
    }

    if (H5Dclose(dset_id) < 0)
        TEST_ERROR

    if (H5Lcreate_hard(subgroup1, LINK_VISIT_TEST_NO_CYCLE_DSET_NAME, subgroup1, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME1,
            H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first hard link '%s'\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME1);
        goto error;
    }

    if (H5Lcreate_soft(LINK_VISIT_TEST_NO_CYCLE_DSET_NAME, subgroup1, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME2, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME2);
        goto error;
    }

    if (H5Lcreate_external(ext_link_filename, "/", subgroup2, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME3, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create external link '%s'\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME3);
        goto error;
    }

    if (H5Lcreate_hard(subgroup2, LINK_VISIT_TEST_NO_CYCLE_DSET_NAME2, subgroup2, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME4,
            H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second hard link '%s'\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME4);
        goto error;
    }

    /* Verify the links have been created */
    if ((link_exists = H5Lexists(subgroup1, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME1, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if first link '%s' exists\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME1);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 1 did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup1, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME2, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if second link '%s' exists\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME2);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 2 did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup2, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME3, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if third link '%s' exists\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME3);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 3 did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup2, LINK_VISIT_TEST_NO_CYCLE_LINK_NAME4, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if fourth link '%s' exists\n", LINK_VISIT_TEST_NO_CYCLE_LINK_NAME4);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 4 did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        /*
         * NOTE: Pass a counter to the iteration callback to try to match up the
         * expected links with a given step throughout all of the following
         * iterations. This is to try and check that the links are indeed being
         * returned in the correct order.
         */
        i = 0;

        PART_BEGIN(H5Lvisit_no_cycles_link_name_increasing) {
            TESTING_2("H5Lvisit by link name in increasing order")

            if (H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_INC, link_visit_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type name in increasing order failed\n");
                PART_ERROR(H5Lvisit_no_cycles_link_name_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_no_cycles_link_name_increasing);

        PART_BEGIN(H5Lvisit_no_cycles_link_name_decreasing) {
            TESTING_2("H5Lvisit by link name in decreasing order")

            if (H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_DEC, link_visit_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type name in decreasing order failed\n");
                PART_ERROR(H5Lvisit_no_cycles_link_name_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_no_cycles_link_name_decreasing);

        PART_BEGIN(H5Lvisit_no_cycles_link_creation_increasing) {
            TESTING_2("H5Lvisit by creation order in increasing order")

            if (H5Lvisit(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, link_visit_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type creation order in increasing order failed\n");
                PART_ERROR(H5Lvisit_no_cycles_link_creation_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_no_cycles_link_creation_increasing);

        PART_BEGIN(H5Lvisit_no_cycles_link_creation_decreasing) {
            TESTING_2("H5Lvisit by creation order in decreasing order")

            if (H5Lvisit(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, link_visit_callback1, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Lvisit_no_cycles_link_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_no_cycles_link_creation_decreasing);

        /*
         * Make sure to reset the special counter.
         */
        i = 0;

        PART_BEGIN(H5Lvisit_by_name_no_cycles_link_name_increasing) {
            TESTING_2("H5Lvisit_by_name by link name in increasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, link_visit_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type name in increasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_no_cycles_link_name_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_no_cycles_link_name_increasing);

        PART_BEGIN(H5Lvisit_by_name_no_cycles_link_name_decreasing) {
            TESTING_2("H5Lvisit_by_name by link name in decreasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_DEC, link_visit_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type name in decreasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_no_cycles_link_name_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_no_cycles_link_name_decreasing);

        PART_BEGIN(H5Lvisit_by_name_no_cycles_link_creation_increasing) {
            TESTING_2("H5Lvisit_by_name by creation order in increasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_INC, link_visit_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type creation order in increasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_no_cycles_link_creation_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_no_cycles_link_creation_increasing);

        PART_BEGIN(H5Lvisit_by_name_no_cycles_link_creation_decreasing) {
            TESTING_2("H5Lvisit_by_name by creation order in decreasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_DEC, link_visit_callback1, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_no_cycles_link_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_no_cycles_link_creation_decreasing);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Gclose(subgroup1) < 0)
        TEST_ERROR
    if (H5Gclose(subgroup2) < 0)
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
        H5Gclose(subgroup1);
        H5Gclose(subgroup2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check the functionality of recursive
 * link iteration using H5Lvisit where there are
 * cyclic links. Iteration is done in increasing
 * and decreasing order of both link name and link
 * creation order.
 */
static int
test_link_visit_cycles(void)
{
    htri_t link_exists;
    size_t i;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  subgroup1 = H5I_INVALID_HID, subgroup2 = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link visiting with cycles");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME);
        goto error;
    }

    if ((subgroup1 = H5Gcreate2(group_id, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first subgroup '%s'\n", LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2);
        goto error;
    }

    if ((subgroup2 = H5Gcreate2(group_id, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second subgroup '%s'\n", LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3);
        goto error;
    }

    if (H5Lcreate_hard(group_id, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2, subgroup1, LINK_VISIT_TEST_CYCLE_LINK_NAME1,
            H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first hard link '%s'\n", LINK_VISIT_TEST_CYCLE_LINK_NAME1);
        goto error;
    }

    if (H5Lcreate_soft("/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME "/" LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2,
            subgroup1, LINK_VISIT_TEST_CYCLE_LINK_NAME2, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", LINK_VISIT_TEST_CYCLE_LINK_NAME2);
        goto error;
    }

    if (H5Lcreate_external(ext_link_filename, "/", subgroup2, LINK_VISIT_TEST_CYCLE_LINK_NAME3, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create external link '%s'\n", LINK_VISIT_TEST_CYCLE_LINK_NAME3);
        goto error;
    }

    if (H5Lcreate_hard(group_id, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3, subgroup2, LINK_VISIT_TEST_CYCLE_LINK_NAME4,
            H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second hard link '%s'\n", LINK_VISIT_TEST_CYCLE_LINK_NAME4);
        goto error;
    }

    /* Verify the links have been created */
    if ((link_exists = H5Lexists(subgroup1, LINK_VISIT_TEST_CYCLE_LINK_NAME1, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if first link '%s' exists\n", LINK_VISIT_TEST_CYCLE_LINK_NAME1);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    first link '%s' did not exist\n", LINK_VISIT_TEST_CYCLE_LINK_NAME1);
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup1, LINK_VISIT_TEST_CYCLE_LINK_NAME2, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if second link '%s' exists\n", LINK_VISIT_TEST_CYCLE_LINK_NAME2);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    second link '%s' did not exist\n", LINK_VISIT_TEST_CYCLE_LINK_NAME2);
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup2, LINK_VISIT_TEST_CYCLE_LINK_NAME3, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if third link '%s' exists\n", LINK_VISIT_TEST_CYCLE_LINK_NAME3);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    third link '%s' did not exist\n", LINK_VISIT_TEST_CYCLE_LINK_NAME3);
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup2, LINK_VISIT_TEST_CYCLE_LINK_NAME4, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if fourth link '%s' exists\n", LINK_VISIT_TEST_CYCLE_LINK_NAME4);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    fourth link '%s' did not exist\n", LINK_VISIT_TEST_CYCLE_LINK_NAME4);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        /*
         * NOTE: Pass a counter to the iteration callback to try to match up the
         * expected links with a given step throughout all of the following
         * iterations. This is to try and check that the links are indeed being
         * returned in the correct order.
         */
        i = 0;

        PART_BEGIN(H5Lvisit_cycles_link_name_increasing) {
            TESTING_2("H5Lvisit by link name in increasing order")

            if (H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_INC, link_visit_callback2, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type name in increasing order failed\n");
                PART_ERROR(H5Lvisit_cycles_link_name_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_cycles_link_name_increasing);

        PART_BEGIN(H5Lvisit_cycles_link_name_decreasing) {
            TESTING_2("H5Lvisit by link name in decreasing order")

            if (H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_DEC, link_visit_callback2, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type name in decreasing order failed\n");
                PART_ERROR(H5Lvisit_cycles_link_name_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_cycles_link_name_decreasing);

        PART_BEGIN(H5Lvisit_cycles_link_creation_increasing) {
            TESTING_2("H5Lvisit by creation order in increasing order")

            if (H5Lvisit(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, link_visit_callback2, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type creation order in increasing order failed\n");
                PART_ERROR(H5Lvisit_cycles_link_creation_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_cycles_link_creation_increasing);

        PART_BEGIN(H5Lvisit_cycles_link_creation_decreasing) {
            TESTING_2("H5Lvisit by creation order in decreasing order")

            if (H5Lvisit(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, link_visit_callback2, &i) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Lvisit_cycles_link_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_cycles_link_creation_decreasing);

        /*
         * Make sure to reset the special counter.
         */
        i = 0;

        PART_BEGIN(H5Lvisit_by_name_cycles_link_name_increasing) {
            TESTING_2("H5Lvisit_by_name by link name in increasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, link_visit_callback2, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type name in increasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_cycles_link_name_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_cycles_link_name_increasing);

        PART_BEGIN(H5Lvisit_by_name_cycles_link_name_decreasing) {
            TESTING_2("H5Lvisit_by_name by link name in decreasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_DEC, link_visit_callback2, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type name in decreasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_cycles_link_name_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_cycles_link_name_decreasing);

        PART_BEGIN(H5Lvisit_by_name_cycles_link_creation_increasing) {
            TESTING_2("H5Lvisit_by_name by creation order in increasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_INC, link_visit_callback2, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type creation order in increasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_cycles_link_creation_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_cycles_link_creation_increasing);

        PART_BEGIN(H5Lvisit_by_name_cycles_link_creation_decreasing) {
            TESTING_2("H5Lvisit_by_name by creation order in decreasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_DEC, link_visit_callback2, &i, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_cycles_link_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_cycles_link_creation_decreasing);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Gclose(subgroup1) < 0)
        TEST_ERROR
    if (H5Gclose(subgroup2) < 0)
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
        H5Gclose(subgroup1);
        H5Gclose(subgroup2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

static int
test_link_visit_invalid_params(void)
{
    herr_t err_ret = -1;
    htri_t link_exists;
    hid_t  file_id = H5I_INVALID_HID;
    hid_t  container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t  subgroup1 = H5I_INVALID_HID, subgroup2 = H5I_INVALID_HID;
    hid_t  dset_id = H5I_INVALID_HID;
    hid_t  dset_dtype = H5I_INVALID_HID;
    hid_t  fspace_id = H5I_INVALID_HID;
    char   ext_link_filename[VOL_TEST_FILENAME_MAX_LENGTH];

    TESTING_MULTIPART("link visiting with invalid parameters");

    TESTING_2("test setup")

    HDsnprintf(ext_link_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s", EXTERNAL_LINK_TEST_FILE_NAME);

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME);
        goto error;
    }

    if ((subgroup1 = H5Gcreate2(group_id, LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME2, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first subgroup '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME2);
        goto error;
    }

    if ((subgroup2 = H5Gcreate2(group_id, LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME3, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second subgroup '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME3);
        goto error;
    }

    if ((dset_dtype = generate_random_datatype(H5T_NO_CLASS)) < 0)
        TEST_ERROR

    if ((fspace_id = generate_random_dataspace(LINK_VISIT_INVALID_PARAMS_TEST_DSET_SPACE_RANK, NULL, NULL)) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(subgroup1, LINK_VISIT_INVALID_PARAMS_TEST_DSET_NAME, dset_dtype, fspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first dataset '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_DSET_NAME);
    }

    if (H5Dclose(dset_id) < 0)
        TEST_ERROR

    if ((dset_id = H5Dcreate2(subgroup2, LINK_VISIT_INVALID_PARAMS_TEST_DSET_NAME, dset_dtype, fspace_id,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second dataset '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_DSET_NAME);
    }

    if (H5Dclose(dset_id) < 0)
        TEST_ERROR

    if (H5Lcreate_hard(subgroup1, LINK_VISIT_INVALID_PARAMS_TEST_DSET_NAME, subgroup1, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME1,
            H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first hard link '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME1);
        goto error;
    }

    if (H5Lcreate_soft(LINK_VISIT_INVALID_PARAMS_TEST_DSET_NAME, subgroup1, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME2, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create soft link '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME2);
        goto error;
    }

    if (H5Lcreate_external(ext_link_filename, "/", subgroup2, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME3, H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create external link '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME3);
        goto error;
    }

    if (H5Lcreate_hard(subgroup2, LINK_VISIT_INVALID_PARAMS_TEST_DSET_NAME, subgroup2, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME4,
            H5P_DEFAULT, H5P_DEFAULT) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second hard link '%s'\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME4);
        goto error;
    }

    /* Verify the links have been created */
    if ((link_exists = H5Lexists(subgroup1, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME1, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if first link '%s' exists\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME1);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 1 did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup1, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME2, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if second link '%s' exists\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME2);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 2 did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup2, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME3, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if third link '%s' exists\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME3);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 3 did not exist\n");
        goto error;
    }

    if ((link_exists = H5Lexists(subgroup2, LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME4, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't determine if fourth link '%s' exists\n", LINK_VISIT_INVALID_PARAMS_TEST_LINK_NAME4);
        goto error;
    }

    if (!link_exists) {
        H5_FAILED();
        HDprintf("    link 4 did not exist\n");
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lvisit_invalid_grp_id) {
            TESTING_2("H5Lvisit with an invalid group ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit(H5I_INVALID_HID, H5_INDEX_NAME, H5_ITER_INC, link_visit_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit succeeded with an invalid group ID!\n");
                PART_ERROR(H5Lvisit_invalid_grp_id);
            }

            PASSED();
        } PART_END(H5Lvisit_invalid_grp_id);

        PART_BEGIN(H5Lvisit_invalid_index_type) {
            TESTING_2("H5Lvisit with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit(group_id, H5_INDEX_UNKNOWN, H5_ITER_INC, link_visit_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Lvisit_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit(group_id, H5_INDEX_N, H5_ITER_INC, link_visit_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Lvisit_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Lvisit_invalid_index_type);

        PART_BEGIN(H5Lvisit_invalid_iter_order) {
            TESTING_2("H5Lvisit with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_UNKNOWN, link_visit_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Lvisit_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_N, link_visit_callback3, NULL);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Lvisit_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Lvisit_invalid_iter_order);

        PART_BEGIN(H5Lvisit_by_name_invalid_loc_id) {
            TESTING_2("H5Lvisit_by_name with an invalid location ID")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(H5I_INVALID_HID, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_INC, link_visit_callback1, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with an invalid location ID!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_loc_id);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_invalid_loc_id);

        PART_BEGIN(H5Lvisit_by_name_invalid_grp_name) {
            TESTING_2("H5Lvisit_by_name with an invalid group name")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(file_id, NULL, H5_INDEX_NAME, H5_ITER_INC, link_visit_callback1, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with a NULL group name!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_grp_name);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(file_id, "", H5_INDEX_NAME, H5_ITER_INC, link_visit_callback1, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with an invalid group name of ''!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_grp_name);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_invalid_grp_name);

        PART_BEGIN(H5Lvisit_by_name_invalid_index_type) {
            TESTING_2("H5Lvisit_by_name with an invalid index type")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_UNKNOWN, H5_ITER_INC, link_visit_callback1, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with invalid index type H5_INDEX_UNKNOWN!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_index_type);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_N, H5_ITER_INC, link_visit_callback1, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with invalid index type H5_INDEX_N!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_index_type);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_invalid_index_type);

        PART_BEGIN(H5Lvisit_by_name_invalid_iter_order) {
            TESTING_2("H5Lvisit_by_name with an invalid iteration ordering")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_UNKNOWN, link_visit_callback1, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with invalid iteration ordering H5_ITER_UNKNOWN!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_iter_order);
            }

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_N, link_visit_callback1, NULL, H5P_DEFAULT);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with invalid iteration ordering H5_ITER_N!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_iter_order);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_invalid_iter_order);

        PART_BEGIN(H5Lvisit_by_name_invalid_lapl) {
            TESTING_2("H5Lvisit_by_name with an invalid LAPL")

            H5E_BEGIN_TRY {
                err_ret = H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME,
                        H5_INDEX_NAME, H5_ITER_INC, link_visit_callback1, NULL, H5I_INVALID_HID);
            } H5E_END_TRY;

            if (err_ret >= 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name succeeded with an invalid LAPL!\n");
                PART_ERROR(H5Lvisit_by_name_invalid_lapl);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_invalid_lapl);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Sclose(fspace_id) < 0)
        TEST_ERROR
    if (H5Tclose(dset_dtype) < 0)
        TEST_ERROR
    if (H5Gclose(subgroup1) < 0)
        TEST_ERROR
    if (H5Gclose(subgroup2) < 0)
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
        H5Gclose(subgroup1);
        H5Gclose(subgroup2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * A test to check that recursive link iteration
 * performed on a group with no links in it is
 * not problematic.
 */
static int
test_link_visit_0_links(void)
{
    hid_t file_id = H5I_INVALID_HID;
    hid_t container_group = H5I_INVALID_HID, group_id = H5I_INVALID_HID;
    hid_t subgroup1 = H5I_INVALID_HID, subgroup2 = H5I_INVALID_HID;

    TESTING_MULTIPART("link visiting on group with subgroups containing 0 links");

    TESTING_2("test setup")

    if ((file_id = H5Fopen(vol_test_filename, H5F_ACC_RDWR, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open file '%s'\n", vol_test_filename);
        goto error;
    }

    if ((container_group = H5Gopen2(file_id, LINK_TEST_GROUP_NAME, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't open container group '%s'\n", LINK_TEST_GROUP_NAME);
        goto error;
    }

    if ((group_id = H5Gcreate2(container_group, LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create container subgroup '%s'\n", LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME);
        goto error;
    }

    if ((subgroup1 = H5Gcreate2(group_id, LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME2,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create first subgroup '%s'\n", LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME2);
        goto error;
    }

    if ((subgroup2 = H5Gcreate2(group_id, LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME3,
            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT)) < 0) {
        H5_FAILED();
        HDprintf("    couldn't create second subgroup '%s'\n", LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME3);
        goto error;
    }

    PASSED();

    BEGIN_MULTIPART {
        PART_BEGIN(H5Lvisit_0_links_name_increasing) {
            TESTING_2("H5Lvisit by link name in increasing order")

            if (H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_INC, link_visit_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type name in increasing order failed\n");
                PART_ERROR(H5Lvisit_0_links_name_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_0_links_name_increasing);

        PART_BEGIN(H5Lvisit_0_links_name_decreasing) {
            TESTING_2("H5Lvisit by link name in decreasing order")

            if (H5Lvisit(group_id, H5_INDEX_NAME, H5_ITER_DEC, link_visit_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type name in decreasing order failed\n");
                PART_ERROR(H5Lvisit_0_links_name_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_0_links_name_decreasing);

        PART_BEGIN(H5Lvisit_0_links_creation_increasing) {
            TESTING_2("H5Lvisit by creation order in increasing order")

            if (H5Lvisit(group_id, H5_INDEX_CRT_ORDER, H5_ITER_INC, link_visit_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type creation order in increasing order failed\n");
                PART_ERROR(H5Lvisit_0_links_creation_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_0_links_creation_increasing);

        PART_BEGIN(H5Lvisit_0_links_creation_decreasing) {
            TESTING_2("H5Lvisit by creation order in decreasing order")

            if (H5Lvisit(group_id, H5_INDEX_CRT_ORDER, H5_ITER_DEC, link_visit_callback3, NULL) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Lvisit_0_links_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_0_links_creation_decreasing);

        PART_BEGIN(H5Lvisit_by_name_0_links_name_increasing) {
            TESTING_2("H5Lvisit_by_name by link name in increasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_INC, link_visit_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type name in increasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_0_links_name_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_0_links_name_increasing);

        PART_BEGIN(H5Lvisit_by_name_0_links_name_decreasing) {
            TESTING_2("H5Lvisit_by_name by link name in decreasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_NAME, H5_ITER_DEC, link_visit_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type name in decreasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_0_links_name_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_0_links_name_decreasing);

        PART_BEGIN(H5Lvisit_by_name_0_links_creation_increasing) {
            TESTING_2("H5Lvisit_by_name by creation order in increasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_INC, link_visit_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type creation order in increasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_0_links_creation_increasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_0_links_creation_increasing);

        PART_BEGIN(H5Lvisit_by_name_0_links_creation_decreasing) {
            TESTING_2("H5Lvisit_by_name by creation order in decreasing order")

            if (H5Lvisit_by_name(file_id, "/" LINK_TEST_GROUP_NAME "/" LINK_VISIT_TEST_0_LINKS_SUBGROUP_NAME,
                    H5_INDEX_CRT_ORDER, H5_ITER_DEC, link_visit_callback3, NULL, H5P_DEFAULT) < 0) {
                H5_FAILED();
                HDprintf("    H5Lvisit_by_name by index type creation order in decreasing order failed\n");
                PART_ERROR(H5Lvisit_by_name_0_links_creation_decreasing);
            }

            PASSED();
        } PART_END(H5Lvisit_by_name_0_links_creation_decreasing);
    } END_MULTIPART;

    TESTING_2("test cleanup")

    if (H5Gclose(subgroup1) < 0)
        TEST_ERROR
    if (H5Gclose(subgroup2) < 0)
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
        H5Gclose(subgroup1);
        H5Gclose(subgroup2);
        H5Gclose(group_id);
        H5Gclose(container_group);
        H5Fclose(file_id);
    } H5E_END_TRY;

    return 1;
}

/*
 * Link iteration callback to simply iterate through all of the links in a
 * group and check to make sure their names and link classes match what is
 * expected.
 */
static herr_t
link_iter_callback1(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data)
{
    size_t *i = (size_t *) op_data;
    size_t  counter_val = *((size_t *) op_data);
    herr_t  ret_val = 0;

    UNUSED(group_id);
    UNUSED(op_data);

    if (!HDstrcmp(name, LINK_ITER_TEST_HARD_LINK_NAME) &&
            (counter_val == 1 || counter_val == 4 || counter_val == 6 || counter_val == 11)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n", name);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_ITER_TEST_SOFT_LINK_NAME) &&
            (counter_val == 2 || counter_val == 3 || counter_val == 7 || counter_val == 10)) {
        if (H5L_TYPE_SOFT != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_SOFT!\n", name);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_ITER_TEST_EXT_LINK_NAME) &&
            (counter_val == 0 || counter_val == 5 || counter_val == 8 || counter_val == 9)) {
        if (H5L_TYPE_EXTERNAL != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_EXTERNAL!\n", name);
        }

        goto done;
    }

    HDprintf("    link name '%s' didn't match known names or came in an incorrect order\n", name);

    ret_val = -1;

done:
    (*i)++;

    return ret_val;
}

/*
 * Link iteration callback to test that the index-saving behavior of H5Literate
 * works correctly.
 */
static herr_t
link_iter_callback2(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data)
{
    int *broken = (int *) op_data;

    UNUSED(group_id);

    if (broken && !*broken && !HDstrcmp(name, LINK_ITER_TEST_SOFT_LINK_NAME)) {
        return (*broken = 1);
    }

    if (!HDstrcmp(name, LINK_ITER_TEST_HARD_LINK_NAME)) {
        if (H5L_TYPE_HARD != info->type) {
            H5_FAILED();
            HDprintf("    link type did not match\n");
            goto error;
        }
    }
    else if (!HDstrcmp(name, LINK_ITER_TEST_SOFT_LINK_NAME)) {
        if (H5L_TYPE_SOFT != info->type) {
            H5_FAILED();
            HDprintf("    link type did not match\n");
            goto error;
        }
    }
    else if (!HDstrcmp(name, LINK_ITER_TEST_EXT_LINK_NAME)) {
        if (H5L_TYPE_EXTERNAL != info->type) {
            H5_FAILED();
            HDprintf("    link type did not match\n");
            goto error;
        }
    }
    else {
        H5_FAILED();
        HDprintf("    link name didn't match known names\n");
        goto error;
    }

    return 0;

error:
    return -1;
}

static herr_t
link_iter_callback3(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data)
{
    UNUSED(group_id);
    UNUSED(name);
    UNUSED(info);
    UNUSED(op_data);

    return 0;
}

/*
 * Link visit callback to simply iterate recursively through all of the links in a
 * group and check to make sure their names and link classes match what is expected.
 */
static herr_t
link_visit_callback1(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data)
{
    size_t *i = (size_t *) op_data;
    size_t  counter_val = *((size_t *) op_data);
    herr_t  ret_val = 0;

    UNUSED(group_id);
    UNUSED(op_data);

    if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME1) &&
            (counter_val == 2 || counter_val == 14 || counter_val == 18 || counter_val == 30)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME1);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME2) &&
            (counter_val == 3 || counter_val == 13 || counter_val == 19 || counter_val == 29)) {
        if (H5L_TYPE_SOFT != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_SOFT!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME2);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME3) &&
            (counter_val == 6 || counter_val == 10 || counter_val == 22 || counter_val == 26)) {
        if (H5L_TYPE_EXTERNAL != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_EXTERNAL!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME3);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME4) &&
            (counter_val == 7 || counter_val == 9 || counter_val == 23 || counter_val == 25)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_NO_CYCLE_LINK_NAME4);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_NO_CYCLE_DSET_NAME) &&
            (counter_val == 1 || counter_val == 15 || counter_val == 17 || counter_val == 31)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_NO_CYCLE_DSET_NAME);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_NO_CYCLE_DSET_NAME2) &&
            (counter_val == 5 || counter_val == 11 || counter_val == 21 || counter_val == 27)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_NO_CYCLE_DSET_NAME2);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2) &&
            (counter_val == 0 || counter_val == 12 || counter_val == 16 || counter_val == 28)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME2);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3) &&
            (counter_val == 4 || counter_val == 8 || counter_val == 20 || counter_val == 24)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_NO_CYCLE_SUBGROUP_NAME3);
        }

        goto done;
    }

    HDprintf("    link name '%s' didn't match known names or came in an incorrect order\n", name);

    ret_val = -1;

done:
    (*i)++;

    return ret_val;
}

static herr_t
link_visit_callback2(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data)
{
    size_t *i = (size_t *) op_data;
    size_t  counter_val = *((size_t *) op_data);
    herr_t  ret_val = 0;

    UNUSED(group_id);
    UNUSED(op_data);

    if (!HDstrcmp(name, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME1) &&
            (counter_val == 1 || counter_val == 11 || counter_val == 13 || counter_val == 23)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME1);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME2) &&
            (counter_val == 2 || counter_val == 10 || counter_val == 14 || counter_val == 22)) {
        if (H5L_TYPE_SOFT != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_SOFT!\n",
                    LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME2);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME3) &&
            (counter_val == 4 || counter_val == 8 || counter_val == 16 || counter_val == 20)) {
        if (H5L_TYPE_EXTERNAL != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_EXTERNAL!\n",
                    LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME3);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME4) &&
            (counter_val == 5 || counter_val == 7 || counter_val == 17 || counter_val == 19)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n",
                    LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3 "/" LINK_VISIT_TEST_CYCLE_LINK_NAME4);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2) &&
            (counter_val == 0 || counter_val == 9 || counter_val == 12 || counter_val == 21)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n", LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME2);
        }

        goto done;
    }
    else if (!HDstrcmp(name, LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3) &&
            (counter_val == 3 || counter_val == 6 || counter_val == 15 || counter_val == 18)) {
        if (H5L_TYPE_HARD != info->type) {
            ret_val = -1;
            HDprintf("    link type for link '%s' was not H5L_TYPE_HARD!\n", LINK_VISIT_TEST_CYCLE_SUBGROUP_NAME3);
        }

        goto done;
    }

    HDprintf("    link name '%s' didn't match known names or came in an incorrect order\n", name);

    ret_val = -1;

done:
    (*i)++;

    return ret_val;
}

static herr_t
link_visit_callback3(hid_t group_id, const char *name, const H5L_info_t *info, void *op_data)
{
    UNUSED(group_id);
    UNUSED(name);
    UNUSED(info);
    UNUSED(op_data);

    return 0;
}

int
vol_link_test(void)
{
    size_t i;
    int    nerrors;

    HDprintf("**********************************************\n");
    HDprintf("*                                            *\n");
    HDprintf("*              VOL Link Tests                *\n");
    HDprintf("*                                            *\n");
    HDprintf("**********************************************\n\n");

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(link_tests); i++) {
        nerrors += (*link_tests[i])() ? 1 : 0;
    }

    HDprintf("\n");

done:
    return nerrors;
}
