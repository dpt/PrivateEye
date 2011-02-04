/* --------------------------------------------------------------------------
 *    Name: array.h
 * Purpose: Array
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
