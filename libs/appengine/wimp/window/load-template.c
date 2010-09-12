/* $Id: load-template.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include <string.h>

#include "oslib/types.h"
#include "oslib/font.h"
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

static byte *template_font_array = NULL;

void window_use_fonts(byte *array)
{
  template_font_array = array;

  /* Clear the font reference count array */
  memset(template_font_array, 0, 256);
}

void window_lose_fonts(void)
{
  int i;

  /* For each font counter, call Font_LoseFont for it 'count' times */
  for (i = 0; i < 256; i++)
    while (template_font_array[i]--)
      font_lose_font(i);
}

void window_set_font_array(byte *array)
{
  template_font_array = array;
}

byte *window_get_font_array(void)
{
  return template_font_array;
}

int window_load_template(const char  *template_name,
                         wimp_window *window,
                         char        *data,
                         int          indirected_size)
{
  os_error *err;
  char      name[13];

  strcpy(name, template_name);

  err = xwimp_load_template(window,
                            data,
                            data + indirected_size,
                           (template_font_array == NULL) ? (byte *) -1 : template_font_array,
                            name,
                            0,
                            NULL,
                            NULL,
                            NULL);

  /* 'name' is now updated */

  window->sprite_area = window_get_sprite_area();

  return (err == NULL) ? 0 : -1;
}
