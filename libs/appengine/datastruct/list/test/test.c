/* $Id: test.c,v 1.2 2008-08-08 22:41:05 dpt Exp $ */

#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/types.h"
#include "appengine/datastruct/list.h"

typedef struct
{
  list_t ll;
  char   string[32];
}
node;

static int printelement(list_t *e, void *arg)
{
  node *n;

  NOT_USED(arg);

  n = (node *) e;

  printf("%p: %s\n", (void *) e, n->string);

  return 0;
}

int list_test(void)
{
  node data[] =
  {
    { { NULL }, "deckard"   },
    { { NULL }, "batty"     },
    { { NULL }, "rachael"   },
    { { NULL }, "gaff"      },
    { { NULL }, "bryant"    },
    { { NULL }, "pris"      },
    { { NULL }, "sebastian" },
  };

  list_t anchor = { NULL };

  int i;

  printf("test: add to head\n");

  for (i = 0; i < NELEMS(data); i++)
  {
    printf("adding '%s'...\n", data[i].string);
    list__add_to_head(&anchor, &data[i].ll);
  }

  printf("test: iterate\n");

  list__walk(&anchor, printelement, NULL);

  return 0;
}
