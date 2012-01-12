
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

  lex = lex_create(s);
  if (lex == NULL)
    goto failure;

  /*t = lex_scan(lex);
  while (t != END)
  {
    if (t < 256)
      printf("<%c>", t);
    else
      printf("<%d>", t);
    t = lex_scan(lex);
  }
  printf("\n");

  lex_dump(lex);*/

  parser = parser_create(lex);
  if (parser == NULL)
    goto failure;

  parse_program(parser);

  // run

  parser_destroy(parser);

  lex_destroy(lex);

  return 0;


failure:

  return 1;
}
