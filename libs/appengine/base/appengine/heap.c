
/* #include "MemCheck:MemCheck.h" */

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"

ae_heap *ae_heap_create_dynamic(const char *name)
{
  ae_heap *heap;

  if (EC(xappengine_heap_create_dynamic(name, &heap)) != NULL)
    return NULL;

  return heap;
}

void ae_heap_delete(ae_heap *heap)
{
  EC(xappengine_heap_delete(heap));
}

ae_anchor ae_heap_claim(ae_heap *heap, size_t size)
{
  ae_anchor anchor;

  if (EC(xappengine_heap_claim(heap, size, &anchor)) != NULL)
    return NULL;

  if (anchor != NULL)
  {
    /* MemCheck_RegisterMiscBlock(anchor, sizeof(anchor)); */
    /* MemCheck_RegisterFlexBlock(anchor, size); */
  }

  return anchor;
}

void ae_heap_release(ae_heap *heap, ae_anchor *anchor)
{
  /* MemCheck_UnRegisterFlexBlock(*anchor); */
  /* MemCheck_UnRegisterMiscBlock(*anchor); */

  EC(xappengine_heap_release(heap, anchor));
}

int ae_heap_resize(ae_heap *heap, ae_anchor anchor, size_t change)
{
  int success;

  if (EC(xappengine_heap_resize(heap, anchor, change, &success)) != NULL)
    return 0 /* failure */;

  /* if (success == 1)
    MemCheck_ResizeFlexBlock(anchor, ae_heap_size(heap, anchor)); */

  return (success == -1) ? 1 /* success */ : 0 /* failure */;
}

size_t ae_heap_size(ae_heap *heap, ae_anchor anchor)
{
  unsigned int *sl;
  size_t s;
  heap = heap; /* unused */

  sl = (unsigned int *) anchor + 1;

  /* MemCheck_RegisterMiscBlock(sl, sizeof(unsigned int)); */

  /* Careful Now: This assumes the heap layout. */
  s = *sl;

  /* MemCheck_UnRegisterMiscBlock(sl); */

  return s;
}
