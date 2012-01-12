
#include <stdio.h>

#include "error.h"
#include "sym.h"
#include "tokens.h"

#include "emit.h"

// will build the tree
void emit(sym *sy, Token t, int val)
{
  printf("emit: ");

  switch (t)
  {
  case IDENT:
    printf("ident: %s\n", sym_get(sy, val));
    break;

  case NUMBER:
    printf("number: %d\n", val);
    break;

  default:
    printf("op: %c\n", t);
    break;
  }
}
