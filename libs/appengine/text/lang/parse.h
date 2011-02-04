
#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

#define T parse

typedef struct T T;

T *parser__create(lex *lx);
void parser__destroy(T *p);
void parse__program(T *p);

#undef T

#endif /* PARSE_H */

