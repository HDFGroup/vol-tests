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

#include "vol_attribute_test_parallel.h"



/*
 * The array of parallel attribute tests to be performed.
 */
static int (*par_attribute_tests[])(void) = {
    NULL
};

int
vol_attribute_test_parallel(void)
{
    size_t i;
    int    nerrors;

    HDprintf("**********************************************\n");
    HDprintf("*                                            *\n");
    HDprintf("*        VOL Parallel Attribute Tests        *\n");
    HDprintf("*                                            *\n");
    HDprintf("**********************************************\n\n");

    for (i = 0, nerrors = 0; i < ARRAY_LENGTH(par_attribute_tests); i++) {
        /* nerrors += (*par_attribute_tests[i])() ? 1 : 0; */
    }

    HDprintf("\n");

    return nerrors;
}
