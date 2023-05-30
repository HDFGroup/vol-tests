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

/*
 * A test suite which only makes public HDF5 API calls and which is meant
 * to test the native VOL connector or a specified HDF5 VOL connector (or
 * set of connectors stacked with each other). This test suite must assume
 * that a VOL connector could only implement the File interface. Therefore,
 * the suite should check that a particular piece of functionality is supported
 * by the VOL connector before actually testing it. If the functionality is
 * not supported, the test should simply be skipped, perhaps with a note as
 * to why the test was skipped, if possible.
 *
 * If the VOL connector being used supports the creation of groups, this
 * test suite will attempt to organize the output of these various tests
 * into groups based on their respective HDF5 interface.
 */

#include "vol_test.h"
#include "vol_test_util.h"

#include "vol_file_test.h"
#include "vol_group_test.h"
#include "vol_dataset_test.h"
#include "vol_datatype_test.h"
#include "vol_attribute_test.h"
#include "vol_link_test.h"
#include "vol_object_test.h"
#include "vol_misc_test.h"
#ifdef H5VL_TEST_HAS_ASYNC
#include "vol_async_test.h"
#endif

char vol_test_filename[VOL_TEST_FILENAME_MAX_LENGTH];

const char *test_path_prefix;

size_t n_tests_run_g;
size_t n_tests_passed_g;
size_t n_tests_failed_g;
size_t n_tests_skipped_g;

uint64_t vol_cap_flags_g;

/* X-macro to define the following for each test:
 * - enum type
 * - name
 * - test function
 * - enabled by default
 */
#ifdef H5VL_TEST_HAS_ASYNC
#define VOL_TESTS                                                                                            \
    X(VOL_TEST_NULL, "", NULL, 0)                                                                            \
    X(VOL_TEST_FILE, "file", vol_file_test, 1)                                                               \
    X(VOL_TEST_GROUP, "group", vol_group_test, 1)                                                            \
    X(VOL_TEST_DATASET, "dataset", vol_dataset_test, 1)                                                      \
    X(VOL_TEST_DATATYPE, "datatype", vol_datatype_test, 1)                                                   \
    X(VOL_TEST_ATTRIBUTE, "attribute", vol_attribute_test, 1)                                                \
    X(VOL_TEST_LINK, "link", vol_link_test, 1)                                                               \
    X(VOL_TEST_OBJECT, "object", vol_object_test, 1)                                                         \
    X(VOL_TEST_MISC, "misc", vol_misc_test, 1)                                                               \
    X(VOL_TEST_ASYNC, "async", vol_async_test, 1)                                                            \
    X(VOL_TEST_MAX, "", NULL, 0)
#else
#define VOL_TESTS                                                                                            \
    X(VOL_TEST_NULL, "", NULL, 0)                                                                            \
    X(VOL_TEST_FILE, "file", vol_file_test, 1)                                                               \
    X(VOL_TEST_GROUP, "group", vol_group_test, 1)                                                            \
    X(VOL_TEST_DATASET, "dataset", vol_dataset_test, 1)                                                      \
    X(VOL_TEST_DATATYPE, "datatype", vol_datatype_test, 1)                                                   \
    X(VOL_TEST_ATTRIBUTE, "attribute", vol_attribute_test, 1)                                                \
    X(VOL_TEST_LINK, "link", vol_link_test, 1)                                                               \
    X(VOL_TEST_OBJECT, "object", vol_object_test, 1)                                                         \
    X(VOL_TEST_MISC, "misc", vol_misc_test, 1)                                                               \
    X(VOL_TEST_MAX, "", NULL, 0)
#endif

#define X(a, b, c, d) a,
enum vol_test_type { VOL_TESTS };
#undef X
#define X(a, b, c, d) b,
static char *const vol_test_name[] = {VOL_TESTS};
#undef X
#define X(a, b, c, d) c,
static int (*vol_test_func[])(void) = {VOL_TESTS};
#undef X
#define X(a, b, c, d) d,
static int vol_test_enabled[] = {VOL_TESTS};
#undef X

