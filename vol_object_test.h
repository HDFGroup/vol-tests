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

#ifndef VOL_OBJECT_TEST_H
#define VOL_OBJECT_TEST_H

#include "vol_test.h"

int vol_object_test(void);

/***********************************************
 *                                             *
 *      VOL connector Object test defines      *
 *                                             *
 ***********************************************/

#define OBJECT_OPEN_TEST_SPACE_RANK 2
#define OBJECT_OPEN_TEST_GROUP_NAME "object_open_test"
#define OBJECT_OPEN_TEST_GRP_NAME   "object_open_test_group"
#define OBJECT_OPEN_TEST_DSET_NAME  "object_open_test_dset"
#define OBJECT_OPEN_TEST_TYPE_NAME  "object_open_test_type"

#define OBJECT_OPEN_INVALID_PARAMS_TEST_GROUP_NAME "object_open_invalid_params_test"
#define OBJECT_OPEN_INVALID_PARAMS_TEST_GRP_NAME   "object_open_invalid_params_test_group"

#define OBJECT_CLOSE_INVALID_TEST_GROUP_NAME "object_close_invalid_params_test"
#define OBJECT_CLOSE_INVALID_TEST_ATTRIBUTE_NAME "object_close_invalid_test_attribute"
#define OBJECT_CLOSE_INVALID_TEST_SPACE_RANK 2

#define OBJECT_EXISTS_TEST_DSET_SPACE_RANK    2
#define OBJECT_EXISTS_TEST_SUBGROUP_NAME      "object_exists_test"
#define OBJECT_EXISTS_TEST_DANGLING_LINK_NAME "object_exists_test_dangling_soft_link"
#define OBJECT_EXISTS_TEST_SOFT_LINK_NAME     "object_exists_test_soft_link"
#define OBJECT_EXISTS_TEST_GRP_NAME           "object_exists_test_group"
#define OBJECT_EXISTS_TEST_TYPE_NAME          "object_exists_test_type"
#define OBJECT_EXISTS_TEST_DSET_NAME          "object_exists_test_dset"

#define OBJECT_EXISTS_INVALID_PARAMS_TEST_SUBGROUP_NAME "object_exists_invalid_params_test"
#define OBJECT_EXISTS_INVALID_PARAMS_TEST_GRP_NAME      "object_exists_invalid_params_test_group"

#define OBJECT_COPY_TEST_SHALLOW_GROUP_NO_ATTRS_NAME "object_copy_test_shallow_group_no_attrs"
#define OBJECT_COPY_TEST_SHALLOW_GROUP_ATTRS_NAME    "object_copy_test_shallow_group_with_attrs"
#define OBJECT_COPY_TEST_DEEP_GROUP_NO_ATTRS_NAME    "object_copy_test_deep_group_no_attrs"
#define OBJECT_COPY_TEST_DEEP_GROUP_ATTRS_NAME       "object_copy_test_deep_group_with_attrs"
#define OBJECT_COPY_TEST_FILE_COPY_GROUP_NAME        "object_copy_test_group_from_other_file"
#define OBJECT_COPY_TEST_FILE_COPY_DSET_NAME         "object_copy_test_dset_from_other_file"
#define OBJECT_COPY_TEST_FILE_COPY_TYPE_NAME         "object_copy_test_type_from_other_file"
#define OBJECT_COPY_TEST_DANGLING_LINK_NAME          "object_copy_test_dangling_link"
#define OBJECT_COPY_TEST_DSET_NO_ATTRS_NAME          "object_copy_test_dset_no_attrs"
#define OBJECT_COPY_TEST_TYPE_NO_ATTRS_NAME          "object_copy_test_type_no_attrs"
#define OBJECT_COPY_TEST_DSET_ATTRS_NAME             "object_copy_test_dset_attrs"
#define OBJECT_COPY_TEST_TYPE_ATTRS_NAME             "object_copy_test_type_attrs"
#define OBJECT_COPY_TEST_NUM_NESTED_OBJS             3
#define OBJECT_COPY_TEST_SUBGROUP_NAME               "object_copy_test"
#define OBJECT_COPY_TEST_SPACE_RANK                  2
#define OBJECT_COPY_TEST_GROUP_NAME                  "object_copy_test_group"
#define OBJECT_COPY_TEST_DSET_NAME                   "object_copy_test_dset"
#define OBJECT_COPY_TEST_TYPE_NAME                   "object_copy_test_type"
#define OBJECT_COPY_TEST_FILE_NAME                   "object_copy_test_file.h5"
#define OBJECT_COPY_TEST_NUM_ATTRS                   3

#define OBJECT_COPY_INVALID_PARAMS_TEST_SUBGROUP_NAME "object_copy_invalid_params_test"
#define OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME    "object_copy_invalid_params_group"
#define OBJECT_COPY_INVALID_PARAMS_TEST_GROUP_NAME2   "object_copy_invalid_params_group_copy"

