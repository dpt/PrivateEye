/* --------------------------------------------------------------------------
 *    Name: hist.c
 * Purpose: Histogram
 * Version: $Id: hist.c,v 1.36 2009-06-11 21:25:02 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/hourglass.h"
#include "oslib/osbyte.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/datastruct/list.h"
#include "appengine/wimp/menu.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/strings.h"
#include "appengine/wimp/window.h"

#include "globals.h"
#include "iconnames.h"          /* generated */
#include "menunames.h"

#include "hist.h"

/* Icon number is hardcoded due to it not being indirected. */
#define HIST_ICON 1

/* ----------------------------------------------------------------------- */

#define flag_LUMA        0
#define flag_RED         1
#define flag_GREEN       2
#define flag_BLUE        3
#define flag_ALPHA       4
#define flag_COMPS      (0x7 << 0) /* mask of components */
#define flag_CUMULATIVE (0x1 << 3)

typedef unsigned int hist_flags;

typedef struct hist_window
{
  list_t        list;
  image        *image;
  wimp_w        w;
  unsigned int *display_hist;
  hist_flags    flags;
  os_colour     colour;

  struct
  {
    int         nbars;
    int         gap;
  }
  scale;
}
hist_window;

static struct
{
  list_t        list_anchor;
  hist_window  *last_hw;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static event_wimp_handler hist__event_redraw_window_request,
                          hist__event_close_window_request,
                          hist__event_mouse_click,
                          hist__event_menu_selection;

/* ----------------------------------------------------------------------- */

static void hist__set_handlers(int reg, wimp_w w, void *handle)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, hist__event_redraw_window_request },
    { wimp_CLOSE_WINDOW_REQUEST,  hist__event_close_window_request  },
    { wimp_MOUSE_CLICK,           hist__event_mouse_click           },
    { wimp_MENU_SELECTION,        hist__event_menu_selection        },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            w, event_ANY_ICON,
                            handle);
}

/* ----------------------------------------------------------------------- */

error hist__init(void)
{
  error err;

  /* dependencies */

  err = help__init();
  if (err)
    return err;

  /* init */

  GLOBALS.hist_w = window_create("histogram");

  GLOBALS.hist_m = menu_create_from_desc(message0("menu.hist"));

  err = help__add_menu(GLOBALS.hist_m, "hist");
  if (err)
    return err;

  list__init(&LOCALS.list_anchor);

  return error_OK;
}

void hist__fin(void)
{
  help__remove_menu(GLOBALS.hist_m);

  menu_destroy(GLOBALS.hist_m);

  help__fin();
}

/* ----------------------------------------------------------------------- */

static hist_window *hist__image_to_win(image *image)
{
  return (hist_window *) list__find(&LOCALS.list_anchor,
                                     offsetof(hist_window, image),
                               (int) image);
}

/* ----------------------------------------------------------------------- */

static void hist__delete(hist_window *hw);

/* ----------------------------------------------------------------------- */

