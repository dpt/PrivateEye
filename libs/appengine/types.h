/* --------------------------------------------------------------------------
 *    Name: types.h
 * Purpose: Common types and macros
 *  Author: David Thomas
 * Version: $Id: types.h,v 1.2 2009-02-05 23:49:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TYPES_H
#define APPENGINE_TYPES_H

#define NELEMS(ARRAY) ((int)(sizeof(ARRAY) / sizeof(ARRAY[0])))

#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#define MAX(A,B) (((A) > (B)) ? (A) : (B))

#define CLAMP(VAL,MIN,MAX) ((VAL < MIN) ? MIN : (VAL > MAX) ? MAX : VAL)

#endif /* APPENGINE_TYPES_H */
