/* --------------------------------------------------------------------------
 *    Name: hist.c
 * Purpose: Histogram windows
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/hourglass.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/os.h"
#include "appengine/base/strings.h"
#include "appengine/gadgets/imageobwin.h"
#include "appengine/graphics/image.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/hist.h"

/* ----------------------------------------------------------------------- */

static imageobwin_alloc   hist_alloc;
static imageobwin_dealloc hist_dealloc;
static imageobwin_compute hist_compute;
static imageobwin_refresh hist_refresh;

static event_wimp_handler hist_event_redraw_window_request,
                          hist_event_mouse_click,
                          hist_event_menu_selection;

/* ----------------------------------------------------------------------- */

/* Icons for window "histogram" */
enum
{
  HISTOGRAM_D_DISPLAY    = 0,
  HISTOGRAM_O_CUMULATIVE = 3,
  HISTOGRAM_P_COMPS      = 5
};

/* Icon number is hardcoded due to it not being indirected. */
#define HIST_ICON 1

/* ----------------------------------------------------------------------- */

/* Menus */
enum
{
  HIST_LUM,
  HIST_R,
  HIST_G,
  HIST_B,
  HIST_ALPHA,
};

/* ----------------------------------------------------------------------- */

typedef struct hist_config
{
  int nbars; /* number of bars */
}
hist_config;

/* ----------------------------------------------------------------------- */

#define flag_LUMA        0
#define flag_RED         1
#define flag_GREEN       2
#define flag_BLUE        3
#define flag_ALPHA       4
/* values 5..7 unused */
#define flag_COMPS      (0x7 << 0) /* mask of component indices */
#define flag_CUMULATIVE (0x1 << 3)

typedef unsigned int hist_flags;

typedef struct hist_window
{
  imageobwin_t  base;         /* a hist_window is an image observer window */

  hist_config   config;

  unsigned int *display_hist; /* histogram data */
  hist_flags    flags;        /* flags (see above) */
  os_colour     colour;       /* colour to draw the bars */
  struct
  {
    int         nbars;        /* number of horizontal bars to draw */
    int         gap;          /* gap inbetween bars */
  }
  scale;
}
hist_window;

static struct
{
  imageobwin_factory_t factory;
  int                  refcount;
}
LOCALS;

/* ----------------------------------------------------------------------- */

result_t hist_init(void)
{
  result_t err;

  if (LOCALS.refcount)
    return result_OK; /* already initialised */

  /* dependencies */

  err = imageobwin_construct(&LOCALS.factory,
                              "histogram",
                              AT_BOTTOMPOINTER,
                              hist_available,
                              hist_alloc,
                              hist_dealloc,
                              hist_compute,
                              hist_refresh,
                              hist_event_redraw_window_request,
                              hist_event_mouse_click,
                              hist_event_menu_selection);
  if (err)
    return err;

  LOCALS.refcount++;

  return result_OK;
}

void hist_fin(void)
{
  if (LOCALS.refcount == 0)
    return;

  imageobwin_destruct(&LOCALS.factory);

  LOCALS.refcount--;
}

/* ----------------------------------------------------------------------- */

void hist_open(image_t *image, int nbars)
{
  hist_config config;

  config.nbars = nbars;

  imageobwin_open(&LOCALS.factory, image, &config);
}

int hist_available(const image_t *image)
{
  return image &&
         image->flags & image_FLAG_CAN_HIST;
}

/* ----------------------------------------------------------------------- */

static result_t hist_alloc(const void    *opaque_config,
                           imageobwin_t **obwin)
{
  hist_window       *self;
  const hist_config *config = opaque_config;

  *obwin = NULL;

  self = malloc(sizeof(*self));
  if (self == NULL)
    return result_OOM;

  self->config       = *config;

  self->display_hist = NULL;
  self->flags        = flag_LUMA;
  self->colour       = os_COLOUR_BLACK;

  *obwin = &self->base;

  return result_OK;
}

static void hist_dealloc(imageobwin_t *doomed)
{
  hist_window *self = (hist_window *) doomed;

  free(self->display_hist);

  free(self);
}

