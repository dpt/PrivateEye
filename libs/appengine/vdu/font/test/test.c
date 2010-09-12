/* $Id: test.c,v 1.1 2009-05-21 22:27:21 dpt Exp $ */

#include <stdio.h>

#include "appengine/types.h"
#include "appengine/vdu/font.h"

int font_test(void); /* suppress no declaration in header warning */

int font_test(void)
{
  static const struct
  {
    const char  *name;
    font__attrs  attrs;
  }
  data[] =
  {
    { "Homerton",               font__WEIGHT_NORMAL  |
                                font__VARIANT_NORMAL |
                                font__STYLE_NORMAL   |
                                font__STRETCH_NORMAL },
    { "Goudy",                  font__WEIGHT_BOLD    |
                                font__VARIANT_NORMAL |
                                font__STYLE_NORMAL   |
                                font__STRETCH_NORMAL },
    { "eric",                   font__WEIGHT_BOLD    |
                                font__VARIANT_NORMAL |
                                font__STYLE_NORMAL   |
                                font__STRETCH_COND   },
    { "eric.cond.bold",         font__WEIGHT_LIGHT   |
                                font__VARIANT_NORMAL |
                                font__STYLE_NORMAL   |
                                font__STRETCH_NORMAL },
    { "goudy.oldstyle.italic",  font__WEIGHT_NORMAL  |
                                font__VARIANT_NORMAL |
                                font__STYLE_NORMAL   |
                                font__STRETCH_NORMAL },
  };

  error err;
  char  buf[128];
  int   i;

  printf("test: select\n");

  for (i = 0; i < NELEMS(data); i++)
  {
    printf("Matching for '%s' with attrs %x\n", data[i].name, data[i].attrs);

    err = font__select(data[i].name, data[i].attrs, buf, sizeof(buf));
    if (!err)
      printf("font__select says: Use '%s'.\n", buf);
    else if (err == error_FONT_NO_MATCH)
      printf("font__select says: No good match.\n");
    else
      printf("error: %lx\n", err);
  }

  return 0;
}
