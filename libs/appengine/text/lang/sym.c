
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "error.h"
#include "tokens.h"

#include "sym.h"

#define STRMAX 999
#define SYMMAX 99

typedef struct sym_ent
{
  char *lexptr;
  int   token;
}
sym_ent;

struct sym
{
  char           lexemes[STRMAX];
  int            lastchar;
  struct sym_ent symtable[SYMMAX];
  int            lastentry;
};

sym *sym__create(void)
{
  sym *s;

  s = malloc(sizeof(*s));
  if (s == NULL)
    return NULL;

  s->lastchar  = -1;
  s->lastentry =  0;

  return s;
}

void sym__destroy(sym *doomed)
{
  if (doomed == NULL)
    return;

  free(doomed);
}

int sym__lookup(sym *sy, const char *s)
{
  int i;

  for (i = sy->lastentry; i > 0; i--)
    if (strcmp(sy->symtable[i].lexptr, s) == 0)
      return i;

  return 0;
}

int sym__insert(sym *sy, const char *s, Token tok)
{
  int len;

  len = strlen(s);
  if (sy->lastentry + 1 >= SYMMAX)
    lerror("sym_insert: symbol table full");
  if (sy->lastchar + len + 1 >= STRMAX)
    lerror("sym_insert: lexemes array full");
  sy->lastentry++;
  sy->symtable[sy->lastentry].token  = tok;
  sy->symtable[sy->lastentry].lexptr = sy->lexemes + sy->lastchar + 1;
  sy->lastchar += len + 1;

  memcpy(sy->symtable[sy->lastentry].lexptr, s, len + 1);

  return sy->lastentry;
}

const char *sym__get(sym *sy, int index)
{
  assert(index >= 0);
  assert(index <= sy->lastentry);

  return sy->symtable[index].lexptr;
}

void sym__dump(sym *sy)
{
  int i;

  for (i = sy->lastentry; i > 0; i--)
    printf("%d: %s -> %d\n", i, sy->symtable[i].lexptr,
                                sy->symtable[i].token);
}
