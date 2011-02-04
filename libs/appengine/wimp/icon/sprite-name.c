
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "appengine/wimp/icon.h"

/* Nr_jpg;Sradiooff,radioon */
/* Nd_cleaning;R2 */
/* Np_background;Sgrightc,pgrightc;Pptr_picker,8,4;R5 */
/* Nw_scale;A0-9;Pptr_write,4,10 */

void icon_sprite_name(const char *validation, char *name)
{
  enum { Check, Scan, Found, Bingo } state;
  const char *p;
  const char *l;
  const char *r;

  *name = '\0';

  l = r = NULL; /* STFU */

  state = Check;

  for (p = validation; *p != '\0'; p++)
  {
    int c = *p;

    switch (state)
    {
    case Check:
      if (toupper(c) == 'S')
      {
        state = Found;
        l = p + 1;
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

    case Found:
      if (c == ';' || c == '\0')
      {
        state = Bingo;
        r = p; /* exclusive right hand bound */
      }
      break;

    case Bingo:
      if (r - l > 0)
      {
        memcpy(name, l, r - l);
        name[r - l] = '\0';
        return;
      }
      else
      {
        return; /* none */
      }
    }
  }
}
