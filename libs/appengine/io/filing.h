/* --------------------------------------------------------------------------
 *    Name: Filing.h
 * Purpose: Declarations for the Filing library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_FILING_H
#define APPENGINE_FILING_H

#include <stddef.h> /* for size_t */

#include "oslib/types.h"
#include "oslib/osgbpb.h"

#include "appengine/base/errors.h"

/* buffer.c -------------------------------------------------------------- */

extern int buffer_start(const char *, size_t);
extern void buffer_stop(void);
extern int buffer_getbyte(void);
extern int buffer_getblock(char *, size_t);

/* scan.c ---------------------------------------------------------------- */

typedef unsigned int dirscan_flags;
enum
{
  dirscan_RECURSE     = 1, /* recurse into directories */
  dirscan_FILES       = 2, /* return files */
  dirscan_DIRECTORIES = 4, /* return directories */
};

typedef result_t (dirscan_callback)(const char          *obj_name,
                                    osgbpb_info_stamped *info,
                                    void                *context);

result_t dirscan(const char       *dir_name,
                 dirscan_callback *callback,
                 dirscan_flags     flags,
                 void             *context);

/* filetype.c ------------------------------------------------------------ */

void file_type_to_name(bits file_type, char name[9]);

#endif /* APPENGINE_FILING_H */
