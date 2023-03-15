/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Programmer:  Robb Matzke <matzke@llnl.gov>
 *    Friday, October 30, 1998
 *
 * Purpose:  This file is included by all HDF5 library source files to
 *    define common things which are not defined in the HDF5 API.
 *    The configuration constants like H5_HAVE_UNISTD_H etc. are
 *    defined in H5config.h which is included by H5public.h.
 *
 */

#ifndef _H5private_H
#define _H5private_H

#include "H5public.h"    /* Include Public Definitions    */

/* include the pthread header */
#ifdef H5_HAVE_THREADSAFE
 #ifdef H5_HAVE_WIN32_API
  #ifndef H5_HAVE_WIN_THREADS
   #ifdef H5_HAVE_PTHREAD_H
    #include <pthread.h>
   #endif /* H5_HAVE_PTHREAD_H */
  #endif /* H5_HAVE_WIN_THREADS */
 #else /* H5_HAVE_WIN32_API */
  #ifdef H5_HAVE_PTHREAD_H
   #include <pthread.h>
  #endif /* H5_HAVE_PTHREAD_H */
 #endif /* H5_HAVE_WIN32_API */
#endif /* H5_HAVE_THREADSAFE */

/*
 * Include ANSI-C header files.
 */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * If _POSIX_VERSION is defined in unistd.h then this system is Posix.1
 * compliant. Otherwise all bets are off.
 */
#ifdef H5_HAVE_UNISTD_H
#   include <sys/types.h>
#   include <unistd.h>
#endif
#ifdef _POSIX_VERSION
#   include <sys/wait.h>
#   include <pwd.h>
#endif

/*
 * C9x integer types
 */
#ifndef __cplusplus
#ifdef H5_HAVE_STDINT_H
#   include <stdint.h>
#endif
#endif

/*
 * The `struct stat' data type for stat() and fstat(). This is a Posix file
 * but often apears on non-Posix systems also.  The `struct stat' is required
 * for hdf5 to compile, although only a few fields are actually used.
 */
#ifdef H5_HAVE_SYS_STAT_H
#   include <sys/stat.h>
#endif

/*
 * If a program may include both `time.h' and `sys/time.h' then
 * TIME_WITH_SYS_TIME is defined (see AC_HEADER_TIME in configure.ac).
 * On some older systems, `sys/time.h' includes `time.h' but `time.h' is not
 * protected against multiple inclusion, so programs should not explicitly
 * include both files. This macro is useful in programs that use, for example,
 * `struct timeval' or `struct timezone' as well as `struct tm'.  It is best
 * used in conjunction with `HAVE_SYS_TIME_H', whose existence is checked
 * by `AC_CHECK_HEADERS(sys/time.h)' in configure.ac.
 */
#include <time.h>
#ifdef H5_HAVE_SYS_TIME_H
#   include <sys/time.h>
#endif


/*
 * Longjumps are used to detect alignment constrants
 */
#ifdef H5_HAVE_SETJMP_H
#   include <setjmp.h>
#endif

/*
 * flock() in sys/file.h is used for the implementation of file locking.
 */
#if defined(H5_HAVE_FLOCK) && defined(H5_HAVE_SYS_FILE_H)
#   include <sys/file.h>
#endif

/*
 * Resource usage is not Posix.1 but HDF5 uses it anyway for some performance
 * and debugging code if available.
 */
#ifdef H5_HAVE_SYS_RESOURCE_H
#   include <sys/resource.h>
#endif

/*
 * Unix ioctls.   These are used by h5ls (and perhaps others) to determine a
 * reasonable output width.
 */
#ifdef H5_HAVE_SYS_IOCTL_H
#   include <sys/ioctl.h>
#endif

/*
 * System information. These are needed on the DEC Alpha to turn off fixing
 * of unaligned accesses by the operating system during detection of
 * alignment constraints in H5detect.c:main().
 */
#ifdef H5_HAVE_SYS_SYSINFO_H
#   include <sys/sysinfo.h>
#endif
#ifdef H5_HAVE_SYS_PROC_H
#   include <sys/proc.h>
#endif
#ifdef H5_HAVE_IO_H
#   include <io.h>
#endif

/*
 * Dynamic library handling.  These are needed for dynamically loading I/O
 * filters and VFDs.
 */
#ifdef H5_HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#ifdef H5_HAVE_DIRENT_H
#include <dirent.h>
#endif

/* Define the default VFD for this platform.
 * Since the removal of the Windows VFD, this is sec2 for all platforms.
 */
#define H5_DEFAULT_VFD      H5FD_SEC2

/* Define the default VOL driver */
#define H5_DEFAULT_VOL      H5VL_NATIVE

#ifdef H5_HAVE_WIN32_API
/* The following two defines must be before any windows headers are included */
#define WIN32_LEAN_AND_MEAN    /* Exclude rarely-used stuff from Windows headers */
#define NOGDI                  /* Exclude Graphic Display Interface macros */

#ifdef H5_HAVE_WINSOCK2_H
#include <winsock2.h>
#endif

#ifdef H5_HAVE_THREADSAFE
#include <process.h>        /* For _beginthread() */
#endif

#include <windows.h>
#include <direct.h>         /* For _getcwd() */

#endif /*H5_HAVE_WIN32_API*/

/* Various ways that inline functions can be declared */
#if defined(H5_HAVE___INLINE__)
    /* GNU (alternative form) */
    #define H5_INLINE __inline__
#elif defined(H5_HAVE___INLINE)
    /* Visual Studio */
    #define H5_INLINE __inline
#elif defined(H5_HAVE_INLINE)
    /* GNU, C++
     * Use "inline" as a last resort on the off-chance that there will
     * be C++ problems.
     */
    #define H5_INLINE inline
#else
    #define H5_INLINE
#endif /* inline choices */

#ifndef F_OK
#   define F_OK  00
#   define W_OK 02
#   define R_OK 04
#endif

/*
 * dmalloc (debugging malloc) support
 */
#ifdef H5_HAVE_DMALLOC_H
#include "dmalloc.h"
#endif /* H5_HAVE_DMALLOC_H */

/*
 * NT doesn't define SIGBUS, but since NT only runs on processors
 * that do not have alignment constraints a SIGBUS would never be
 * raised, so we just replace it with SIGILL (which also should
 * never be raised by the hdf5 library).
 */
#ifndef SIGBUS
#  define SIGBUS SIGILL
#endif

/*
 * Does the compiler support the __attribute__(()) syntax?  It's no
 * big deal if we don't.
 *
 * Note that Solaris Studio supports attribute, but does not support the
 * attributes we use.
 *
 * H5_ATTR_CONST is redefined in tools/h5repack/dynlib_rpk.c to quiet
 * gcc warnings (it has to use the public API and can't include this
 * file). Be sure to update that file if the #ifdefs change here.
 */
#ifdef __cplusplus
#   define H5_ATTR_FORMAT(X,Y,Z)  /*void*/
#   define H5_ATTR_UNUSED       /*void*/
#   define H5_ATTR_NORETURN     /*void*/
#   define H5_ATTR_CONST        /*void*/
#   define H5_ATTR_PURE         /*void*/
#else /* __cplusplus */
#if defined(H5_HAVE_ATTRIBUTE) && !defined(__SUNPRO_C)
#   define H5_ATTR_FORMAT(X,Y,Z)  __attribute__((format(X, Y, Z)))
#   define H5_ATTR_UNUSED       __attribute__((unused))
#   define H5_ATTR_NORETURN     __attribute__((noreturn))
#   define H5_ATTR_CONST        __attribute__((const))
#   define H5_ATTR_PURE         __attribute__((pure))
#else
#   define H5_ATTR_FORMAT(X,Y,Z)  /*void*/
#   define H5_ATTR_UNUSED       /*void*/
#   define H5_ATTR_NORETURN     /*void*/
#   define H5_ATTR_CONST        /*void*/
#   define H5_ATTR_PURE         /*void*/
#endif
#endif /* __cplusplus */

