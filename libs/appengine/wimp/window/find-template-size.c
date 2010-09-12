/* $Id: find-template-size.c,v 1.3 2009-05-18 22:07:52 dpt Exp $ */

#include <stddef.h>
#include <string.h>

#include "oslib/wimp.h"

#include "appengine/base/oserror.h"

#include "appengine/wimp/window.h"

int window_find_template_size(const char *template_name,
                              int        *window_size,
                              int        *indirected_size)
{
  char name[13];
  int  context;

  /* the name is updated on exit, so the name must first be copied into a
   * buffer */

  strcpy(name, template_name);

  EC(xwimp_load_template(wimp_GET_SIZE,
                         NULL,
                         NULL,
                         NULL,
                         name,
                         0, /* context */
                         window_size,
                         indirected_size,
                        &context));

  return (context == 0) ? -1 : 0; /* not found : found */
}
