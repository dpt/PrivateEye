/* --------------------------------------------------------------------------
 *    Name: numstr.h
 * Purpose: Number strings
 * Version: $Id: numstr.h,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_NUMSTR_H
#define APPENGINE_NUMSTR_H

/* Creates a comma-separated version of an integer */
void comma_number(int num, char *buf, int bufsz);

/* Creates a comma-separated version of a double. */
void comma_double(double num, char *buf, int bufsz);

#endif /* APPENGINE_NUMSTR_H */
