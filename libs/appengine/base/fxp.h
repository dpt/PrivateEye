/* --------------------------------------------------------------------------
 *    Name: fxp.h
 * Purpose: Fixed point
 * Version: $Id: fxp.h,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_FXP_H
#define APPENGINE_FXP_H

/* Signed multiply of two 16.16 fixed-point numbers. */
int smull_fxp16(int x, int y);

/* Unsigned multiply of two 16.16 fixed-point numbers. */
unsigned int umull_fxp16(unsigned int x, unsigned int y);

#endif /* APPENGINE_FXP_H */
