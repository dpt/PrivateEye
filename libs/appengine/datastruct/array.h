/* --------------------------------------------------------------------------
 *    Name: array.h
 * Purpose: Array
 * Version: $Id: array.h,v 1.2 2008-08-05 21:36:33 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_ARRAY_H
#define APPENGINE_ARRAY_H

#include <stdlib.h>

void array__delete_element(void  *array,
                           size_t elemsize,
                           int    nelems,
                           int    doomed);

/* last_doomed is inclusive. */
void array__delete_elements(void  *array,
                            size_t elemsize,
                            int    nelems,
                            int    first_doomed,
                            int    last_doomed);

#endif /* APPENGINE_ARRAY_H */
