
#ifndef SYM_H
#define SYM_H

#include "tokens.h"

#define T sym

typedef struct T T;

T *sym_create(void);
void sym_destroy(T *sy);
int sym_lookup(T *sy, const char *s);
int sym_insert(T *sy, const char *s, Token tok);
const char *sym_get(sym *sy, int index);
void sym_dump(sym *sy);

#undef T

#endif /* SYM_H */
