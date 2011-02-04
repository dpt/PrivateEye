
#ifndef SYM_H
#define SYM_H

#include "tokens.h"

#define T sym

typedef struct T T;

T *sym__create(void);
void sym__destroy(T *sy);
int sym__lookup(T *sy, const char *s);
int sym__insert(T *sy, const char *s, Token tok);
const char *sym__get(sym *sy, int index);
void sym__dump(sym *sy);

#undef T

#endif /* SYM_H */
