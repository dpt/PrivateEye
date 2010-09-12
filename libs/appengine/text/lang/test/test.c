/* $Id: test.c,v 1.2 2009-02-05 23:49:25 dpt Exp $ */

#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/types.h"

extern int lang_parse(const char *s);

int lang_test(void)
{
  static const char *exps[] =
  {
    "frob42 = (4*x * 2) / (9-y);",
    " a=b+c; ",

    "Q=P+R*S;",
    "Q=P+(R*S);",

    "Q=(P+R)*S;",
    "Q=((P+R)*S);",

    "Q=P*R/S%T;"
  };

  int i;

  for (i = 0; i < NELEMS(exps); i++)
  {
    printf("test: %d\n", i);
    lang_parse(exps[i]);
  }

  return 0;
}
