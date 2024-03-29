/* --------------------------------------------------------------------------
 *    Name: iconbar.c
 * Purpose: Icon bar icon
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/app/choices.h"
#include "appengine/gadgets/iconbar.h"
#include "appengine/graphics/imagecache.h"
#include "appengine/wimp/menu.h"

#include "canvas.h"
#include "choicesdat.h"
#include "globals.h"
#include "iconbar.h"
#include "iconnames.h"          /* generated */
#include "menunames.h"          /* not generated */
#include "quit.h"
#include "tags-search.h"
#include "thumbview.h"
#include "viewer.h"

static void selected(const wimp_selection *selection, void *opaque)
{
  NOT_USED(opaque);

  switch (selection->items[0])
  {
  case ICONBAR_HELP:
    xwimp_start_task("X Filer_Run " APPNAME "Res:Help", NULL);
    break;

#ifdef EYE_CANVAS
  case ICONBAR_NEW:
    switch (selection->items[1])
    {
      case NEW_CANVAS:
        {
          result_t  err;
          canvas_t *canvas;

          err = canvas_create(&canvas);
          if (err)
            return;

          canvas_open(canvas);
        }
        break;
    }
    break;
#endif

  /* Close all */
  case ICONBAR_CLOSE:
    switch (selection->items[1])
    {
      case CLOSE_VIEWERS:
        if (can_quit())
          viewer_close_all();
        break;

#ifdef EYE_THUMBVIEW
      case CLOSE_THUMBVIEWS:
        thumbview_close_all();
        break;
#endif

#ifdef EYE_CANVAS
      case CLOSE_CANVASES:
        canvas_close_all();
        break;
#endif
    }
    break;

  case ICONBAR_EMPTYCACHE:
    imagecache_empty(GLOBALS.cache);
    break;

#ifdef EYE_TAGS
  case ICONBAR_SEARCHTAGS:
    tags_search_open();
    break;
#endif

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
  // FIXME: Perhaps, could use a structure like this:
  //
  // static const struct
  // {
  //   int index;  // menu entry
  //   int (*valid)(void); // function returns zero if entry should be shaded
  // }
  // shade[] =
  // {
  //   { ICONBAR_CLOSE, viewer_get_count },
  //   { ICONBAR_EMPTYCACHE, imagecache_get_count }
  // };

  NOT_USED(opaque);

  /* Shade the "Close all" entry if there are no images open */
  menu_set_icon_flags(menu->entries[ICONBAR_CLOSE].sub_menu,
                      CLOSE_VIEWERS,
                     (viewer_get_count() > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);

#ifdef EYE_THUMBVIEW
  menu_set_icon_flags(menu->entries[ICONBAR_CLOSE].sub_menu,
                      CLOSE_THUMBVIEWS,
                     (thumbview_get_count() > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);
#endif

#ifdef EYE_CANVAS
  menu_set_icon_flags(menu->entries[ICONBAR_CLOSE].sub_menu,
                      CLOSE_CANVASES,
                     (canvas_get_count() > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);
#endif

  /* Shade the "Empty cache" entry if the cache is already empty */
  menu_set_icon_flags(menu,
                      ICONBAR_EMPTYCACHE,
                     (imagecache_get_count(GLOBALS.cache) > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);
}

result_t eye_icon_bar_init(void)
{
  result_t err;

  err = icon_bar_init();
  if (err)
    return err;

  icon_bar_set_handlers(NULL, selected, update, NULL);

  return result_OK;
}

void eye_icon_bar_fin(void)
{
  icon_bar_fin();
}
