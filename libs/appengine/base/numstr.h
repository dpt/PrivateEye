/* --------------------------------------------------------------------------
 *    Name: numstr.h
 * Purpose: Number strings
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_NUMSTR_H
#define APPENGINE_NUMSTR_H

/* Creates a comma-separated version of an integer */
void comma_number(int num, char *buf, int bufsz);

/* Creates a comma-separated version of a double. */
void comma_double(double num, char *buf, int bufsz);

#endif /* APPENGINE_NUMSTR_H */
