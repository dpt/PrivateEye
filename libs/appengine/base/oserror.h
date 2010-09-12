/* --------------------------------------------------------------------------
 *    Name: oserror.h
 * Purpose: Handling OS errors
 *  Author: David Thomas
 * Version: $Id: oserror.h,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_OSERROR_H
#define APPENGINE_OSERROR_H

#include "oslib/os.h"

/* ----------------------------------------------------------------------- */

os_error *oserror__check(os_error   *e,
                         const char *file,
                         int         line);

#ifndef NDEBUG
#define EC(_e) oserror__check((os_error *) _e, __FILE__, __LINE__)
#else
#define EC(_e) ((os_error *) _e)
#endif

/* ----------------------------------------------------------------------- */

void oserror__report(int, const char *, ...);
void oserror__report_block(os_error *error);

/* ----------------------------------------------------------------------- */

/* Plots the error message using Wimp_TextOp */
void oserror__plot(os_error *e, int x, int y);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_OSERROR_H */
