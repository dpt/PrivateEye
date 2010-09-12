/* $Id: heap.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include "kernel.h"
#include "swis.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

/* #include "MemCheck:MemCheck.h" */

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"
#include "appengine/base/heap.h"


/* Note: must use the 'real' malloc and free, can't use the appengine
 *       equivalents/veneers here because *they* use *this*. */


/* A heap control block */
typedef struct _heapcb
{
  struct _heapcb *next;
  char           *base;
  unsigned int    area;
}
heapcb;


static heapcb *first_cb = NULL;


static heapcb *getcb(char *base)
{
  heapcb *cb;

  if (base == NULL)
    return NULL;

  for (cb = first_cb; cb != NULL; cb = cb->next)
    if (cb->base == base)
      break;

  return cb;
}


char *heap_create(const char *description, size_t size_limit)
{
  heapcb *cb;
  int     page_size;

  /* Get a new heapcb and link it into the start of the chain */
  cb = malloc(sizeof(heapcb));
  if (cb == NULL)
    return NULL; /* failed - unable to allocate space for control block */

  cb->next = first_cb;
  first_cb = cb;

  /* Create a dynamic area at least one page in size and set it up as a heap
   */
  _swi(OS_ReadMemMapInfo, _OUT(0), &page_size);

  if (EC(_swix(OS_DynamicArea, _INR(0,8)|_OUT(1)|_OUT(3), 0, -1, page_size,
               -1, 0x80, size_limit, NULL, -1, description, &cb->area,
               &cb->base)))
  {
    free(cb);
    return NULL;
  }

  /* page_size must be enough to hold the default heap structure */

  _swi(OS_Heap, _INR(0,1)|_IN(3), 0, cb->base, page_size);

  return cb->base;
}


void heap_delete(char *base)
{
  heapcb *cb;
  heapcb *thiscb;

  cb = getcb(base);
  if (cb == NULL)
    return; /* unknown heap */

  /* Delete the dynamic area */
  EC(_swix(OS_DynamicArea, _INR(0,1), 1, cb->area));

  /* remove its heapcb from the chain */
  if (cb == first_cb)
  {
    /* start of chain */
    first_cb = first_cb->next;
  }
  else
  {
    /* elsewhere in chain */
    for (thiscb = first_cb; thiscb->next != cb; thiscb = thiscb->next)
      ;
    thiscb->next = cb->next;
  }

  free(cb);
}


char *heap_claim(char *base, size_t required_size)
{
  _kernel_oserror *e;
  int              largest;
  unsigned int     actual_change;
  void            *block;
  heapcb          *cb;

  if (required_size == 0)
    return NULL;

  cb = getcb(base);
  if (cb == NULL)
    return NULL; /* unknown heap */

  /* Repeatedly attempt to claim a block of the required size */
  while ((e = _swix(OS_Heap, _INR(0,1)|_IN(3)|_OUT(2), 2, base,
                    required_size, &block)) != NULL)
  {
    int errnum;

    /* MemCheck_RegisterMiscBlock(e, sizeof(e->errnum)); */
    errnum = e->errnum;
    /* MemCheck_UnRegisterMiscBlock(e); */
    if (errnum != 0x184 /* Heap Full */)
    {
#ifndef NDEBUG
      EC(e);
#endif
      return NULL; /* unknown error */
    }

    /* Read largest free space */
    _swi(OS_Heap, _INR(0,1)|_OUT(2), 1, base, &largest);

    /* Increase the dynamic area by at least the difference between what we
     * want and what we have spare */
    if (EC(_swix(OS_ChangeDynamicArea, _INR(0,1)|_OUT(1), cb->area,
                 required_size - largest, &actual_change)) != NULL)
      return NULL; /* (most likely) out of memory */

    /* Resize the heap itself */
    _swi(OS_Heap, _INR(0,1)|_IN(3), 5, base, actual_change);
  }

  /* MemCheck_RegisterMiscBlock(block, required_size); */

  return block;
}


void heap_release(char *base, char *block)
{
  _kernel_oserror *e;
  heapcb          *cb;
  int              errnum;
  int              change;
  unsigned int     actual_change;

  cb = getcb(base);
  if (cb == NULL)
    return; /* unknown heap */

  /* MemCheck_UnRegisterMiscBlock(block); */

  /* Release the block */
  _swi(OS_Heap, _INR(0,2), 3, base, block);

  /* Current SWI methods don't let us easily get at the returned R3, so we
   * use our own assembler veneer */
  e = xosheap_resize_r3(base, INT_MIN, &change);
  /* MemCheck_RegisterMiscBlock(e, sizeof(e->errnum)); */
  errnum = e->errnum;
  /* MemCheck_UnRegisterMiscBlock(e); */
  if (errnum != 0x187 /* Can't shrink heap any further */)
    _swi(OS_GenerateError, _IN(0), e);

  /* Shrink the dynamic area (note this needs to occur before the shrink of
   * the heap itself, since we need the actual size change) */
  _swi(OS_ChangeDynamicArea, _INR(0,1)|_OUT(1), cb->area, -change,
       &actual_change);

  /* Shrink the heap by the actual size change */
  _swi(OS_Heap, _INR(0,1)|_IN(3), 5, base, change - actual_change);
}
