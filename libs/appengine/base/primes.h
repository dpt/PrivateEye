/* --------------------------------------------------------------------------
 *    Name: primes.h
 * Purpose: Prime numbers
 * Version: $Id: primes.h,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_PRIMES_H
#define APPENGINE_PRIMES_H

/* Returns the nearest prime number to 'x' from a greatly reduced range.
 * Intended as a cache of values for use when sizing data structures. */
int prime_nearest(int x);

#endif /* APPENGINE_PRIMES_H */

