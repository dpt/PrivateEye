/* --------------------------------------------------------------------------
 *    Name: oserror.h
 * Purpose: Handling OS errors
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_OSERROR_H
#define APPENGINE_OSERROR_H

#include "oslib/os.h"

/* ----------------------------------------------------------------------- */

os_error *oserror_check(os_error   *e,
                         const char *file,
                         int         line);

#ifndef NDEBUG
#define EC(_e) oserror_check((os_error *) _e, __FILE__, __LINE__)
#else
#define EC(_e) ((os_error *) _e)
#endif

/* ----------------------------------------------------------------------- */

void oserror_report(int, const char *, ...);
void oserror_report_block(os_error *error);

/* ----------------------------------------------------------------------- */

/* Plots the error message using Wimp_TextOp */
void oserror_plot(os_error *e, int x, int y);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_OSERROR_H */
