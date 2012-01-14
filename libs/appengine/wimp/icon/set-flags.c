
#include "kernel.h"
#include "swis.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"
#include "appengine/wimp/icon.h"

void icon_set_flags(wimp_w          w,
                    wimp_i          i,
                    wimp_icon_flags eor,
                    wimp_icon_flags clear)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  if (((state.icon.flags & ~clear) ^ eor) != state.icon.flags)
    wimp_set_icon_state(w, i, eor, clear);
}
