/*
  ImageMagick Application Programming Interface declarations.
*/
#ifndef _MAGICK_H
#define _MAGICK_H

#define __EXTENSIONS__  1
#define _POSIX_C_SOURCE  199506L
#define _XOPEN_SOURCE  500
#define _XOPEN_SOURCE_EXTENDED  1

/*
  System include declarations.
*/
#if defined(__cplusplus) || defined(c_plusplus)
# include <cstdio>
# include <cstdlib>
# include <cstdarg>
# include <cstring>
#else
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>
#endif

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
#define fseek  fseeko
#define ftell  ftello
#endif

#if defined(_VISUALC_)
# include <direct.h>
#else
# include <unistd.h>
#endif

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <locale.h>
#include <errno.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#if !defined(vms) && !defined(macintosh)
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
#if !defined(WIN32)
# include <sys/times.h>
#if defined(HAVE_MMAP)
# include <sys/mman.h>
#endif
# include "magick/api.h"
#else
# include "api.h"
#endif
#else
# include <types.h>
# include <stat.h>
# include <time.h>
#if defined(macintosh)
# include <SIOUX.h>
# include <console.h>
# include <unix.h>
#endif
# include "api.h"
#endif

/*
  ImageMagick API headers
*/
#if defined(macintosh)
#define HasJPEG
#define HasLZW
#define HasPNG
#define HasTIFF
#define HasTTF
#define HasZLIB
#endif

#if defined(VMS)
#define HasJPEG
#define HasLZW
#define HasPNG
#define HasTIFF
#define HasTTF
#define HasX11
#define HasZLIB
#endif

#if defined(WIN32)
#define HasJBIG
#define HasJPEG
#define HasLZW
#define HasPNG
#define HasTIFF
#define HasTTF
#define HasX11
#define HasZLIB
#define HasBZLIB
#define HasHDF
#define HAVE_MMAP
#endif

#undef index

#endif
