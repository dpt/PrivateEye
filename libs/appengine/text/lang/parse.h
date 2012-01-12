
#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

#define T parse

typedef struct T T;

T *parser_create(lex *lx);
void parser_destroy(T *p);
void parse_program(T *p);

#undef T

#endif /* PARSE_H */

