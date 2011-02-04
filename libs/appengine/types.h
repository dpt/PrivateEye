/* --------------------------------------------------------------------------
 *    Name: types.h
 * Purpose: Common types and macros
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TYPES_H
#define APPENGINE_TYPES_H

#define NELEMS(ARRAY) ((int)(sizeof(ARRAY) / sizeof(ARRAY[0])))

#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#define MAX(A,B) (((A) > (B)) ? (A) : (B))

#define CLAMP(VAL,MIN,MAX) ((VAL < MIN) ? MIN : (VAL > MAX) ? MAX : VAL)

#endif /* APPENGINE_TYPES_H */
