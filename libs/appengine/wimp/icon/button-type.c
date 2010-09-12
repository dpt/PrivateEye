/* $Id: button-type.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include <ctype.h>
#include <stdlib.h>

#include "appengine/wimp/icon.h"

/* Nr_jpg;Sradiooff,radioon */
/* Nd_cleaning;R2 */
/* Np_background;Sgrightc,pgrightc;Pptr_picker,8,4;R5 */
/* Nw_scale;A0-9;Pptr_write,4,10 */

int icon_button_type(const char *validation)
{
  enum { Check, Scan, Number, Bingo } state;
  const char *p;
  const char *nump;
  const char *numq;

  nump = numq = NULL; /* STFU */

  state = Check;

  for (p = validation; *p != '\0'; p++)
  {
    int c = *p;

    switch (state)
    {
    case Check:
      if (toupper(c) == 'R')
      {
        state = Number;
        nump = p + 1;
      }
      else
      {
        state = Scan;
      }
      break;

    case Scan: /* Scan forward for a semicolon */
      if (c == ';')
        state = Check;
      break;

    case Number:
      if (!isdigit(c))
      {
        state = Bingo;
        numq = p; /* exclusive right hand bound */
      }
      break;

    case Bingo:
      if (numq - nump > 0)
        return atoi(nump);
      else
        return -1; /* none */
    }
  }

  return -1; /* none */
}