static enum vol_test_type
vol_test_name_to_type(const char *test_name)
{
    enum vol_test_type i = 0;

    while (strcmp(vol_test_name[i], test_name) && i != VOL_TEST_MAX)
        i++;

    return ((i == VOL_TEST_MAX) ? VOL_TEST_NULL : i);
}

static void
vol_test_run(void)
{
    enum vol_test_type i;

    for (i = VOL_TEST_FILE; i < VOL_TEST_MAX; i++)
        if (vol_test_enabled[i])
            (void)vol_test_func[i]();
}

/******************************************************************************/

int
main(int argc, char **argv)
{
    const char *vol_connector_string;
    const char *vol_connector_name;
    unsigned    seed;
    hid_t       fapl_id                   = H5I_INVALID_HID;
    hid_t       default_con_id            = H5I_INVALID_HID;
    hid_t       registered_con_id         = H5I_INVALID_HID;
    char       *vol_connector_string_copy = NULL;
    char       *vol_connector_info        = NULL;
    hbool_t     err_occurred              = FALSE;

    /* Simple argument checking, TODO can improve that later */
    if (argc > 1) {
        enum vol_test_type i = vol_test_name_to_type(argv[1]);
        if (i != VOL_TEST_NULL) {
            /* Run only specific VOL test */
            memset(vol_test_enabled, 0, sizeof(vol_test_enabled));
            vol_test_enabled[i] = 1;
        }
    }

#ifdef H5_HAVE_PARALLEL
    /* If HDF5 was built with parallel enabled, go ahead and call MPI_Init before
     * running these tests. Even though these are meant to be serial tests, they will
     * likely be run using mpirun (or similar) and we cannot necessarily expect HDF5 or
     * an HDF5 VOL connector to call MPI_Init.
     */
    MPI_Init(&argc, &argv);
#endif

    H5open();

    n_tests_run_g     = 0;
    n_tests_passed_g  = 0;
    n_tests_failed_g  = 0;
    n_tests_skipped_g = 0;

    seed = (unsigned)HDtime(NULL);
    srand(seed);

    if (NULL == (vol_connector_string = HDgetenv("HDF5_VOL_CONNECTOR"))) {
        HDprintf("No VOL connector selected; using native VOL connector\n");
        vol_connector_name = "native";
        vol_connector_info = NULL;
    }
    else {
        char *token;

        if (NULL == (vol_connector_string_copy = HDstrdup(vol_connector_string))) {
            HDfprintf(stderr, "Unable to copy VOL connector string\n");
            err_occurred = TRUE;
            goto done;
        }

        if (NULL == (token = HDstrtok(vol_connector_string_copy, " "))) {
            HDfprintf(stderr, "Error while parsing VOL connector string\n");
            err_occurred = TRUE;
            goto done;
        }

        vol_connector_name = token;

        if (NULL != (token = HDstrtok(NULL, " "))) {
            vol_connector_info = token;
        }
    }

    if (NULL == (test_path_prefix = HDgetenv(HDF5_API_TEST_PATH_PREFIX)))
        test_path_prefix = "";

    HDsnprintf(vol_test_filename, VOL_TEST_FILENAME_MAX_LENGTH, "%s%s", test_path_prefix, TEST_FILE_NAME);

    HDprintf("Running VOL tests with VOL connector '%s' and info string '%s'\n\n", vol_connector_name,
             vol_connector_info ? vol_connector_info : "");
    HDprintf("Test parameters:\n");
    HDprintf("  - Test file name: '%s'\n", vol_test_filename);
    HDprintf("  - Test seed: %u\n", seed);
    HDprintf("\n\n");

    if ((fapl_id = H5Pcreate(H5P_FILE_ACCESS)) < 0) {
        HDfprintf(stderr, "Unable to create FAPL\n");
        err_occurred = TRUE;
        goto done;
    }

    /*
     * If using a VOL connector other than the native
     * connector, check whether the VOL connector was
     * successfully registered before running the tests.
     * Otherwise, HDF5 will default to running the tests
     * with the native connector, which could be misleading.
     */
    if (0 != HDstrcmp(vol_connector_name, "native")) {
        htri_t is_registered;

        if ((is_registered = H5VLis_connector_registered_by_name(vol_connector_name)) < 0) {
            HDfprintf(stderr, "Unable to determine if VOL connector is registered\n");
            err_occurred = TRUE;
            goto done;
        }

        if (!is_registered) {
            HDfprintf(stderr, "Specified VOL connector '%s' wasn't correctly registered!\n",
                      vol_connector_name);
            err_occurred = TRUE;
            goto done;
        }
        else {
            /*
             * If the connector was successfully registered, check that
             * the connector ID set on the default FAPL matches the ID
             * for the registered connector before running the tests.
             */
            if (H5Pget_vol_id(fapl_id, &default_con_id) < 0) {
                HDfprintf(stderr, "Couldn't retrieve ID of VOL connector set on default FAPL\n");
                err_occurred = TRUE;
                goto done;
            }

            if ((registered_con_id = H5VLget_connector_id_by_name(vol_connector_name)) < 0) {
                HDfprintf(stderr, "Couldn't retrieve ID of registered VOL connector\n");
                err_occurred = TRUE;
                goto done;
            }

            if (default_con_id != registered_con_id) {
                HDfprintf(stderr, "VOL connector set on default FAPL didn't match specified VOL connector\n");
                err_occurred = TRUE;
                goto done;
            }
        }
    }

    /* Retrieve the VOL cap flags - work around an HDF5
     * library issue by creating a FAPL
     */
    vol_cap_flags_g = H5VL_CAP_FLAG_NONE;
    if (H5Pget_vol_cap_flags(fapl_id, &vol_cap_flags_g) < 0) {
        HDfprintf(stderr, "Unable to retrieve VOL connector capability flags\n");
        err_occurred = TRUE;
        goto done;
    }

    /*
     * Create the file that will be used for all of the tests,
     * except for those which test file creation.
     */
    if (create_test_container(vol_test_filename, vol_cap_flags_g) < 0) {
        HDfprintf(stderr, "Unable to create testing container file '%s'\n", vol_test_filename);
        err_occurred = TRUE;
        goto done;
    }

    /* Run all the tests that are enabled */
    vol_test_run();

    HDprintf("Cleaning up testing files\n");
    H5Fdelete(vol_test_filename, fapl_id);

    if (n_tests_run_g > 0) {
        HDprintf("%ld/%ld (%.2f%%) VOL tests passed with VOL connector '%s'\n", (long)n_tests_passed_g,
                 (long)n_tests_run_g, ((float)n_tests_passed_g / (float)n_tests_run_g * 100.0),
                 vol_connector_name);
        HDprintf("%ld/%ld (%.2f%%) VOL tests did not pass with VOL connector '%s'\n", (long)n_tests_failed_g,
                 (long)n_tests_run_g, ((float)n_tests_failed_g / (float)n_tests_run_g * 100.0),
                 vol_connector_name);
        HDprintf("%ld/%ld (%.2f%%) VOL tests were skipped with VOL connector '%s'\n", (long)n_tests_skipped_g,
                 (long)n_tests_run_g, ((float)n_tests_skipped_g / (float)n_tests_run_g * 100.0),
                 vol_connector_name);
    }

done:
    HDfree(vol_connector_string_copy);

    if (default_con_id >= 0 && H5VLclose(default_con_id) < 0) {
        HDfprintf(stderr, "Unable to close VOL connector ID\n");
        err_occurred = TRUE;
    }

    if (registered_con_id >= 0 && H5VLclose(registered_con_id) < 0) {
        HDfprintf(stderr, "Unable to close VOL connector ID\n");
        err_occurred = TRUE;
    }

    if (fapl_id >= 0 && H5Pclose(fapl_id) < 0) {
        HDfprintf(stderr, "Unable to close FAPL\n");
        err_occurred = TRUE;
    }

    H5close();

#ifdef H5_HAVE_PARALLEL
    MPI_Finalize();
#endif

    HDexit(((err_occurred || n_tests_failed_g > 0) ? EXIT_FAILURE : EXIT_SUCCESS));
}
