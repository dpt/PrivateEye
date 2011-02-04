
#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "parse.h"
#include "tokens.h"
#include "sym.h"

int lang_parse(const char *s)
{
  lex   *lex;
  parse *parser;

  lex = lex__create(s);
  if (lex == NULL)
    goto failure;

  /*t = lex__scan(lex);
  while (t != END)
  {
    if (t < 256)
      printf("<%c>", t);
    else
      printf("<%d>", t);
    t = lex__scan(lex);
  }
  printf("\n");

  lex__dump(lex);*/

  parser = parser__create(lex);
  if (parser == NULL)
    goto failure;

  parse__program(parser);

  // run

  parser__destroy(parser);

  lex__destroy(lex);

  return 0;


failure:

  return 1;
}
