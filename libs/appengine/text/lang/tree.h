
#ifndef TREE_H
#define TREE_H

typedef enum Op
{
  OP_NUMBER,
  OP_VARIABLE,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_MIN,
  OP_MAX,
  OP_ASSIGN,
  OP__LIMIT
}
Op;

typedef struct Symbol Symbol;

struct Symbol
{
  int       value;
  // char     *name; not used atm
};

typedef struct Tree Tree;

struct Tree
{
  Op        op;
  Tree     *left;
  Tree     *right;

  union
  {
    int     value;
    Symbol *symbol;
  }
  u;
};

#endif /* TREE_H */
