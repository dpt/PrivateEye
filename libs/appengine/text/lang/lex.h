/* $Id: lex.h,v 1.2 2009-02-05 23:49:24 dpt Exp $ */

#ifndef LEX_H
#define LEX_H

#include "sym.h"
#include "tokens.h"

#define T lex

typedef struct T T;

T *lex__create(const char *input);
void lex__destroy(T *lx);
Token lex__scan(T *lx);
int lex__get_val(T *lx);
sym *lex__get_sym(T *lx);
void lex__dump(T *lx);

#undef T

#endif /* LEX_H */
