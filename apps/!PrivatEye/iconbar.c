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

#include "choicesdat.h"
#include "globals.h"
#include "iconbar.h"
#include "iconnames.h"          /* generated */
#include "imagecache.h"
#include "menunames.h"          /* not generated */
#include "quit.h"
#include "tags-search.h"
#include "viewer.h"

static void selectionx(const wimp_selection *selection, void *opaque)
{
  NOT_USED(opaque);

  switch (selection->items[0])
  {
  case ICONBAR_CLOSE:
    /* Close all */
    if (can_quit())
      viewer_close_all();
    break;

  case ICONBAR_EMPTYCACHE:
    imagecache_empty();
    break;

  case ICONBAR_SEARCHTAGS:
#ifdef EYE_TAGS
    tags_search__open();
#endif
    break;

  case ICONBAR_CHOICES:
    /* Choices... */
    choices_open(&privateeye_choices);
    break;

  case ICONBAR_QUIT:
    /* Quit */
    if (can_quit())
      GLOBALS.flags |= Flag_Quit;
    break;
  }
}

static void update(wimp_menu *menu, void *opaque)
{
  NOT_USED(opaque);

  /* Shade the "Close all" entry if there are no images open */
  menu_set_icon_flags(menu,
                      ICONBAR_CLOSE,
                     (viewer_get_count() > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);

  /* Shade the "Empty cache" entry if the cache is already empty */
  menu_set_icon_flags(menu,
                      ICONBAR_EMPTYCACHE,
                     (imagecache_get_count() > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);
}

error eye_icon_bar_init(void)
{
  error err;

  err = icon_bar__init();
  if (err)
    return err;

  icon_bar__set_handlers(NULL, selectionx, update, NULL);

  return error_OK;
}

void eye_icon_bar_fin(void)
{
  icon_bar__fin();
}