/*
 * Status return values for the `herr_t' type.
 * Since some unix/c routines use 0 and -1 (or more precisely, non-negative
 * vs. negative) as their return code, and some assumption had been made in
 * the code about that, it is important to keep these constants the same
 * values.  When checking the success or failure of an integer-valued
 * function, remember to compare against zero and not one of these two
 * values.
 */
#define SUCCEED    0
#define FAIL    (-1)
#define UFAIL    (unsigned)(-1)

/* number of members in an array */
#ifndef NELMTS
#    define NELMTS(X)    (sizeof(X)/sizeof(X[0]))
#endif

/* minimum of two, three, or four values */
#undef MIN
#define MIN(a,b)    (((a)<(b)) ? (a) : (b))
#define MIN2(a,b)    MIN(a,b)
#define MIN3(a,b,c)    MIN(a,MIN(b,c))
#define MIN4(a,b,c,d)    MIN(MIN(a,b),MIN(c,d))

/* maximum of two, three, or four values */
#undef MAX
#define MAX(a,b)    (((a)>(b)) ? (a) : (b))
#define MAX2(a,b)    MAX(a,b)
#define MAX3(a,b,c)    MAX(a,MAX(b,c))
#define MAX4(a,b,c,d)    MAX(MAX(a,b),MAX(c,d))

/* limit the middle value to be within a range (inclusive) */
#define RANGE(LO,X,HI)    MAX(LO,MIN(X,HI))

/* absolute value */
#ifndef ABS
#   define ABS(a)    (((a)>=0) ? (a) : -(a))
#endif

/* sign of argument */
#ifndef SIGN
#   define SIGN(a)    ((a)>0 ? 1 : (a)<0 ? -1 : 0)
#endif

/* test for number that is a power of 2 */
/* (from: http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2) */
#  define POWER_OF_TWO(n)  (!(n & (n - 1)) && n)

/* Raise an integer to a power of 2 */
#  define H5_EXP2(n)    (1 << (n))

/*
 * HDF Boolean type.
 */
#ifndef FALSE
  #define FALSE false
#endif
#ifndef TRUE
  #define TRUE true
#endif

/*
 * Numeric data types.  Some of these might be defined in Posix.1g, otherwise
 * we define them with the closest available type which is at least as large
 * as the number of bits indicated in the type name.  The `int8' types *must*
 * be exactly one byte wide because we use it for pointer calculations to
 * void* memory.
 */
#if H5_SIZEOF_INT8_T==0
    typedef signed char int8_t;
#   undef H5_SIZEOF_INT8_T
#   define H5_SIZEOF_INT8_T H5_SIZEOF_CHAR
#elif H5_SIZEOF_INT8_T==1
#else
#   error "the int8_t type must be 1 byte wide"
#endif

#if H5_SIZEOF_UINT8_T==0
    typedef unsigned char uint8_t;
#   undef H5_SIZEOF_UINT8_T
#   define H5_SIZEOF_UINT8_T H5_SIZEOF_CHAR
#elif H5_SIZEOF_UINT8_T==1
#else
#   error "the uint8_t type must be 1 byte wide"
#endif

#if H5_SIZEOF_INT16_T>=2
#elif H5_SIZEOF_SHORT>=2
    typedef short int16_t;
#   undef H5_SIZEOF_INT16_T
#   define H5_SIZEOF_INT16_T H5_SIZEOF_SHORT
#elif H5_SIZEOF_INT>=2
    typedef int int16_t;
#   undef H5_SIZEOF_INT16_T
#   define H5_SIZEOF_INT16_T H5_SIZEOF_INT
#else
#   error "nothing appropriate for int16_t"
#endif

#if H5_SIZEOF_UINT16_T>=2
#elif H5_SIZEOF_SHORT>=2
    typedef unsigned short uint16_t;
#   undef H5_SIZEOF_UINT16_T
#   define H5_SIZEOF_UINT16_T H5_SIZEOF_SHORT
#elif H5_SIZEOF_INT>=2
    typedef unsigned uint16_t;
#   undef H5_SIZEOF_UINT16_T
#   define H5_SIZEOF_UINT16_T H5_SIZEOF_INT
#else
#   error "nothing appropriate for uint16_t"
#endif

#if H5_SIZEOF_INT32_T>=4
#elif H5_SIZEOF_SHORT>=4
    typedef short int32_t;
#   undef H5_SIZEOF_INT32_T
#   define H5_SIZEOF_INT32_T H5_SIZEOF_SHORT
#elif H5_SIZEOF_INT>=4
    typedef int int32_t;
#   undef H5_SIZEOF_INT32_T
#   define H5_SIZEOF_INT32_T H5_SIZEOF_INT
#elif H5_SIZEOF_LONG>=4
    typedef long int32_t;
#   undef H5_SIZEOF_INT32_T
#   define H5_SIZEOF_INT32_T H5_SIZEOF_LONG
#else
#   error "nothing appropriate for int32_t"
#endif

/*
 * Maximum and minimum values.  These should be defined in <limits.h> for the
 * most part.
 */
#ifndef LLONG_MAX
#   define LLONG_MAX  ((long long)(((unsigned long long)1          \
              <<(8*sizeof(long long)-1))-1))
#   define LLONG_MIN    ((long long)(-LLONG_MAX)-1)
#endif
#ifndef ULLONG_MAX
#   define ULLONG_MAX  ((unsigned long long)((long long)(-1)))
#endif
#ifndef SIZET_MAX
#   define SIZET_MAX  ((size_t)(ssize_t)(-1))
#   define SSIZET_MAX  ((ssize_t)(((size_t)1<<(8*sizeof(ssize_t)-1))-1))
#endif

/*
 * Maximum & minimum values for our typedefs.
 */
#define HSIZET_MAX   ((hsize_t)ULLONG_MAX)
#define HSSIZET_MAX  ((hssize_t)LLONG_MAX)
#define HSSIZET_MIN  (~(HSSIZET_MAX))

/*
 * Types and max sizes for POSIX I/O.
 * OS X (Darwin) is odd since the max I/O size does not match the types.
 */
#if defined(H5_HAVE_WIN32_API)
#   define h5_posix_io_t                unsigned int
#   define h5_posix_io_ret_t            int
#   define H5_POSIX_MAX_IO_BYTES        INT_MAX
#elif defined(H5_HAVE_DARWIN)
#   define h5_posix_io_t                size_t
#   define h5_posix_io_ret_t            ssize_t
#   define H5_POSIX_MAX_IO_BYTES        INT_MAX
#else
#   define h5_posix_io_t                size_t
#   define h5_posix_io_ret_t            ssize_t
#   define H5_POSIX_MAX_IO_BYTES        SSIZET_MAX
#endif

/* POSIX I/O mode used as the third parameter to open/_open
 * when creating a new file (O_CREAT is set).
 */
#if defined(H5_HAVE_WIN32_API)
#   define H5_POSIX_CREATE_MODE_RW      (_S_IREAD | _S_IWRITE)
#else
#   define H5_POSIX_CREATE_MODE_RW      0666
#endif

/*
 * A macro to portably increment enumerated types.
 */
#ifndef H5_INC_ENUM
#  define H5_INC_ENUM(TYPE,VAR) (VAR)=((TYPE)((VAR)+1))
#endif

/* Represents an empty asynchronous request handle.
 * Used in the VOL code.
 */