static error hist__compute(image *image)
{
  error              err;
  hist_window       *hw;
  sprite_histograms *newhists;
  sprite_histograms *hists;
  unsigned int       t;
  unsigned int       peak;
  int                i;

  err = error_OK;

  if ((image->flags & image_FLAG_CAN_HIST) == 0)
    return error_PRIVATEEYE_HIST_UNSUPP_FUNC;

  if (image->hists == NULL)
  {
    newhists = malloc(sizeof(sprite_histograms));
    if (newhists == NULL)
      return error_OOM;

    image->hists = newhists;

    hourglass_on();

    if (image->methods.histogram(&GLOBALS.choices.image, image))
    {
      free(image->hists);
      image->hists = NULL;
      return error_PRIVATEEYE_HIST_UNSUPP_FUNC;
    }

    hourglass_off();
  }

  hw = hist__image_to_win(image);

  if (hw->display_hist == NULL)
  {
    hw->display_hist = malloc(256 * sizeof(*hw->display_hist));
    if (hw->display_hist == NULL)
      return error_OOM;
  }

  hists = image->hists;

  {
    static const struct
    {
      int       index;
      os_colour colour;
    }
    map[] =
    {
      { 0, os_COLOUR_BLACK      },
      { 1, os_COLOUR_RED        },
      { 2, os_COLOUR_GREEN      },
      { 3, os_COLOUR_BLUE       },
      { 4, os_COLOUR_DARK_GREY },
    };

    int       h;
    os_colour c;

    i = hw->flags & flag_COMPS;

    h = map[i].index;
    c = map[i].colour;

    for (i = 0; i < 256; i++)
      hw->display_hist[i] = hists->h[h].v[i];

    hw->colour = c;
  }

  if (hw->flags & flag_CUMULATIVE)
  {
    /* add all the elements up, writing back */

    t = 0;
    for (i = 0; i < 256; i++)
    {
      t += hw->display_hist[i];
      hw->display_hist[i] = t;
    }

    peak = hw->display_hist[255];
  }
  else
  {
    /* work out the peak */

    t = 0;
    peak = 0;
    for (i = 0; i < 256; i++)
    {
      t += hw->display_hist[i];
      if (hw->display_hist[i] > peak)
        peak = hw->display_hist[i];
    }
  }

  /* scale to a presentable range */

  for (i = 0; i < 256; i++)
    hw->display_hist[i] = hw->display_hist[i] * 255 / peak;

  /* work out number of bars and size of gap inbetween */

  hw->scale.nbars = GLOBALS.choices.hist.bars * peak / t;
  if (hw->scale.nbars)
  {
    /* Here we multiply by 256 for the height of the box.
     * We also multiply by 256 to keep some fractional precision. */
    hw->scale.gap = 256 * 256 / hw->scale.nbars;
  }

  return err;
}

static int hist__event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_draw    *draw;
  hist_window  *hw;
  unsigned int *hist;
  os_box        b;
  int           more;

  NOT_USED(event_no);
  NOT_USED(handle);

  draw = &block->redraw;

  hw = handle;

  hist = hw->display_hist;
  if (hist == NULL)
    return event_NOT_HANDLED; /* OOM */

  icon_get_bbox(draw->w, HIST_ICON, &b);

  for (more = wimp_redraw_window(draw); more; more = wimp_get_rectangle(draw))
  {
    int x, y;
    int i;

    if (hist == NULL)
      continue;

    x = draw->box.x0 - draw->xscroll + b.x0;
    y = draw->box.y1 - draw->yscroll + b.y0;

    /* Remember that OS plot calls use inclusive upper/right bounds. */

    /* I'd previously used (os_PLOT_RECTANGLE | os_MOVE_TO) as the move
     * command for consistency, but then found that it was quicker to use a
     * plain move. */

    wimp_set_colour(wimp_COLOUR_WHITE);
    os_plot(os_MOVE_TO, x, y);
    os_plot(os_PLOT_RECTANGLE | os_PLOT_BY, b.x1 - b.x0 - 1, b.y1 - b.y0 - 1);

    wimp_set_colour(wimp_COLOUR_VERY_LIGHT_GREY);
    for (i = 0; i < hw->scale.nbars; i++)
    {
      os_plot(os_MOVE_TO, x, y + i * hw->scale.gap / 256);
      os_plot(os_PLOT_RECTANGLE | os_PLOT_BY, 512 - 1, 0);
    }

    colourtrans_set_gcol(hw->colour, 0, os_ACTION_OVERWRITE, NULL);
    for (i = 0; i < 256; i++)
    {
      unsigned int h;

      h = hist[i];
      if (h == 0)
        continue;

      os_plot(os_MOVE_TO, x + i * 2, y);
      os_plot(os_PLOT_RECTANGLE | os_PLOT_BY, 2 - 1, h);
    }
  }

  return event_HANDLED;
}

static int hist__event_close_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_close  *close;
  hist_window *hw;

  NOT_USED(event_no);
  NOT_USED(handle);

  close = &block->close;

  hw = handle;

  hist__delete(hw);

  return event_HANDLED;
}

static void hist__menu_update(hist_window *hw)
{
  menu_set_icon_flags(GLOBALS.hist_m,
                      HIST_ALPHA,
                     (hw->image->flags & image_FLAG_HAS_ALPHA) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);

  /* FIXME: Strictly this needs a mapping. */
  menu_tick_exclusive(GLOBALS.hist_m, hw->flags & flag_COMPS);
}

static void hist__refresh(hist_window *hw)
{
  error err;

  err = hist__compute(hw->image);
  if (err)
    return; // error lost

  wimp_set_icon_state(hw->w, HIST_ICON, 0, 0);
}

