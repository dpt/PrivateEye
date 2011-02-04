
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "lex.h"
#include "tokens.h"
#include "emit.h"

#include "parse.h"

/*
 * program    = {assignment} .
 * assignment = [ident "=" expression] ";" .
 * expression = ["+"|"-"] term {("+"|"-") term} .
 * term       = factor {("*"|"/"|"%") factor} .
 * factor     = ident | number | "(" expression ")" .
 */

struct parse
{
  lex   *lex;
  Token  lookahead;
};


static void expr(parse *p);


static int match(parse *p, Token t)
{
  if (p->lookahead == t)
  {
    p->lookahead = lex__scan(p->lex);
    return 1;
  }
  else
  {
    lerror("match: unexpected token %d, expecting %d", p->lookahead, t);
    return 0;
  }
}

static void factor(parse *p)
{
  switch (p->lookahead)
  {
  case IDENT:
    emit(lex__get_sym(p->lex), p->lookahead, lex__get_val(p->lex));
    match(p, IDENT);
    break;

  case NUMBER:
    emit(lex__get_sym(p->lex), p->lookahead, lex__get_val(p->lex));
    match(p, NUMBER);
    break;

  case '(':
    match(p, '(');
    expr(p);
    match(p, ')');
    break;

  default:
    lerror("factor: syntax error");
    break;
  }
}

static void term(parse *p)
{
  int t;

  factor(p);

  for (;;)
  {
    switch (p->lookahead)
    {
    case '*':
    case '/':
    case '%':
      t = p->lookahead;
      match(p, t);
      factor(p);
      emit(lex__get_sym(p->lex), t, NONE);
      continue;

    default:
      return;
    }
  }
}

static void expr(parse *p)
{
  int t;

  term(p);

  for (;;)
  {
    switch (p->lookahead)
    {
    case '+':
    case '-':
      t = p->lookahead;
      match(p, t);
      term(p);
      emit(lex__get_sym(p->lex), t, NONE);
      continue;

    default:
      return;
    }
  }
}

static void stmt(parse *p)
{
  int t;
  int v;
  int t2;

  switch (p->lookahead)
  {
  case IDENT:
    t = p->lookahead;
    v = lex__get_val(p->lex);
    match(p, IDENT);

    t2 = p->lookahead;
    match(p, '=');

    expr(p);

    emit(lex__get_sym(p->lex), t, v);
    emit(lex__get_sym(p->lex), t2, NONE);

    match(p, ';');
    break;

  default:
    lerror("stmt: expected identifier");
    break;
  }
}

/* ----------------------------------------------------------------------- */

parse *parser__create(lex *lex)
{
  parse *p;

  p = malloc(sizeof(*p));
  if (p == NULL)
    return NULL;

  p->lex = lex;

  return p;
}

void parser__destroy(parse *doomed)
{
  if (doomed == NULL)
    return;

  free(doomed);
}

void parse__program(parse *parse)
{
  parse->lookahead = lex__scan(parse->lex);
  while (parse->lookahead != END)
  {
    stmt(parse);
    printf("eol\n");
  }
}
