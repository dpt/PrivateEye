/* --------------------------------------------------------------------------
 *    Name: Strings.h
 * Purpose: Declarations for the Strings library
 *  Author: David Thomas
 * Version: $Id: strings.h,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STRINGS_H
#define APPENGINE_STRINGS_H

/*
 * Returns a pointer to whatever precedes the last ':' or '.' in the input.
 */
extern const char *str_branch(const char *string);

/*
 * Copies a control-terminated string, terminating new string with NUL.
 */
extern void str_cpy(char *to, const char *from);

/*
 * Duplicates a control-terminated string.
 */
extern char *str_dup(const char *string);

/*
 * Get a pointer to whatever follows the last ':' or '.' in the input.
 */
extern const char *str_leaf(const char *string);

/*
 * Get length of control-terminated string, excluding terminator.
 */
extern int str_len(const char *string);

/*
 * Returns a pointer to the given int as a string.
 */
extern const char *str_num(int number);

/*
 * Copies up to <size> characters of a control-terminated string, terminating
 * the new string with NUL.
 */
extern void str_n_cpy(char *to, const char *from, int size);

/*
 * Duplicates up to <size> characters of a control-terminated string.
 */
extern char *str_n_dup(const char *string, int size);

/*
 * Make the given control-terminated string NUL terminated.
 */
extern void str_term(char *string);

/* Case insensitive strcmp. */
int strcasecmp(const char *s1, const char *s2);

#endif /* APPENGINE_STRINGS_H */
