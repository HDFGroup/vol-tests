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

/* h5delete tool
 *
 * Deletes storage via H5Fdelete() using the VOL connector specified in the
 * environment variable.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>


void
usage(void)
{
    fprintf(stderr, "Usage: h5delete [-f] <filename>\n");
}

int
main(int argc, const char *argv[])
{
    bool        quiet = false;
    const char  *name = NULL;
    int         ret = 0;

    switch (argc) {
        case 3:
            if (strcmp(argv[1], "-f")) {
                usage();
                return EXIT_FAILURE;
            }
            quiet = true;
            name = argv[2];
            break;
        case 2:
            name = argv[1];
            break;
        default:
            usage();
            return EXIT_FAILURE;
    }

    H5E_BEGIN_TRY {
        /* Only uses the environment variable at this time */
        ret = (int)H5Fdelete(name, H5P_DEFAULT);
    } H5E_END_TRY;

    /* The native VOL connector does not implement the H5Fdelete() call
     * at this time, so try to remove the file via the POSIX remove(3)
     * call on failures.
     */
    if (ret < 0)
        ret  = remove(name);

    if (ret < 0 && !quiet)
            fprintf(stderr, "Unable to delete storage at: %s\n", name);

    return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