static int hist__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;
  hist_window  *hw;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  hw = handle;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case HISTOGRAM_O_CUMULATIVE:
      hw->flags ^= flag_CUMULATIVE;
      hist__refresh(hw);
      break;
    }
  }

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_MENU))
  {
    switch (pointer->i)
    {
    case HISTOGRAM_P_COMPS:
      hist__menu_update(hw);
      menu_popup(pointer->w, pointer->i, GLOBALS.hist_m);
      LOCALS.last_hw = hw;
      break;
    }
  }

  return event_HANDLED;
}

static int hist__event_menu_selection(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_selection *selection;
  wimp_menu      *last;
  hist_window    *hw;
  hist_flags      new_flags;
  wimp_pointer    p;

  NOT_USED(event_no);
  NOT_USED(handle);

  selection = &block->selection;

  last = menu_last();
  if (last != GLOBALS.hist_m)
    return event_NOT_HANDLED;

  hw = LOCALS.last_hw;

  switch (selection->items[0])
  {
  default:
  case HIST_LUM:   new_flags = flag_LUMA;  break;
  case HIST_R:     new_flags = flag_RED;   break;
  case HIST_G:     new_flags = flag_GREEN; break;
  case HIST_B:     new_flags = flag_BLUE;  break;
  case HIST_ALPHA: new_flags = flag_ALPHA; break;
  }

  if (new_flags != hw->flags)
  {
    hw->flags = (hw->flags & ~flag_COMPS) | new_flags;
    hist__refresh(hw);
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    hist__menu_update(hw);
    menu_reopen();
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

int hist__available(const image *image)
{
  return image &&
         image->flags & image_FLAG_CAN_HIST;
}

static void hist__image_changed_callback(image                *image,
                                         imageobserver_change  change,
                                         imageobserver_data   *data)
{
  hist_window *hw;

  NOT_USED(data);

  hw = hist__image_to_win(image);

  switch (change)
  {
  case imageobserver_CHANGE_MODIFIED:
    hist__refresh(hw);
    break;

  case imageobserver_CHANGE_HIDDEN:
  case imageobserver_CHANGE_ABOUT_TO_DESTROY:
    imageobserver_deregister(image, hist__image_changed_callback);
    hist__delete(hw);
    break;
  }
}

static error hist__new(image *image, hist_window **new_hw)
{
  error        err;
  hist_window *hw;
  const char  *leaf;
  char         title[256];

  /* no window for this image */

  hw = malloc(sizeof(*hw));
  if (hw == NULL)
    goto NoMem;

  /* clone ourselves a window */

  hw->w = window_clone(GLOBALS.hist_w);
  if (hw->w == NULL)
    goto NoMem;

  /* set its title, including the leafname of the image */

  leaf = str_leaf(image->file_name);
  sprintf(title, message0("hist.title"), leaf);
  window_set_title_text(hw->w, title);

  /* fill out */

  hw->image        = image;
  hw->display_hist = NULL;
  hw->flags        = flag_LUMA;
  hw->colour       = os_COLOUR_BLACK;

  list__add_to_head(&LOCALS.list_anchor, &hw->list);

  hist__set_handlers(1, hw->w, hw);

  err = help__add_window(hw->w, "hist");
  if (err)
    return err;

  /* watch for changes */

  imageobserver_register(image, hist__image_changed_callback);

  *new_hw = hw;

  return error_OK;


NoMem:

  if (hw)
    window_delete_cloned(hw->w);

  free(hw);

  return error_OOM;
}

static void hist__delete(hist_window *hw)
{
  imageobserver_deregister(hw->image, hist__image_changed_callback);

  help__remove_window(hw->w);

  hist__set_handlers(0, hw->w, hw);

  list__remove(&LOCALS.list_anchor, &hw->list);

  free(hw->display_hist);

  window_delete_cloned(hw->w);

  free(hw);
}

/* ----------------------------------------------------------------------- */

void hist__open(image *image)
{
  error        err;
  hist_window *hw;

  if (!hist__available(image))
  {
    beep();
    return;
  }

  hw = hist__image_to_win(image);
  if (hw == NULL)
  {
    err = hist__new(image, &hw);
    if (err)
      goto Failure;

    err = hist__compute(image);
    if (err)
      goto Failure;
  }

  window_open_at(hw->w, AT_BOTTOMPOINTER);

  return;


Failure:

  error__report(err);

  return;
}