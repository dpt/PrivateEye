
#include <stdio.h>

#include "appengine/types.h"
#include "appengine/vdu/font.h"

int font_test(void); /* suppress no declaration in header warning */

int font_test(void)
{
  static const struct
  {
    const char  *name;
    font_attrs  attrs;
  }
  data[] =
  {
    { "Homerton",               font_WEIGHT_NORMAL  |
                                font_VARIANT_NORMAL |
                                font_STYLE_NORMAL   |
                                font_STRETCH_NORMAL },
    { "Goudy",                  font_WEIGHT_BOLD    |
                                font_VARIANT_NORMAL |
                                font_STYLE_NORMAL   |
                                font_STRETCH_NORMAL },
    { "eric",                   font_WEIGHT_BOLD    |
                                font_VARIANT_NORMAL |
                                font_STYLE_NORMAL   |
                                font_STRETCH_COND   },
    { "eric.cond.bold",         font_WEIGHT_LIGHT   |
                                font_VARIANT_NORMAL |
                                font_STYLE_NORMAL   |
                                font_STRETCH_NORMAL },
    { "goudy.oldstyle.italic",  font_WEIGHT_NORMAL  |
                                font_VARIANT_NORMAL |
                                font_STYLE_NORMAL   |
                                font_STRETCH_NORMAL },
  };

  error err;
  char  buf[128];
  int   i;

  printf("test: select\n");

  for (i = 0; i < NELEMS(data); i++)
  {
    printf("Matching for '%s' with attrs %x\n", data[i].name, data[i].attrs);

    err = font_select(data[i].name, data[i].attrs, buf, sizeof(buf));
    if (!err)
      printf("font_select says: Use '%s'.\n", buf);
    else if (err == error_FONT_NO_MATCH)
      printf("font_select says: No good match.\n");
    else
      printf("error: %lx\n", err);
  }

  return 0;
}