#define H5_REQUEST_NULL                 NULL

/*
 * A macro to portably decrement enumerated types.
 */
#ifndef H5_DEC_ENUM
#  define H5_DEC_ENUM(TYPE,VAR) (VAR)=((TYPE)((VAR)-1))
#endif

/* Double constant wrapper
 *
 * Quiets gcc warnings from -Wunsuffixed-float-constants.
 *
 * This is a really annoying warning since the standard specifies that
 * constants of type double do NOT get a suffix so there's no way
 * to specify a constant of type double. To quiet gcc, we specify floating
 * point constants as type long double and cast to double.
 *
 * Note that this macro only needs to be used where using a double
 * is important. For most code, suffixing constants with F will quiet the
 * compiler and not produce erroneous code.
 */
#define H5_DOUBLE(S) ((double) S ## L)

/*
 * Methods to compare the equality of floating-point values:
 *
 *    1. H5_XXX_ABS_EQUAL - check if the difference is smaller than the
 *       Epsilon value.  The Epsilon values, FLT_EPSILON, DBL_EPSILON,
 *       and LDBL_EPSILON, are defined by compiler in float.h.
 *
 *    2. H5_XXX_REL_EQUAL - check if the relative difference is smaller than a
 *       predefined value M.  See if two values are relatively equal.
 *       It's the developer's responsibility not to pass in the value 0, which
 *       may cause the equation to fail.
 */
#define H5_FLT_ABS_EQUAL(X,Y)       (HDfabsf((X)-(Y)) < FLT_EPSILON)
#define H5_DBL_ABS_EQUAL(X,Y)       (HDfabs ((X)-(Y)) < DBL_EPSILON)
#define H5_LDBL_ABS_EQUAL(X,Y)      (HDfabsl((X)-(Y)) < LDBL_EPSILON)

#define H5_FLT_REL_EQUAL(X,Y,M)     (HDfabsf(((Y)-(X)) / (X)) < (M))
#define H5_DBL_REL_EQUAL(X,Y,M)     (HDfabs (((Y)-(X)) / (X)) < (M))
#define H5_LDBL_REL_EQUAL(X,Y,M)    (HDfabsl(((Y)-(X)) / (X)) < (M))

/* KiB, MiB, GiB, TiB, PiB, EiB - Used in profiling and timing code */
#define H5_KB (1024.0F)
#define H5_MB (1024.0F * 1024.0F)
#define H5_GB (1024.0F * 1024.0F * 1024.0F)
#define H5_TB (1024.0F * 1024.0F * 1024.0F * 1024.0F)
#define H5_PB (1024.0F * 1024.0F * 1024.0F * 1024.0F * 1024.0F)
#define H5_EB (1024.0F * 1024.0F * 1024.0F * 1024.0F * 1024.0F * 1024.0F)

#ifndef H5_HAVE_FLOCK
/* flock() operations. Used in the source so we have to define them when
 * the call is not available (e.g.: Windows). These should NOT be used
 * with system-provided flock() calls since the values will come from the
 * header file.
 */
#define LOCK_SH     0x01
#define LOCK_EX     0x02
#define LOCK_NB     0x04
#define LOCK_UN     0x08
#endif /* H5_HAVE_FLOCK */

/*
 * Data types and functions for timing certain parts of the library.
 */
typedef struct {
    double  utime;    /*user time      */
    double  stime;    /*system time      */
    double  etime;    /*elapsed wall-clock time  */
} H5_timer_t;

H5_DLL void H5_timer_reset (H5_timer_t *timer);
H5_DLL void H5_timer_begin (H5_timer_t *timer);
H5_DLL void H5_timer_end (H5_timer_t *sum/*in,out*/,
         H5_timer_t *timer/*in,out*/);
H5_DLL void H5_bandwidth(char *buf/*out*/, double nbytes, double nseconds);
H5_DLL time_t H5_now(void);

/* Depth of object copy */
typedef enum {
    H5_COPY_SHALLOW,    /* Shallow copy from source to destination, just copy field pointers */
    H5_COPY_DEEP        /* Deep copy from source to destination, including duplicating fields pointed to */
} H5_copy_depth_t;

/* Common object copying udata (right now only used for groups and datasets) */
typedef struct H5O_copy_file_ud_common_t {
    struct H5O_pline_t *src_pline;      /* Copy of filter pipeline for object */
} H5O_copy_file_ud_common_t;

/* Unique object "position" */
typedef struct {
    unsigned long fileno;       /* The unique identifier for the file of the object */
    haddr_t addr;               /* The unique address of the object's header in that file */
} H5_obj_t;

/*
 * Redefine all the POSIX functions.  We should never see a POSIX
 * function (or any other non-HDF5 function) in the source!
 */

/* Put all platform-specific definitions in the following file */
/* so that the following definitions are platform free. */
#include "H5win32defs.h"  /* For Windows-specific definitions */

#ifndef HDabort
    #define HDabort()    abort()
#endif /* HDabort */
#ifndef HDabs
    #define HDabs(X)    abs(X)
#endif /* HDabs */
#ifndef HDaccess
    #define HDaccess(F,M)    access(F, M)
#endif /* HDaccess */
#ifndef HDacos
    #define HDacos(X)    acos(X)
#endif /* HDacos */
#ifndef HDvasprintf
#ifdef H5_HAVE_VASPRINTF
#define HDvasprintf(RET, FMT, A) vasprintf(RET, FMT, A)
#endif /* H5_HAVE_VASPRINTF */
#endif /* HDvasprintf */
#ifndef HDalarm
#ifdef H5_HAVE_ALARM
#define HDalarm(N)              alarm(N)
#else /* H5_HAVE_ALARM */
#define HDalarm(N)              (0)
#endif /* H5_HAVE_ALARM */
#endif /* HDalarm */
#ifndef HDasctime
    #define HDasctime(T)    asctime(T)
#endif /* HDasctime */
#ifndef HDasin
    #define HDasin(X)    asin(X)
#endif /* HDasin */
#ifndef HDasprintf
    #define HDasprintf    asprintf /*varargs*/
#endif /* HDasprintf */
#ifndef HDassert
    #define HDassert(X)    assert(X)
#endif /* HDassert */
#ifndef HDatan
    #define HDatan(X)    atan(X)
#endif /* HDatan */
#ifndef HDatan2
    #define HDatan2(X,Y)    atan2(X,Y)
#endif /* HDatan2 */
#ifndef HDatexit
    #define HDatexit(F)    atexit(F)
#endif /* HDatexit */
#ifndef HDatof
    #define HDatof(S)    atof(S)
#endif /* HDatof */
#ifndef HDatoi
    #define HDatoi(S)    atoi(S)
#endif /* HDatoi */
#ifndef HDatol
    #define HDatol(S)    atol(S)
#endif /* HDatol */
#ifndef HDatoll
    #define HDatoll(S)   atoll(S)
#endif /* HDatol */
#ifndef HDbsearch
    #define HDbsearch(K,B,N,Z,F)  bsearch(K,B,N,Z,F)
#endif /* HDbsearch */
#ifndef HDcalloc
    #define HDcalloc(N,Z)    calloc(N,Z)
#endif /* HDcalloc */
#ifndef HDceil
    #define HDceil(X)    ceil(X)
#endif /* HDceil */
#ifndef HDcfgetispeed
    #define HDcfgetispeed(T)  cfgetispeed(T)
#endif /* HDcfgetispeed */
#ifndef HDcfgetospeed
    #define HDcfgetospeed(T)  cfgetospeed(T)
#endif /* HDcfgetospeed */
#ifndef HDcfsetispeed
    #define HDcfsetispeed(T,S)  cfsetispeed(T,S)
