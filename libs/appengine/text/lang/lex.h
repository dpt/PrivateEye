
#ifndef LEX_H
#define LEX_H

#include "sym.h"
#include "tokens.h"

#define T lex

typedef struct T T;

T *lex_create(const char *input);
void lex_destroy(T *lx);
Token lex_scan(T *lx);
int lex_get_val(T *lx);
sym *lex_get_sym(T *lx);
void lex_dump(T *lx);

#undef T

#endif /* LEX_H */