#define OBJECT_VISIT_TEST_SUBGROUP_NAME "object_visit_test"
#define OBJECT_VISIT_TEST_SPACE_RANK    2
#define OBJECT_VISIT_TEST_GROUP_NAME    "object_visit_test_group"
#define OBJECT_VISIT_TEST_DSET_NAME     "object_visit_test_dset"
#define OBJECT_VISIT_TEST_TYPE_NAME     "object_visit_test_type"
#define OBJECT_VISIT_TEST_NUM_OBJS      4 /* Should be one more than the number of objects */

#define OBJECT_VISIT_DANGLING_LINK_TEST_SUBGROUP_NAME "object_visit_dangling_link_test"
#define OBJECT_VISIT_DANGLING_LINK_TEST_LINK_NAME1    "dangling_link1"
#define OBJECT_VISIT_DANGLING_LINK_TEST_LINK_NAME2    "dangling_link2"
#define OBJECT_VISIT_DANGLING_LINK_TEST_LINK_NAME3    "dangling_link3"

#define OBJECT_VISIT_INVALID_PARAMS_TEST_SUBGROUP_NAME "object_visit_invalid_params_test"
#define OBJECT_VISIT_INVALID_PARAMS_TEST_GROUP_NAME    "object_visit_invalid_params_group"

#define OBJECT_CLOSE_TEST_SPACE_RANK 2
#define OBJECT_CLOSE_TEST_GROUP_NAME "object_close_test"
#define OBJECT_CLOSE_TEST_GRP_NAME   "object_close_test_group"
#define OBJECT_CLOSE_TEST_DSET_NAME  "object_close_test_dset"
#define OBJECT_CLOSE_TEST_TYPE_NAME  "object_close_test_type"

#define OBJECT_LINK_TEST_GROUP_NAME  "object_link_test_group"
#define OBJECT_LINK_TEST_GROUP_NAME2 "object_link_test_group_link"
#define OBJECT_LINK_TEST_DSET_NAME   "object_link_test_dataset"
#define OBJECT_LINK_TEST_DTYPE_NAME  "object_link_test_datatype"
#define OBJECT_LINK_TEST_SPACE_RANK  2

#define OBJECT_LINK_INVALID_PARAMS_TEST_GROUP_NAME  "object_link_invalid_params_test_group"

#define OBJ_REF_GET_TYPE_TEST_SUBGROUP_NAME "obj_ref_get_obj_type_test"
#define OBJ_REF_GET_TYPE_TEST_DSET_NAME "ref_dset"
#define OBJ_REF_GET_TYPE_TEST_TYPE_NAME "ref_dtype"
#define OBJ_REF_GET_TYPE_TEST_SPACE_RANK 2

#define OBJ_REF_DATASET_WRITE_TEST_SUBGROUP_NAME  "obj_ref_write_test"
#define OBJ_REF_DATASET_WRITE_TEST_REF_DSET_NAME  "ref_dset"
#define OBJ_REF_DATASET_WRITE_TEST_REF_TYPE_NAME  "ref_dtype"
#define OBJ_REF_DATASET_WRITE_TEST_SPACE_RANK     1
#define OBJ_REF_DATASET_WRITE_TEST_DSET_NAME      "obj_ref_dset"

#define OBJ_REF_DATASET_READ_TEST_SUBGROUP_NAME  "obj_ref_read_test"
#define OBJ_REF_DATASET_READ_TEST_REF_DSET_NAME  "ref_dset"
#define OBJ_REF_DATASET_READ_TEST_REF_TYPE_NAME  "ref_dtype"
#define OBJ_REF_DATASET_READ_TEST_SPACE_RANK     1
#define OBJ_REF_DATASET_READ_TEST_DSET_NAME      "obj_ref_dset"

#define OBJ_REF_DATASET_EMPTY_WRITE_TEST_SUBGROUP_NAME  "obj_ref_empty_write_test"
#define OBJ_REF_DATASET_EMPTY_WRITE_TEST_SPACE_RANK     1
#define OBJ_REF_DATASET_EMPTY_WRITE_TEST_DSET_NAME      "obj_ref_dset"

#define OBJECT_REF_COUNT_TEST_SUBGROUP_NAME   "ref_count_test"
#define OBJECT_REF_COUNT_TEST_GRP_NAME        "ref_count_test_group"
#define OBJECT_REF_COUNT_TEST_DSET_NAME       "ref_count_dset"
#define OBJECT_REF_COUNT_TEST_TYPE_NAME       "ref_count_dtype"
#define OBJECT_REF_COUNT_TEST_DSET_SPACE_RANK 2

#endif