#endif /* HDcfsetispeed */
#ifndef HDcfsetospeed
    #define HDcfsetospeed(T,S)  cfsetospeed(T,S)
#endif /* HDcfsetospeed */
#ifndef HDchdir
    #define HDchdir(S)    chdir(S)
#endif /* HDchdir */
#ifndef HDchmod
    #define HDchmod(S,M)    chmod(S,M)
#endif /* HDchmod */
#ifndef HDchown
    #define HDchown(S,O,G)    chown(S,O,G)
#endif /* HDchown */
#ifndef HDclearerr
    #define HDclearerr(F)    clearerr(F)
#endif /* HDclearerr */
#ifndef HDclock
    #define HDclock()    clock()
#endif /* HDclock */
#ifndef HDclose
    #define HDclose(F)    close(F)
#endif /* HDclose */
#ifndef HDclosedir
    #define HDclosedir(D)    closedir(D)
#endif /* HDclosedir */
#ifndef HDcos
    #define HDcos(X)    cos(X)
#endif /* HDcos */
#ifndef HDcosh
    #define HDcosh(X)    cosh(X)
#endif /* HDcosh */
#ifndef HDcreat
    #define HDcreat(S,M)    creat(S,M)
#endif /* HDcreat */
#ifndef HDctermid
    #define HDctermid(S)    ctermid(S)
#endif /* HDctermid */
#ifndef HDctime
    #define HDctime(T)    ctime(T)
#endif /* HDctime */
#ifndef HDcuserid
    #define HDcuserid(S)    cuserid(S)
#endif /* HDcuserid */
#ifndef HDdifftime
    #ifdef H5_HAVE_DIFFTIME
        #define HDdifftime(X,Y)    difftime(X,Y)
    #else /* H5_HAVE_DIFFTIME */
        #define HDdifftime(X,Y)    ((double)(X)-(double)(Y))
    #endif /* H5_HAVE_DIFFTIME */
#endif /* HDdifftime */
#ifndef HDdiv
    #define HDdiv(X,Y)    div(X,Y)
#endif /* HDdiv */
#ifndef HDdup
    #define HDdup(F)    dup(F)
#endif /* HDdup */
#ifndef HDdup2
    #define HDdup2(F,I)    dup2(F,I)
#endif /* HDdup2 */
/* execl() variable arguments */
/* execle() variable arguments */
/* execlp() variable arguments */
#ifndef HDexecv
    #define HDexecv(S,AV)    execv(S,AV)
#endif /* HDexecv */
#ifndef HDexecve
    #define HDexecve(S,AV,E)  execve(S,AV,E)
#endif /* HDexecve */
#ifndef HDexecvp
    #define HDexecvp(S,AV)    execvp(S,AV)
#endif /* HDexecvp */
#ifndef HDexit
    #define HDexit(N)    exit(N)
#endif /* HDexit */
#ifndef HD_exit
    #define HD_exit(N)    _exit(N)
#endif /* HD_exit */
#ifndef HDexp
    #define HDexp(X)    exp(X)
#endif /* HDexp */
#ifndef HDexp2
    #define HDexp2(X)    exp2(X)
#endif /* HDexp2 */
#ifndef HDfabs
    #define HDfabs(X)    fabs(X)
#endif /* HDfabs */
/* use ABS() because fabsf() fabsl() are not common yet. */
#ifndef HDfabsf
    #define HDfabsf(X)    ABS(X)
#endif /* HDfabsf */
#ifndef HDfabsl
    #define HDfabsl(X)    ABS(X)
#endif /* HDfabsl */
#ifndef HDfclose
    #define HDfclose(F)    fclose(F)
#endif /* HDfclose */
#ifdef H5_HAVE_FCNTL
    #ifndef HDfcntl
        #define HDfcntl(F,C,...)    fcntl(F,C,__VA_ARGS__)
    #endif /* HDfcntl */
#endif /* H5_HAVE_FCNTL */
#ifndef HDfdopen
    #define HDfdopen(N,S)    fdopen(N,S)
#endif /* HDfdopen */
#ifndef HDfeof
    #define HDfeof(F)    feof(F)
#endif /* HDfeof */
#ifndef HDferror
    #define HDferror(F)    ferror(F)
#endif /* HDferror */
#ifndef HDfflush
    #define HDfflush(F)    fflush(F)
#endif /* HDfflush */
#ifndef HDfgetc
    #define HDfgetc(F)    fgetc(F)
#endif /* HDfgetc */
#ifndef HDfgetpos
    #define HDfgetpos(F,P)    fgetpos(F,P)
#endif /* HDfgetpos */
#ifndef HDfgets
    #define HDfgets(S,N,F)    fgets(S,N,F)
#endif /* HDfgets */
#ifndef HDfileno
    #define HDfileno(F)    fileno(F)
#endif /* HDfileno */
/* Since flock is so prevalent, always build these functions
 * when possible to avoid them becoming dead code.
 */
#ifdef H5_HAVE_FCNTL
H5_DLL int Pflock(int fd, int operation);
#endif /* H5_HAVE_FCNTL */
H5_DLL H5_ATTR_CONST int Nflock(int fd, int operation);
#ifndef HDflock
    /* NOTE: flock(2) is not present on all POSIX systems.
     * If it is not present, we try a flock() equivalent based on
     * fcntl(2), then fall back to a function that always fails if
     * it is not present at all (Windows uses a separate Wflock()
     * function).
     */
    #if defined(H5_HAVE_FLOCK)
        #define HDflock(F,L)    flock(F,L)
    #elif defined(H5_HAVE_FCNTL)
        #define HDflock(F,L)    Pflock(F,L)
    #else
        #define HDflock(F,L)    Nflock(F,L)
    #endif /* H5_HAVE_FLOCK */
#endif /* HDflock */
#ifndef HDfloor
    #define HDfloor(X)    floor(X)
#endif /* HDfloor */
#ifndef HDfmod
    #define HDfmod(X,Y)    fmod(X,Y)
#endif /* HDfmod */
#ifndef HDfopen
    #define HDfopen(S,M)    fopen(S,M)
#endif /* HDfopen */
#ifndef HDfork
    #define HDfork()    fork()
#endif /* HDfork */
#ifndef HDfprintf
#define HDfprintf fprintf
#endif
#ifndef HDfpathconf
    #define HDfpathconf(F,N)  fpathconf(F,N)
#endif /* HDfpathconf */
#ifndef HDfputc
    #define HDfputc(C,F)    fputc(C,F)
#endif /* HDfputc */
#ifndef HDfputs
    #define HDfputs(S,F)    fputs(S,F)
#endif /* HDfputs */
#ifndef HDfread
    #define HDfread(M,Z,N,F)  fread(M,Z,N,F)
#endif /* HDfread */
#ifndef HDfree
    #define HDfree(M)    free(M)
#endif /* HDfree */
#ifndef HDfreopen
    #define HDfreopen(S,M,F)  freopen(S,M,F)
#endif /* HDfreopen */
#ifndef HDfrexp
    #define HDfrexp(X,N)    frexp(X,N)
#endif /* HDfrexp */
/* Check for Cray-specific 'frexpf()' and 'frexpl()' routines */
#ifndef HDfrexpf
    #ifdef H5_HAVE_FREXPF
        #define HDfrexpf(X,N)    frexpf(X,N)
    #else /* H5_HAVE_FREXPF */
        #define HDfrexpf(X,N)    frexp(X,N)
    #endif /* H5_HAVE_FREXPF */
