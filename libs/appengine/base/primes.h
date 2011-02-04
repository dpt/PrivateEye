/* --------------------------------------------------------------------------
 *    Name: primes.h
 * Purpose: Prime numbers
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_PRIMES_H
#define APPENGINE_PRIMES_H

/* Returns the nearest prime number to 'x' from a greatly reduced range.
 * Intended as a cache of values for use when sizing data structures. */
int prime_nearest(int x);

#endif /* APPENGINE_PRIMES_H */

