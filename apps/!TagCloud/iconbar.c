/* --------------------------------------------------------------------------
 *    Name: iconbar.c
 * Purpose: Icon bar icon
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/app/choices.h"
#include "appengine/gadgets/iconbar.h"
#include "appengine/wimp/menu.h"

#include "globals.h"
#include "iconbar.h"
#include "makecloud.h"
#include "menunames.h"

static void pointerx(const wimp_pointer *pointer, void *opaque)
{
  NOT_USED(opaque);

  if (pointer->buttons & wimp_CLICK_SELECT)
  {
    make_cloud();
  }
}

static void selectionx(const wimp_selection *selection, void *opaque)
{
  NOT_USED(opaque);

  switch (selection->items[0])
  {
  case ICONBAR_QUIT:
    GLOBALS.flags |= Flag_Quit;
    break;
  }
}

error tag_icon_bar_init(void)
{
  error err;

  err = icon_bar_init();
  if (err)
    return err;

  icon_bar_set_handlers(pointerx, selectionx, NULL, NULL);

  return error_OK;
}

void tag_icon_bar_fin(void)
{
  icon_bar_fin();
}