#endif /* HDfrexpf */
#ifndef HDfrexpl
    #ifdef H5_HAVE_FREXPL
        #define HDfrexpl(X,N)    frexpl(X,N)
    #else /* H5_HAVE_FREXPL */
        #define HDfrexpl(X,N)    frexp(X,N)
    #endif /* H5_HAVE_FREXPL */
#endif /* HDfrexpl */
/* fscanf() variable arguments */
#ifndef HDfseek
    #define HDfseek(F,O,W)  fseeko(F,O,W)
#endif /* HDfseek */
#ifndef HDfsetpos
    #define HDfsetpos(F,P)    fsetpos(F,P)
#endif /* HDfsetpos */
#ifndef HDfstat
    #define HDfstat(F,B)        fstat(F,B)
#endif /* HDfstat */
#ifndef HDlstat
    #define HDlstat(S,B)    lstat(S,B)
#endif /* HDlstat */
#ifndef HDstat
    #define HDstat(S,B)    stat(S,B)
#endif /* HDstat */

#ifndef H5_HAVE_WIN32_API
/* These definitions differ in Windows and are defined in
 * H5win32defs for that platform.
 */
typedef struct stat         h5_stat_t;
typedef off_t               h5_stat_size_t;
#define HDoff_t             off_t
#endif /* H5_HAVE_WIN32_API */

#define H5_SIZEOF_H5_STAT_SIZE_T H5_SIZEOF_OFF_T

#ifndef HDftell
    #define HDftell(F)    ftello(F)
#endif /* HDftell */
#ifndef HDftruncate
    #define HDftruncate(F,L)        ftruncate(F,L)
#endif /* HDftruncate */
#ifndef HDfwrite
    #define HDfwrite(M,Z,N,F)  fwrite(M,Z,N,F)
#endif /* HDfwrite */
#ifndef HDgetc
    #define HDgetc(F)    getc(F)
#endif /* HDgetc */
#ifndef HDgetchar
    #define HDgetchar()    getchar()
#endif /* HDgetchar */
#ifndef HDgetcwd
    #define HDgetcwd(S,Z)    getcwd(S,Z)
#endif /* HDgetcwd */
#ifndef HDgetdcwd
    #define HDgetdcwd(D,S,Z)  getcwd(S,Z)
#endif /* HDgetdcwd */
#ifndef HDgetdrive
    #define HDgetdrive()    0
#endif /* HDgetdrive */
#ifndef HDgetegid
    #define HDgetegid()    getegid()
#endif /* HDgetegid() */
#ifndef HDgetenv
    #define HDgetenv(S)    getenv(S)
#endif /* HDgetenv */
#ifndef HDgeteuid
    #define HDgeteuid()    geteuid()
#endif /* HDgeteuid */
#ifndef HDgetgid
    #define HDgetgid()    getgid()
#endif /* HDgetgid */
#ifndef HDgetgrgid
    #define HDgetgrgid(G)    getgrgid(G)
#endif /* HDgetgrgid */
#ifndef HDgetgrnam
    #define HDgetgrnam(S)    getgrnam(S)
#endif /* HDgetgrnam */
#ifndef HDgetgroups
    #define HDgetgroups(Z,G)  getgroups(Z,G)
#endif /* HDgetgroups */
#ifndef HDgethostname
    #define HDgethostname(N,L)    gethostname(N,L)
#endif /* HDgetlogin */
#ifndef HDgetlogin
    #define HDgetlogin()    getlogin()
#endif /* HDgetlogin */
#ifndef HDgetpgrp
    #define HDgetpgrp()    getpgrp()
#endif /* HDgetpgrp */
#ifndef HDgetpid
    #define HDgetpid()    getpid()
#endif /* HDgetpid */
#ifndef HDgetppid
    #define HDgetppid()    getppid()
#endif /* HDgetppid */
#ifndef HDgetpwnam
    #define HDgetpwnam(S)    getpwnam(S)
#endif /* HDgetpwnam */
#ifndef HDgetpwuid
    #define HDgetpwuid(U)    getpwuid(U)
#endif /* HDgetpwuid */
#ifndef HDgetrusage
    #define HDgetrusage(X,S)  getrusage(X,S)
#endif /* HDgetrusage */
#ifndef HDgets
    #define HDgets(S)    gets(S)
#endif /* HDgets */
#ifndef HDgettimeofday
    #define HDgettimeofday(S,P)  gettimeofday(S,P)
#endif /* HDgettimeofday */
#ifndef HDgetuid
    #define HDgetuid()    getuid()
#endif /* HDgetuid */
#ifndef HDgmtime
    #define HDgmtime(T)    gmtime(T)
#endif /* HDgmtime */
#ifndef HDisalnum
    #define HDisalnum(C)    isalnum((int)(C)) /*cast for solaris warning*/
#endif /* HDisalnum */
#ifndef HDisalpha
    #define HDisalpha(C)    isalpha((int)(C)) /*cast for solaris warning*/
#endif /* HDisalpha */
#ifndef HDisatty
    #define HDisatty(F)    isatty(F)
#endif /* HDisatty */
#ifndef HDiscntrl
    #define HDiscntrl(C)    iscntrl((int)(C)) /*cast for solaris warning*/
#endif /* HDiscntrl */
#ifndef HDisdigit
    #define HDisdigit(C)    isdigit((int)(C)) /*cast for solaris warning*/
#endif /* HDisdigit */
#ifndef HDisgraph
    #define HDisgraph(C)    isgraph((int)(C)) /*cast for solaris warning*/
#endif /* HDisgraph */
#ifndef HDislower
    #define HDislower(C)    islower((int)(C)) /*cast for solaris warning*/
#endif /* HDislower */
#ifndef HDisprint
    #define HDisprint(C)    isprint((int)(C)) /*cast for solaris warning*/
#endif /* HDisprint */
#ifndef HDispunct
    #define HDispunct(C)    ispunct((int)(C)) /*cast for solaris warning*/
#endif /* HDispunct */
#ifndef HDisspace
    #define HDisspace(C)    isspace((int)(C)) /*cast for solaris warning*/
#endif /* HDisspace */
#ifndef HDisupper
    #define HDisupper(C)    isupper((int)(C)) /*cast for solaris warning*/
#endif /* HDisupper */
#ifndef HDisxdigit
    #define HDisxdigit(C)    isxdigit((int)(C)) /*cast for solaris warning*/
#endif /* HDisxdigit */
#ifndef HDkill
    #define HDkill(P,S)    kill(P,S)
#endif /* HDkill */
#ifndef HDlabs
    #define HDlabs(X)    labs(X)
#endif /* HDlabs */
#ifndef HDldexp
    #define HDldexp(X,N)    ldexp(X,N)
#endif /* HDldexp */
#ifndef HDldiv
    #define HDldiv(X,Y)    ldiv(X,Y)
#endif /* HDldiv */
#ifndef HDlink
    #define HDlink(OLD,NEW)    link(OLD,NEW)
#endif /* HDlink */
#ifndef HDllround
    #define HDllround(V)     llround(V)
#endif /* HDround */
#ifndef HDllroundf
    #define HDllroundf(V)    llroundf(V)
#endif /* HDllroundf */
#ifndef HDllroundl
    #define HDllroundl(V)    llroundl(V)
#endif /* HDllroundl */
#ifndef HDlocaleconv
    #define HDlocaleconv()    localeconv()
#endif /* HDlocaleconv */
#ifndef HDlocaltime
    #define HDlocaltime(T)    localtime(T)
#endif /* HDlocaltime */
#ifndef HDlog
    #define HDlog(X)    log(X)
#endif /* HDlog */
#ifndef HDlog10
    #define HDlog10(X)    log10(X)
#endif /* HDlog10 */
#ifndef HDlongjmp
    #define HDlongjmp(J,N)    longjmp(J,N)
