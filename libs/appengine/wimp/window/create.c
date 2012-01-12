
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"

#include "appengine/wimp/window.h"

wimp_w window_create(const char *template_name)
{
  char        *indirected_block;
  wimp_window *window_block;
  int          window_size;
  int          indirected_size;
  wimp_w       handle;

  indirected_block = NULL;
  window_block     = NULL;

  if (window_find_template_size(template_name,
                               &window_size,
                               &indirected_size) < 0)
    goto notfound;

  /* claiming indirected_block then window_block allows more free space to
   * be reclaimed than claiming the other way around */

  if (indirected_size)
  {
    indirected_block = malloc(indirected_size);
    if (indirected_block == NULL)
      goto oom;
  }
  else
  {
    indirected_block = NULL;
  }

  window_block = malloc(window_size);
  if (window_block == NULL)
    goto oom;

  if (window_load_template(template_name,
                           window_block,
                           indirected_block,
                           indirected_size) < 0)
    goto notfound;

  handle = window_create_from_memory(window_block);
  if (handle == NULL)
    goto oom;

  free(window_block);

  return handle; /* no error */


notfound:

  free(window_block);
  free(indirected_block);

  error_fatal1("NoAny", template_name); /* fatal error: <name> not found */

  return 0;


oom:

  free(window_block);
  free(indirected_block);

  error_fatal_oom(); /* fatal error */

  return 0;
}
