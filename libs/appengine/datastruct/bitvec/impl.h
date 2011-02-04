/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_BITVEC_IMPL_H
#define APPENGINE_BITVEC_IMPL_H

struct bitvec_t
{
  unsigned int  length; /* length in words */
  unsigned int *vec;
};

error bitvec__ensure(bitvec_t *v, int need);

#endif /* APPENGINE_BITVEC_IMPL_H */
