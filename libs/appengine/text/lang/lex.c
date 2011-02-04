
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "sym.h"
#include "tokens.h"

#include "lex.h"

struct lex
{
  int         peek;
  int         lineno;

  sym        *sym;

  char       *lexbuf;
  int         lexbufsz;

  const char *input;

  int         val;
};

lex *lex__create(const char *input)
{
  lex *c;

  c = calloc(1, sizeof(*c));
  if (c == NULL)
    return NULL;

  c->peek = ' ';

  c->sym = sym__create();
  if (c->sym == NULL)
    goto oom;

  c->lexbuf = malloc(256);
  if (c->lexbuf == NULL)
    goto oom;

  c->lexbufsz = 256;

  c->input = input;

  return c;

oom:

  sym__destroy(c->sym);
  free(c->lexbuf);
  free(c);

  return NULL;
}

void lex__destroy(lex *doomed)
{
  if (doomed == NULL)
    return;

  sym__destroy(doomed->sym);
  free(doomed->lexbuf);
  free(doomed);
}

static int lex__getc(lex *lex)
{
  int c;

  c = *lex->input++;
  if (c == '\0')
    return EOF;

  return c;
}

// ungetc, but only if we're backing up rather than trying to insert an
// arbitrary char into the stream
static void lex__ungetc(lex *lex, int c)
{
  lex->input--;

  assert(*lex->input == c);
}

Token lex__scan(lex *l)
{
  int t;

  for (;;)
  {
    t = lex__getc(l);

    if (t == ' ' || t == '\t')
    {
      ; /* strip whitespace */
    }
    else if (t == '\n')
    {
      l->lineno++;
    }
    else if (isdigit(t))
    {
      int v;

      v = 0;
      do
      {
        v = 10 * v + (t - '0');
        t = lex__getc(l);
      }
      while (isdigit(t));

      if (t != EOF)
        lex__ungetc(l, t);

      l->val = v;

      return NUMBER;
    }
    else if (isalpha(t))
    {
      char *end;
      char *p;
      int   sym;

      end = l->lexbuf + l->lexbufsz;
      p   = l->lexbuf;
      do
      {
        *p++ = t;
        t = lex__getc(l);
      }
      while (p < end && isalnum(t));
      *p++ = '\0';

      if (t != EOF)
        lex__ungetc(l, t);

      sym = sym__lookup(l->sym, l->lexbuf);
      if (!sym)
        sym = sym__insert(l->sym, l->lexbuf, IDENT);

      l->val = sym;

      return IDENT;
    }
    else if (t == EOF)
    {
      return END;
    }
    else
    {
      l->val = NONE;
      return t;
    }
  }
}

int lex__get_val(lex *lex)
{
  return lex->val;
}

sym *lex__get_sym(lex *lex)
{
  return lex->sym;
}

void lex__dump(lex *lex)
{
  sym__dump(lex->sym);
}
