/* --------------------------------------------------------------------------
 *    Name: Strings.h
 * Purpose: Declarations for the Strings library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STRINGS_H
#define APPENGINE_STRINGS_H

#include <stddef.h>

/*
 * Returns a pointer to whatever precedes the last ':' or '.' in the input.
 */
const char *str_branch(const char *string);

/*
 * Copies a control-terminated string, terminating new string with NUL.
 */
void str_cpy(char *to, const char *from);

/*
 * Duplicates a control-terminated string.
 */
char *str_dup(const char *string);

/*
 * Get a pointer to whatever follows the last ':' or '.' in the input.
 */
const char *str_leaf(const char *string);

/*
 * Get length of control-terminated string, excluding terminator.
 */
int str_len(const char *string);

/*
 * Returns a pointer to the given int as a string.
 */
const char *str_num(int number);

/*
 * Copies up to <size> characters of a control-terminated string, terminating
 * the new string with NUL.
 */
void str_n_cpy(char *to, const char *from, int size);

/*
 * Duplicates up to <size> characters of a control-terminated string.
 */
char *str_n_dup(const char *string, int size);

/*
 * Make the given control-terminated string NUL terminated.
 */
void str_term(char *string);

/* Case insensitive strcmp. */
int strcasecmp(const char *s1, const char *s2);

/* Case insensitive strncmp. */
int strncasecmp(const char *s1, const char *s2, size_t n);

/* Case insensitive string compare for two unterminated strings. */
int strnncasecmp(const char *s1, size_t n1, const char *s2, size_t n2);

#endif /* APPENGINE_STRINGS_H */
