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

static void clicked(const wimp_pointer *pointer, void *opaque)
{
  NOT_USED(opaque);

  if (pointer->buttons & wimp_CLICK_SELECT)
  {
    result_report(make_cloud());
  }
}

static void selected(const wimp_selection *selection, void *opaque)
{
  NOT_USED(opaque);

  switch (selection->items[0])
  {
  case ICONBAR_QUIT:
    GLOBALS.flags |= Flag_Quit;
    break;
  }
}

result_t tag_icon_bar_init(void)
{
  result_t err;

  err = icon_bar_init();
  if (err)
    return err;

  icon_bar_set_handlers(clicked, selected, NULL, NULL);

  return result_OK;
}

void tag_icon_bar_fin(void)
{
  icon_bar_fin();
}