#endif /* HDlongjmp */
#ifndef HDlround
    #define HDlround(V)     lround(V)
#endif /* HDround */
#ifndef HDlroundf
    #define HDlroundf(V)    lroundf(V)
#endif /* HDlroundf */
#ifndef HDlroundl
    #define HDlroundl(V)    lroundl(V)
#endif /* HDroundl */
#ifndef HDlseek
    #define HDlseek(F,O,W)  lseek(F,O,W)
#endif /* HDlseek */
#ifndef HDmalloc
    #define HDmalloc(Z)    malloc(Z)
#endif /* HDmalloc */
#ifndef HDposix_memalign
    #define HDposix_memalign(P,A,Z) posix_memalign(P,A,Z)
#endif /* HDposix_memalign */
#ifndef HDmblen
    #define HDmblen(S,N)    mblen(S,N)
#endif /* HDmblen */
#ifndef HDmbstowcs
    #define HDmbstowcs(P,S,Z)  mbstowcs(P,S,Z)
#endif /* HDmbstowcs */
#ifndef HDmbtowc
    #define HDmbtowc(P,S,Z)    mbtowc(P,S,Z)
#endif /* HDmbtowc */
#ifndef HDmemchr
    #define HDmemchr(S,C,Z)    memchr(S,C,Z)
#endif /* HDmemchr */
#ifndef HDmemcmp
    #define HDmemcmp(X,Y,Z)    memcmp(X,Y,Z)
#endif /* HDmemcmp */
/*
 * The (char*) casts are required for the DEC when optimizations are turned
 * on and the source and/or destination are not aligned.
 */
#ifndef HDmemcpy
    #define HDmemcpy(X,Y,Z)    memcpy((char*)(X),(const char*)(Y),Z)
#endif /* HDmemcpy */
#ifndef HDmemmove
    #define HDmemmove(X,Y,Z)  memmove((char*)(X),(const char*)(Y),Z)
#endif /* HDmemmove */
#ifndef HDmemset
    #define HDmemset(X,C,Z)    memset(X,C,Z)
#endif /* HDmemset */
#ifndef HDmkdir
    #define HDmkdir(S,M)    mkdir(S,M)
#endif /* HDmkdir */
#ifndef HDmkfifo
    #define HDmkfifo(S,M)    mkfifo(S,M)
#endif /* HDmkfifo */
#ifndef HDmktime
    #define HDmktime(T)    mktime(T)
#endif /* HDmktime */
#ifndef HDmodf
    #define HDmodf(X,Y)    modf(X,Y)
#endif /* HDmodf */
#ifndef HDnanosleep
    #define HDnanosleep(N, O)    nanosleep(N, O)
#endif /* HDnanosleep */
#ifndef HDopen
    #define HDopen(F,...)    open(F,__VA_ARGS__)
#endif /* HDopen */
#ifndef HDopendir
    #define HDopendir(S)    opendir(S)
#endif /* HDopendir */
#ifndef HDpathconf
    #define HDpathconf(S,N)    pathconf(S,N)
#endif /* HDpathconf */
#ifndef HDpause
    #define HDpause()    pause()
#endif /* HDpause */
#ifndef HDperror
    #define HDperror(S)    perror(S)
#endif /* HDperror */
#ifndef HDpipe
    #define HDpipe(F)    pipe(F)
#endif /* HDpipe */
#ifndef HDpow
    #define HDpow(X,Y)    pow(X,Y)
#endif /* HDpow */
#ifndef HDpowf
    #define HDpowf(X,Y)   powf(X,Y)
#endif /* HDpowf */
#ifndef HDpread
    #define HDpread(F,B,C,O)    pread(F,B,C,O)
#endif /* HDpread */
#ifndef HDprintf
#define HDprintf printf
#endif /* HDprintf */
#ifndef HDputc
    #define HDputc(C,F)    putc(C,F)
#endif /* HDputc*/
#ifndef HDputchar
    #define HDputchar(C)    putchar(C)
#endif /* HDputchar */
#ifndef HDputs
    #define HDputs(S)    puts(S)
#endif /* HDputs */
#ifndef HDpwrite
    #define HDpwrite(F,B,C,O)    pwrite(F,B,C,O)
#endif /* HDpwrite */
#ifndef HDqsort
    #define HDqsort(M,N,Z,F)  qsort(M,N,Z,F)
#endif /* HDqsort*/
#ifndef HDraise
    #define HDraise(N)    raise(N)
#endif /* HDraise */

#ifdef H5_HAVE_RAND_R
    #ifndef HDrandom
        #define HDrandom()    HDrand()
    #endif /* HDrandom */
    H5_DLL int HDrand(void);
#elif H5_HAVE_RANDOM
    #ifndef HDrand
        #define HDrand()    random()
    #endif /* HDrand */
    #ifndef HDrandom
        #define HDrandom()    random()
    #endif /* HDrandom */
#else /* H5_HAVE_RANDOM */
    #ifndef HDrand
        #define HDrand()    rand()
    #endif /* HDrand */
    #ifndef HDrandom
        #define HDrandom()    rand()
    #endif /* HDrandom */
#endif /* H5_HAVE_RANDOM */

#ifndef HDread
    #define HDread(F,M,Z)    read(F,M,Z)
#endif /* HDread */
#ifndef HDreaddir
    #define HDreaddir(D)    readdir(D)
#endif /* HDreaddir */
#ifndef HDrealloc
    #define HDrealloc(M,Z)    realloc(M,Z)
#endif /* HDrealloc */
#ifndef HDrealpath
    #define HDrealpath(F1,F2)    realpath(F1,F2)
#endif /* HDrealloc */
#ifndef HDremove
    #define HDremove(S)    remove(S)
#endif /* HDremove */
#ifndef HDrename
    #define HDrename(OLD,NEW)  rename(OLD,NEW)
#endif /* HDrename */
#ifndef HDrewind
    #define HDrewind(F)    rewind(F)
#endif /* HDrewind */
#ifndef HDrewinddir
    #define HDrewinddir(D)    rewinddir(D)
#endif /* HDrewinddir */
#ifndef HDround
    #define HDround(V)    round(V)
#endif /* HDround */
#ifndef HDroundf
    #define HDroundf(V)    roundf(V)
#endif /* HDroundf */
#ifndef HDroundl
    #define HDroundl(V)    roundl(V)
#endif /* HDroundl */
#ifndef HDrmdir
    #define HDrmdir(S)    rmdir(S)
#endif /* HDrmdir */
/* scanf() variable arguments */
#ifndef HDselect
    #define HDselect(N,RD,WR,ER,T)    select(N,RD,WR,ER,T)
#endif /* HDsetbuf */
#ifndef HDsetbuf
    #define HDsetbuf(F,S)    setbuf(F,S)
#endif /* HDsetbuf */
#ifndef HDsetenv
    #define HDsetenv(N,V,O)    setenv(N,V,O)
#endif /* HDsetenv */
#ifndef HDsetgid
    #define HDsetgid(G)    setgid(G)
#endif /* HDsetgid */
#ifndef HDsetjmp
    #define HDsetjmp(J)    setjmp(J)
#endif /* HDsetjmp */
#ifndef HDsetlocale
    #define HDsetlocale(N,S)  setlocale(N,S)
#endif /* HDsetlocale */
#ifndef HDsetpgid
    #define HDsetpgid(P,PG)    setpgid(P,PG)
#endif /* HDsetpgid */
#ifndef HDsetsid
    #define HDsetsid()    setsid()
#endif /* HDsetsid */
#ifndef HDsetuid
    #define HDsetuid(U)    setuid(U)
