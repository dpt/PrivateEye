/* $Id: delelem.c,v 1.2 2008-08-05 21:36:33 dpt Exp $ */

#include <stdlib.h>
#include <string.h>

#include "appengine/datastruct/array.h"

void array__delete_element(void  *array,
                           size_t elemsize,
                           int    nelems,
                           int    doomed)
{
  /* alternative:
     array__delete_elements(array, elemsize, nelems, doomed, doomed); */

  size_t n;
  char  *to;
  char  *from;

  n = nelems - (doomed + 1);
  if (n == 0)
    return;

  to   = (char *) array + elemsize * doomed;
  from = (char *) array + elemsize * (doomed + 1);

  memmove(to, from, n * elemsize);
}
