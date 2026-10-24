/* --------------------------------------------------------------------------
 *    Name: types.h
 * Purpose: Common types and macros
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TYPES_H
#define APPENGINE_TYPES_H

/* DPTLib's base/utils.h may already define these. */

#ifndef NELEMS
#define NELEMS(ARRAY) ((int)(sizeof(ARRAY) / sizeof(ARRAY[0])))
#endif

#ifndef MIN
#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#endif
#ifndef MAX
#define MAX(A,B) (((A) > (B)) ? (A) : (B))
#endif

#ifndef CLAMP
#define CLAMP(VAL,MIN,MAX) ((VAL < MIN) ? MIN : (VAL > MAX) ? MAX : VAL)
#endif

#endif /* APPENGINE_TYPES_H */