#endif /* HDsetuid */
#ifndef HDsetvbuf
    #define HDsetvbuf(F,S,M,Z)  setvbuf(F,S,M,Z)
#endif /* HDsetvbuf */
#ifndef HDsigaddset
    #define HDsigaddset(S,N)  sigaddset(S,N)
#endif /* HDsigaddset */
#ifndef HDsigdelset
    #define HDsigdelset(S,N)  sigdelset(S,N)
#endif /* HDsigdelset */
#ifndef HDsigemptyset
    #define HDsigemptyset(S)  sigemptyset(S)
#endif /* HDsigemptyset */
#ifndef HDsigfillset
    #define HDsigfillset(S)    sigfillset(S)
#endif /* HDsigfillset */
#ifndef HDsigismember
    #define HDsigismember(S,N)  sigismember(S,N)
#endif /* HDsigismember */
#ifndef HDsiglongjmp
    #define HDsiglongjmp(J,N)  siglongjmp(J,N)
#endif /* HDsiglongjmp */
#ifndef HDsignal
    #define HDsignal(N,F)    signal(N,F)
#endif /* HDsignal */
#ifndef HDsigpending
    #define HDsigpending(S)    sigpending(S)
#endif /* HDsigpending */
#ifndef HDsigprocmask
    #define HDsigprocmask(H,S,O)  sigprocmask(H,S,O)
#endif /* HDsigprocmask */
#ifndef HDsigsetjmp
    #define HDsigsetjmp(J,N)  sigsetjmp(J,N)
#endif /* HDsigsetjmp */
#ifndef HDsigsuspend
    #define HDsigsuspend(S)    sigsuspend(S)
#endif /* HDsigsuspend */
#ifndef HDsin
    #define HDsin(X)    sin(X)
#endif /* HDsin */
#ifndef HDsinh
    #define HDsinh(X)    sinh(X)
#endif /* HDsinh */
#ifndef HDsleep
    #define HDsleep(N)    sleep(N)
#endif /* HDsleep */
#ifndef HDsnprintf
    #define HDsnprintf    snprintf /*varargs*/
#endif /* HDsnprintf */
#ifndef HDsprintf
    #define HDsprintf    sprintf /*varargs*/
#endif /* HDsprintf */
#ifndef HDsqrt
    #define HDsqrt(X)    sqrt(X)
#endif /* HDsqrt */
#ifdef H5_HAVE_RAND_R
    H5_DLL void HDsrand(unsigned int seed);
    #ifndef HDsrandom
        #define HDsrandom(S)    HDsrand(S)
    #endif /* HDsrandom */
#elif H5_HAVE_RANDOM
    #ifndef HDsrand
        #define HDsrand(S)    srandom(S)
    #endif /* HDsrand */
    #ifndef HDsrandom
        #define HDsrandom(S)    srandom(S)
    #endif /* HDsrandom */
#else /* H5_HAVE_RAND_R */
    #ifndef HDsrand
        #define HDsrand(S)    srand(S)
    #endif /* HDsrand */
    #ifndef HDsrandom
        #define HDsrandom(S)    srand(S)
    #endif /* HDsrandom */
#endif /* H5_HAVE_RAND_R */
#ifndef HDsscanf
    #define HDsscanf(S,FMT,...)   sscanf(S,FMT,__VA_ARGS__)
#endif /* HDsscanf */
#ifndef HDstrcat
    #define HDstrcat(X,Y)    strcat(X,Y)
#endif /* HDstrcat */
#ifndef HDstrchr
    #define HDstrchr(S,C)    strchr(S,C)
#endif /* HDstrchr */
#ifndef HDstrcmp
    #define HDstrcmp(X,Y)       strcmp(X,Y)
#endif /* HDstrcmp */
#ifndef HDstrcasecmp
    #define HDstrcasecmp(X,Y)       strcasecmp(X,Y)
#endif /* HDstrcasecmp */
#ifndef HDstrcoll
    #define HDstrcoll(X,Y)    strcoll(X,Y)
#endif /* HDstrcoll */
#ifndef HDstrcpy
    #define HDstrcpy(X,Y)    strcpy(X,Y)
#endif /* HDstrcpy */
#ifndef HDstrcspn
    #define HDstrcspn(X,Y)    strcspn(X,Y)
#endif /* HDstrcspn */
#ifndef HDstrerror
    #define HDstrerror(N)    strerror(N)
#endif /* HDstrerror */
#ifndef HDstrftime
    #define HDstrftime(S,Z,F,T)  strftime(S,Z,F,T)
#endif /* HDstrftime */
#ifndef HDstrlen
    #define HDstrlen(S)    strlen(S)
#endif /* HDstrlen */
#ifndef HDstrncat
    #define HDstrncat(X,Y,Z)  strncat(X,Y,Z)
#endif /* HDstrncat */
#ifndef HDstrncmp
    #define HDstrncmp(X,Y,Z)  strncmp(X,Y,Z)
#endif /* HDstrncmp */
#ifndef HDstrncpy
    #define HDstrncpy(X,Y,Z)  strncpy(X,Y,Z)
#endif /* HDstrncpy */
#ifndef HDstrpbrk
    #define HDstrpbrk(X,Y)    strpbrk(X,Y)
#endif /* HDstrpbrk */
#ifndef HDstrrchr
    #define HDstrrchr(S,C)    strrchr(S,C)
#endif /* HDstrrchr */
#ifndef HDstrspn
    #define HDstrspn(X,Y)    strspn(X,Y)
#endif /* HDstrspn */
#ifndef HDstrstr
    #define HDstrstr(X,Y)    strstr(X,Y)
#endif /* HDstrstr */
#ifndef HDstrtod
    #define HDstrtod(S,R)    strtod(S,R)
#endif /* HDstrtod */
#ifndef HDstrtok
    #define HDstrtok(X,Y)    strtok(X,Y)
#endif /* HDstrtok */
#ifndef HDstrtok_r
    #define HDstrtok_r(X,Y,Z) strtok_r(X,Y,Z)
#endif /* HDstrtok */
#ifndef HDstrtol
    #define HDstrtol(S,R,N)    strtol(S,R,N)
#endif /* HDstrtol */
#ifndef HDstrtoll
    #ifdef H5_HAVE_STRTOLL
        #define HDstrtoll(S,R,N)  strtoll(S,R,N)
    #else
        H5_DLL int64_t HDstrtoll (const char *s, const char **rest, int base);
    #endif /* H5_HAVE_STRTOLL */
#endif /* HDstrtoll */
#ifndef HDstrtoul
    #define HDstrtoul(S,R,N)  strtoul(S,R,N)
#endif /* HDstrtoul */
#ifndef HDstrtoull
    #define HDstrtoull(S,R,N)  strtoull(S,R,N)
#endif /* HDstrtoul */
#ifndef HDstrxfrm
    #define HDstrxfrm(X,Y,Z)  strxfrm(X,Y,Z)
#endif /* HDstrxfrm */
#ifdef H5_HAVE_SYMLINK
    #ifndef HDsymlink
        #define HDsymlink(F1,F2)  symlink(F1,F2)
    #endif /* HDsymlink */
#endif /* H5_HAVE_SYMLINK */
#ifndef HDsysconf
    #define HDsysconf(N)    sysconf(N)
#endif /* HDsysconf */
#ifndef HDsystem
    #define HDsystem(S)    system(S)
#endif /* HDsystem */
#ifndef HDtan
    #define HDtan(X)    tan(X)
#endif /* HDtan */
#ifndef HDtanh
    #define HDtanh(X)    tanh(X)
#endif /* HDtanh */
#ifndef HDtcdrain
    #define HDtcdrain(F)    tcdrain(F)
