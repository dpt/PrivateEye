/* $Id: tokens.h,v 1.2 2009-02-05 23:49:25 dpt Exp $ */

#ifndef TOKENS_H
#define TOKENS_H

typedef int Token;

enum
{
  IDENT  = 256,
  NUMBER,
  NONE,
  END,
};

#endif /* TOKENS_H */