static result_t hist_compute(imageobwin_t *obwin)
{
  hist_window       *self;
  image_t           *image;
  sprite_histograms *newhists;
  sprite_histograms *hists;
  unsigned int       t;
  unsigned int       peak;
  int                i;

  self  = (hist_window *) obwin;
  image = self->base.image;

  if (image->hists == NULL)
  {
    newhists = malloc(sizeof(sprite_histograms));
    if (newhists == NULL)
      return result_OOM;

    image->hists = newhists;

    hourglass_on();

    if (image->methods.histogram(image))
    {
      free(image->hists);
      image->hists = NULL;
      return result_PRIVATEEYE_HIST_UNSUPP_FUNC;
    }

    hourglass_off();
  }

  if (self->display_hist == NULL)
  {
    self->display_hist = malloc(256 * sizeof(*self->display_hist));
    if (self->display_hist == NULL)
      return result_OOM;
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
      { 0, os_COLOUR_BLACK     },
      { 1, os_COLOUR_RED       },
      { 2, os_COLOUR_GREEN     },
      { 3, os_COLOUR_BLUE      },
      { 4, os_COLOUR_DARK_GREY },
    };

    int       h;
    os_colour c;

    i = self->flags & flag_COMPS;

    h = map[i].index;
    c = map[i].colour;

    for (i = 0; i < 256; i++)
      self->display_hist[i] = hists->h[h].v[i];

    self->colour = c;
  }

  if (self->flags & flag_CUMULATIVE)
  {
    /* add all the elements up, writing back */

    t = 0;
    for (i = 0; i < 256; i++)
    {
      t += self->display_hist[i];
      self->display_hist[i] = t;
    }

    peak = self->display_hist[255];
  }
  else
  {
    /* work out the peak */

    t = 0;
    peak = 0;
    for (i = 0; i < 256; i++)
    {
      t += self->display_hist[i];
      if (self->display_hist[i] > peak)
        peak = self->display_hist[i];
    }
  }

  /* scale to a presentable range */

  for (i = 0; i < 256; i++)
    self->display_hist[i] = self->display_hist[i] * 255 / peak;

  /* work out number of bars and size of gap inbetween */

  self->scale.nbars = self->config.nbars * peak / t;
  if (self->scale.nbars)
  {
    /* Here we multiply by 256 for the height of the box.
     * We also multiply by 256 to keep some fractional precision. */
    self->scale.gap = 256 * 256 / self->scale.nbars;
  }

  return result_OK;
}

static void hist_refresh(imageobwin_t *obwin)
{
  hist_window *self = (hist_window *) obwin;

  wimp_set_icon_state(self->base.w, HIST_ICON, 0, 0);
}

/* ----------------------------------------------------------------------- */

static int hist_event_redraw_window_request(wimp_event_no event_no,
                                            wimp_block   *block,
                                            void         *handle)
{
  hist_window  *self;
  wimp_draw    *draw;
  unsigned int *hist;
  os_box        b;
  int           more;

  NOT_USED(event_no);

  self = handle;
  draw = &block->redraw;

  hist = self->display_hist;
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
    for (i = 0; i < self->scale.nbars; i++)
    {
      os_plot(os_MOVE_TO, x, y + i * self->scale.gap / 256);
      os_plot(os_PLOT_RECTANGLE | os_PLOT_BY, 512 - 1, 0);
    }

    colourtrans_set_gcol(self->colour, 0, os_ACTION_OVERWRITE, NULL);
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

static void hist_menu_update(hist_window *self);

static int hist_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  hist_window  *self;
  wimp_pointer *pointer;

  NOT_USED(event_no);

  self    = handle;
  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case HISTOGRAM_O_CUMULATIVE:
      self->flags ^= flag_CUMULATIVE;
      imageobwin_kick(&self->base);
      break;
    }
  }

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_MENU))
  {
    switch (pointer->i)
    {
    case HISTOGRAM_P_COMPS:
      hist_menu_update(self);
      menu_popup(pointer->w, pointer->i, self->base.factory->menu);
      break;
    }
  }

  return event_HANDLED;
}

static void hist_menu_update(hist_window *self)
{
  menu_set_icon_flags(LOCALS.factory.menu,
                      HIST_ALPHA,
                     (self->base.image->flags & image_FLAG_HAS_ALPHA) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);

  /* FIXME: Strictly this needs to be a mapping from flags to menu indices.
   */
  menu_tick_exclusive(LOCALS.factory.menu, self->flags & flag_COMPS);
}

static int hist_event_menu_selection(wimp_event_no event_no,
                                     wimp_block   *block,
                                     void         *handle)
{
  hist_window    *self;
  wimp_selection *selection;
  wimp_menu      *last;
  hist_flags      new_flags;
  wimp_pointer    p;

  NOT_USED(event_no);

  self      = handle;
  selection = &block->selection;

  last = menu_last();
  if (last != self->base.factory->menu)
    return event_NOT_HANDLED;

  switch (selection->items[0])
  {
  default:
  case HIST_LUM:   new_flags = flag_LUMA;  break;
  case HIST_R:     new_flags = flag_RED;   break;
  case HIST_G:     new_flags = flag_GREEN; break;
  case HIST_B:     new_flags = flag_BLUE;  break;
  case HIST_ALPHA: new_flags = flag_ALPHA; break;
  }

  if (new_flags != self->flags)
  {
    self->flags = (self->flags & ~flag_COMPS) | new_flags;
    imageobwin_kick(&self->base);
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    hist_menu_update(self);
    menu_reopen();
  }

  return event_HANDLED;
}