#endif /* HDtcdrain */
#ifndef HDtcflow
    #define HDtcflow(F,A)    tcflow(F,A)
#endif /* HDtcflow */
#ifndef HDtcflush
    #define HDtcflush(F,N)    tcflush(F,N)
#endif /* HDtcflush */
#ifndef HDtcgetattr
    #define HDtcgetattr(F,T)  tcgetattr(F,T)
#endif /* HDtcgetattr */
#ifndef HDtcgetpgrp
    #define HDtcgetpgrp(F)    tcgetpgrp(F)
#endif /* HDtcgetpgrp */
#ifndef HDtcsendbreak
    #define HDtcsendbreak(F,N)  tcsendbreak(F,N)
#endif /* HDtcsendbreak */
#ifndef HDtcsetattr
    #define HDtcsetattr(F,O,T)  tcsetattr(F,O,T)
#endif /* HDtcsetattr */
#ifndef HDtcsetpgrp
    #define HDtcsetpgrp(F,N)  tcsetpgrp(F,N)
#endif /* HDtcsetpgrp */
#ifndef HDtime
    #define HDtime(T)    time(T)
#endif /* HDtime */
#ifndef HDtimes
    #define HDtimes(T)    times(T)
#endif /* HDtimes*/
#ifndef HDtmpfile
    #define HDtmpfile()    tmpfile()
#endif /* HDtmpfile */
#ifndef HDtmpnam
    #define HDtmpnam(S)    tmpnam(S)
#endif /* HDtmpnam */
#ifndef HDtolower
    #define HDtolower(C)    tolower(C)
#endif /* HDtolower */
#ifndef HDtoupper
    #define HDtoupper(C)    toupper(C)
#endif /* HDtoupper */
#ifndef HDttyname
    #define HDttyname(F)    ttyname(F)
#endif /* HDttyname */
#ifndef HDtzset
    #define HDtzset()    tzset()
#endif /* HDtzset */
#ifndef HDumask
    #define HDumask(N)    umask(N)
#endif /* HDumask */
#ifndef HDuname
    #define HDuname(S)    uname(S)
#endif /* HDuname */
#ifndef HDungetc
    #define HDungetc(C,F)    ungetc(C,F)
#endif /* HDungetc */
#ifndef HDunlink
    #define HDunlink(S)    unlink(S)
#endif /* HDunlink */
#ifndef HDutime
    #define HDutime(S,T)    utime(S,T)
#endif /* HDutime */
#ifndef HDva_arg
    #define HDva_arg(A,T)    va_arg(A,T)
#endif /* HDva_arg */
#ifndef HDva_copy
#define HDva_copy(D,S)    va_copy(D,S)
#endif /* HDva_copy */
#ifndef HDva_end
    #define HDva_end(A)    va_end(A)
#endif /* HDva_end */
#ifndef HDva_start
    #define HDva_start(A,P)    va_start(A,P)
#endif /* HDva_start */
#ifndef HDvfprintf
    #define HDvfprintf(F,FMT,A)  vfprintf(F,FMT,A)
#endif /* HDvfprintf */
#ifndef HDvprintf
    #define HDvprintf(FMT,A)  vprintf(FMT,A)
#endif /* HDvprintf */
#ifndef HDvsprintf
    #define HDvsprintf(S,FMT,A)  vsprintf(S,FMT,A)
#endif /* HDvsprintf */
#ifndef HDvsnprintf
    #define HDvsnprintf(S,N,FMT,A) vsnprintf(S,N,FMT,A)
#endif /* HDvsnprintf */
#ifndef HDwait
    #define HDwait(W)    wait(W)
#endif /* HDwait */
#ifndef HDwaitpid
    #define HDwaitpid(P,W,O)  waitpid(P,W,O)
#endif /* HDwaitpid */
#ifndef HDwcstombs
    #define HDwcstombs(S,P,Z)  wcstombs(S,P,Z)
#endif /* HDwcstombs */
#ifndef HDwctomb
    #define HDwctomb(S,C)    wctomb(S,C)
#endif /* HDwctomb */
#ifndef HDwrite
    #define HDwrite(F,M,Z)    write(F,M,Z)
#endif /* HDwrite */

/*
 * And now for a couple non-Posix functions...  Watch out for systems that
 * define these in terms of macros.
 */
#if !defined strdup && !defined H5_HAVE_STRDUP
extern char *strdup(const char *s);
#endif

#ifndef HDstrdup
    #define HDstrdup(S)     strdup(S)
#endif /* HDstrdup */

#ifndef HDpthread_self
    #define HDpthread_self()    pthread_self()
#endif /* HDpthread_self */

/* Use this version of pthread_self for printing the thread ID */
#ifndef HDpthread_self_ulong
    #define HDpthread_self_ulong()    ((unsigned long)pthread_self())
#endif /* HDpthread_self_ulong */

/* Include the generated overflow header file */
#include "H5overflow.h"

#if defined(H5_HAVE_WINDOW_PATH)

/* directory delimiter for Windows: slash and backslash are acceptable on Windows */
#define H5_DIR_SLASH_SEPC       '/'
#define H5_DIR_SEPC             '\\'
#define H5_DIR_SEPS             "\\"
#define H5_CHECK_DELIMITER(SS)     ((SS == H5_DIR_SEPC) || (SS == H5_DIR_SLASH_SEPC))
#define H5_CHECK_ABSOLUTE(NAME)    ((HDisalpha(NAME[0])) && (NAME[1] == ':') && (H5_CHECK_DELIMITER(NAME[2])))
#define H5_CHECK_ABS_DRIVE(NAME)   ((HDisalpha(NAME[0])) && (NAME[1] == ':'))
#define H5_CHECK_ABS_PATH(NAME)    (H5_CHECK_DELIMITER(NAME[0]))

#define H5_GET_LAST_DELIMITER(NAME, ptr) {                 \
    char        *slash, *backslash;                     \
                                                        \
    slash = HDstrrchr(NAME, H5_DIR_SLASH_SEPC);         \
    backslash = HDstrrchr(NAME, H5_DIR_SEPC);           \
    if(backslash > slash)                               \
        (ptr = backslash);                              \
    else                                                \
        (ptr = slash);                                  \
}

#else /* H5_HAVE_WINDOW_PATH */

#define H5_DIR_SEPC             '/'
#define H5_DIR_SEPS             "/"
#define H5_CHECK_DELIMITER(SS)     (SS == H5_DIR_SEPC)
#define H5_CHECK_ABSOLUTE(NAME)    (H5_CHECK_DELIMITER(*NAME))
#define H5_CHECK_ABS_DRIVE(NAME)   (0)
#define H5_CHECK_ABS_PATH(NAME)    (0)
#define H5_GET_LAST_DELIMITER(NAME, ptr)   ptr = HDstrrchr(NAME, H5_DIR_SEPC);

#endif /* H5_HAVE_WINDOW_PATH */

/* Forward declaration of H5CXpush() / H5CXpop() */
/* (Including H5CXprivate.h creates bad circular dependencies - QAK, 3/18/2018) */
H5_DLL herr_t H5CX_push(void);
H5_DLL herr_t H5CX_pop(void);

/* Compile-time "assert" macro */
#define HDcompile_assert(e)     ((void)sizeof(char[ !!(e) ? 1 : -1]))

/* Checksum functions */
H5_DLL uint32_t H5_checksum_fletcher32(const void *data, size_t len);
H5_DLL uint32_t H5_checksum_crc(const void *data, size_t len);
H5_DLL uint32_t H5_checksum_lookup3(const void *data, size_t len, uint32_t initval);

/* Time related routines */
H5_DLL double H5_get_time(void);

#endif /* _H5private_H */

