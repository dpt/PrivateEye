/* $Id: eval.c,v 1.2 2009-02-05 23:49:24 dpt Exp $ */

#include <assert.h>

#include "tree.h"

#include "eval.h"

static int pushop(Tree *t)
{
  return t->u.value;
}

static int pushsymop(Tree *t)
{
  return t->u.symbol->value;
}

static int addop(Tree *t)
{
  return eval(t->left) + eval(t->right);
}

static int subop(Tree *t)
{
  return eval(t->left) - eval(t->right);
}

static int mulop(Tree *t)
{
  return eval(t->left) * eval(t->right);
}

static int divop(Tree *t)
{
  int left, right;

  left  = eval(t->left);
  right = eval(t->right);
  if (right == 0)
    return 0;

  return left / right;
}

static int minop(Tree *t)
{
  int left, right;

  left  = eval(t->left);
  right = eval(t->right);
  return left < right ? left : right;
}

static int maxop(Tree *t)
{
  int left, right;

  left  = eval(t->left);
  right = eval(t->right);
  return left > right ? left : right;
}

static int assignop(Tree *t)
{
  t->left->u.symbol->value = eval(t->right);
  return t->left->u.symbol->value;
}

typedef int (*opfn)(Tree *);

static const opfn optab[] =
{
  pushop,    /* OP_NUMBER */
  pushsymop, /* OP_VARIABLE */
  addop,     /* OP_ADD */
  subop,     /* OP_SUBTRACT */
  mulop,     /* OP_MULTIPLY */
  divop,     /* OP_DIVIDE */
  minop,     /* OP_MIN */
  maxop,     /* OP_MAX */
  assignop,  /* OP_ASSIGN */
};

int eval(Tree *t)
{
  assert(t->op < OP__LIMIT);

  return (*optab[t->op])(t);
}
