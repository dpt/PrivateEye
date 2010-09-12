/* --------------------------------------------------------------------------
 *    Name: thumbview.c
 * Purpose: Thumbnail viewer
 * Version: $Id: thumbview.c,v 1.22 2010-01-29 15:09:00 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifdef EYE_THUMBVIEW

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/types.h"
#include "oslib/colourtrans.h"
#include "oslib/hourglass.h"
#include "oslib/os.h"
#include "oslib/osfscontrol.h"
#include "oslib/osgbpb.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/bsearch.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/numstr.h"
#include "appengine/base/os.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/dict.h"
#include "appengine/datastruct/list.h"
#include "appengine/gadgets/card.h"
#include "appengine/gadgets/filerwin.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/geom/box.h"
#include "appengine/geom/layout.h"
#include "appengine/geom/packer.h"
#include "appengine/graphics/drawable.h"
#include "appengine/io/filing.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/window.h"

#include "actions.h"
#include "globals.h"
#include "imagecache.h"
#include "keymap.h"
#include "menunames.h"
#include "scale.h"

#include "thumbview.h"

/* ----------------------------------------------------------------------- */

typedef unsigned int layout_flags;
#define layout_flag_LEFT   (1u << 0)
#define layout_flag_CENTRE (1u << 1)
#define layout_flag_RIGHT  (1u << 2)

enum
{
  InfoTextIndex_FileName, //
  InfoTextIndex_FileType,
  InfoTextIndex_Resolution,
  InfoTextIndex_Depth,
  InfoTextIndex_Size,
  InfoTextIndex__Limit,
};

typedef int element_index;
enum
{
  ElementIndex_Thumbnail,
  ElementIndex_Info,
  ElementIndex_Tags,
  ElementIndex_Filename,
  ElementIndex__Limit,
};

/* ----------------------------------------------------------------------- */

typedef int thumbview_display_mode;
enum
{
  thumbview_display_mode_LARGE_THUMBS,
  thumbview_display_mode_SMALL_THUMBS,
  thumbview_display_mode_FULL_INFO_HORIZONTAL,
  thumbview_display_mode_FULL_INFO_VERTICAL,
};

static error thumbview__set_display_mode(thumbview              *tv,
                                         thumbview_display_mode  mode);

/* ---------------------------------------------------------------------- */

typedef struct thumbview_entry
{
  image           *image;
  drawable        *drawable;
  dict_index       text[InfoTextIndex__Limit];
}
thumbview_entry;

struct thumbview
{
  list_t                  list;

  thumbview_display_mode  mode;

  dict_t                 *text; /* holds all strings */

  filerwin               *fw;

  thumbview_entry        *entries;
  int                     nentries;
  int                     maxentries;

  int                     thumb_w, thumb_h; /* dimensions of the thumbnail */
  int                     item_w, item_h;   /* dimensions of the cards */

  unsigned int            elements_present;
  struct
  {
    os_box elements[ElementIndex__Limit];
  }
  layout;
};

/* ---------------------------------------------------------------------- */

static struct
{
  list_t     list_anchor;
  thumbview *last_tv;
}
LOCALS;

/* ---------------------------------------------------------------------- */

static event_wimp_handler thumbview__event_key_pressed,
                          thumbview__event_menu_selection,
                          thumbview__event_scroll_request;

static event_message_handler thumbview__message_palette_change,
                             thumbview__message_mode_change;

static void thumbview__menu_update(void);

/* ---------------------------------------------------------------------- */

typedef int (thumbview__map_callback)(thumbview *tv, void *arg);

static void thumbview__map(thumbview__map_callback *fn, void *arg)
{
  list__walk(&LOCALS.list_anchor, (list__walk_callback *) fn, arg);
}

/* ---------------------------------------------------------------------- */

static int close_all_callback(thumbview *tv, void *arg)
{
  NOT_USED(arg);

  thumbview_destroy(tv);

  return 0;
}

void thumbview__close_all(void)
{
  thumbview__map(close_all_callback, NULL);
}

/* ----------------------------------------------------------------------- */

static void redraw(wimp_draw *redraw, int x, int y, int index, int sel, void *arg)
{
  thumbview       *tv;
  thumbview_entry *e;
  int              cx,cy;
  const char      *s;
  int              i;
  wimp_icon        icon;
  int              workarea_x, workarea_y;

  NOT_USED(sel);

  tv = arg;

  e = tv->entries + index;


  card__draw(redraw, x, y, sel ? card__draw_flag_INVERT : 0);

  for (i = 0; i < ElementIndex__Limit; i++)
  {
    os_box *b;

    b = &tv->layout.elements[i];

    if (box__is_empty(b))
      continue;

    if (i == 1)
      continue;

    colourtrans_set_gcol(0x8000 + 0x2000 * i,
                         colourtrans_SET_FG_GCOL,
                         os_ACTION_OVERWRITE,
                         0);

    os_plot(os_MOVE_TO, x + b->x0, y + b->y0);
    os_plot(os_PLOT_RECTANGLE | os_PLOT_TO, x + b->x1 - 1, y + b->y1 - 1);
  }

  // want actual thumb widths

  if (tv->elements_present & (1u << ElementIndex_Thumbnail))
  {
    cx = x + tv->layout.elements[ElementIndex_Thumbnail].x0;
    cy = y + tv->layout.elements[ElementIndex_Thumbnail].y0;

    e->drawable->methods.redraw(&GLOBALS.choices.drawable,
                                 redraw,
                                 e->drawable,
                                 cx, cy);
  }

#define HEIGHT 36

  /* this draws text using an innacurate background colour unless the icon is
   * filled */

  icon.flags = wimp_ICON_FILLED | wimp_ICON_TEXT |
               wimp_ICON_VCENTRED | wimp_ICON_INDIRECTED |
               (wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT) |
               (wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT);

  icon.data.indirected_text.validation = "";
  icon.data.indirected_text.size       = 255;

  workarea_x = x - (redraw->box.x0 - redraw->xscroll);
  workarea_y = y - (redraw->box.y1 - redraw->yscroll);

  if (tv->elements_present & (1u << ElementIndex_Info))
  {
    /* the coordinates as they arrive have been worked out in screen space.
     * we want them in work area coordinates so remove the redraw shift.
     */

    icon.extent = tv->layout.elements[ElementIndex_Info];

    icon.extent.x0 += workarea_x;
    icon.extent.x1 += workarea_x;
    icon.extent.y1 += workarea_y;
    icon.extent.y0 = icon.extent.y1 - HEIGHT;

    for (i = InfoTextIndex_FileType; i < InfoTextIndex__Limit; i++)
    {
      s = dict__string(tv->text, e->text[i]);

      icon.data.indirected_text.text = (char *) s;

      wimp_plot_icon(&icon);

      icon.extent.y0 -= HEIGHT;
      icon.extent.y1 -= HEIGHT;
    }
  }

  if (tv->elements_present & (1u << ElementIndex_Tags))
  {
    icon.extent = tv->layout.elements[ElementIndex_Tags];

    icon.extent.x0 += workarea_x;
    icon.extent.y0 += workarea_y;
    icon.extent.x1 += workarea_x;
    icon.extent.y1 += workarea_y;

    s = "tags tags tags tags tags";

    icon.data.indirected_text.text       = (char *) s;
    icon.data.indirected_text.validation = "";
    icon.data.indirected_text.size       = 255;

    wimp_plot_icon(&icon);
  }

  if (tv->elements_present & (1u << ElementIndex_Filename))
  {
    icon.flags |= wimp_ICON_HCENTRED;

    icon.extent = tv->layout.elements[ElementIndex_Filename];

    icon.extent.x0 += workarea_x;
    icon.extent.y0 += workarea_y;
    icon.extent.x1 += workarea_x;
    icon.extent.y1 += workarea_y;

    s = dict__string(tv->text, e->text[InfoTextIndex_FileName]);

    icon.data.indirected_text.text       = (char *) s;
    icon.data.indirected_text.validation = "";
    icon.data.indirected_text.size       = 255;

    wimp_plot_icon(&icon);
  }
}

/* ----------------------------------------------------------------------- */

static void close(wimp_close *close, void *arg)
{
  thumbview *tv;

  NOT_USED(close);

  tv = arg;

  thumbview_destroy(tv);
}

/* ----------------------------------------------------------------------- */

static void pointer(wimp_pointer *pointer, void *arg)
{
  thumbview *tv;
//  wimp_menu *a;

  tv = arg;

  LOCALS.last_tv = tv;

  thumbview__menu_update();

//  a = GLOBALS.thumbview_m->entries[1].sub_menu;
//  a->entries[0].sub_menu = tag_cloud__get_window_handle(tv->tc);

  menu_open(GLOBALS.thumbview_m, pointer->pos.x - 64, pointer->pos.y);
}

/* ----------------------------------------------------------------------- */

static void thumbview__reg(int reg, thumbview *tv)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_KEY_PRESSED,    thumbview__event_key_pressed    },
    { wimp_SCROLL_REQUEST, thumbview__event_scroll_request },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            filerwin__get_window_handle(tv->fw), event_ANY_ICON,
                            tv);
}

static error thumbview__set_handlers(thumbview *tv)
{
  error err;

  thumbview__reg(1, tv);

  filerwin__set_handlers(tv->fw, redraw, close, pointer);

  err = help__add_window(filerwin__get_window_handle(tv->fw), "thumbview");

  return err;
}

static void thumbview__release_handlers(thumbview *tv)
{
  help__remove_window(filerwin__get_window_handle(tv->fw));

  thumbview__reg(0, tv);
}

static void thumbview__set_single_handlers(int reg)
{
  /* menu_selection is 'vague' so should only be registered once */
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MENU_SELECTION,    thumbview__event_menu_selection },
  };
  static const event_message_handler_spec message_handlers[] =
  {
    { message_PALETTE_CHANGE, thumbview__message_palette_change },
    { message_MODE_CHANGE,    thumbview__message_mode_change    },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            NULL);

  event_register_message_group(reg,
                               message_handlers, NELEMS(message_handlers),
                               event_ANY_WINDOW, event_ANY_ICON,
                               NULL);
}

/* ----------------------------------------------------------------------- */

error thumbview__init(void)
{
  error err;

  /* dependencies */

  err = help__init();
  if (err)
    return err;

  err = tag_cloud__init();
  if (err)
    return err;

  err = viewer_keymap_init();
  if (err)
    return err;

  /* handlers */

  thumbview__set_single_handlers(1);

  /* menu */

  GLOBALS.thumbview_m = menu_create_from_desc(
                                          message0("menu.thumbview"),
                                          "(filename goes here)",
                                          NULL);

  err = help__add_menu(GLOBALS.thumbview_m, "thumbview");
  if (err)
    return err;

  list__init(&LOCALS.list_anchor);

  return error_OK;
}

void thumbview__fin(void)
{
  // discard any open thumbviews
  thumbview__close_all();

  help__remove_menu(GLOBALS.thumbview_m);

  menu_destroy(GLOBALS.thumbview_m);

  thumbview__set_single_handlers(0);

  viewer_keymap_fin();
  tag_cloud__fin();
  help__fin();
}

/* ----------------------------------------------------------------------- */

#if 0
static void disable(wimp_menu *m, int entry, int available)
{
  menu_set_icon_flags(m,
                      entry,
                      available ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);
}
#endif

static void thumbview__menu_update(void)
{
#if 0
  viewer          *viewer;
  image           *image;
  wimp_menu_entry *entries;
  wimp_menu       *m;

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;


  /* Misc menu */

  entries = GLOBALS.thumbview_m->entries;
  m = entries[IMAGE_MISC].sub_menu;

  /* Shade the "Histogram" entry if we don't have that method */
  disable(m, INFO_HIST, hist__available(image));

  /* Edit menu */

  entries = GLOBALS.thumbview_m->entries;
  m = entries[IMAGE_EDIT].sub_menu;

  /* Shade the "Effects" entry if effects module says "no" */
  disable(m, EDIT_EFFECTS, effects__available(image));

  /* Shade the "Rotate" entry if rotate module says "no" */
  disable(m, EDIT_ROTATE,  rotate__available(image));

  /* Shade the "Release clipboard" entry if we don't own the clipboard */
  disable(m, EDIT_RELEASE, clipboard_own());
#endif
}

/* ---------------------------------------------------------------------- */

static void thumbview__action(thumbview *tv, int op)
{
  error     err = error_OK;
  filerwin *fw;

  fw = tv->fw;

  switch (op)
  {
  case Thumbview_LargeThumbs:
  case Thumbview_SmallThumbs:
  case Thumbview_FullInfoHorz:
  case Thumbview_FullInfoVert:
    {
      static const thumbview_display_mode mode[] =
      {
        thumbview_display_mode_LARGE_THUMBS,
        thumbview_display_mode_SMALL_THUMBS,
        thumbview_display_mode_FULL_INFO_HORIZONTAL,
        thumbview_display_mode_FULL_INFO_VERTICAL,
      };

      err = thumbview__set_display_mode(tv, mode[op - Thumbview_LargeThumbs]);
    }
    break;

  case Thumbview_SortByCount:
    break;
  case Thumbview_SortByName:
    break;

  case Thumbview_SelectAll:
    filerwin__select(fw, -1);
    break;

  case Thumbview_ClearSelection:
    filerwin__deselect(fw, -1);
    break;

  case Close:
    err = action_close_window(filerwin__get_window_handle(tv->fw));
    break;

  case Help:
    err = action_help();
    break;
  }

  error__report(err);
}

static int thumbview__event_key_pressed(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_key  *key;
  thumbview *tv;
  int        op;

  NOT_USED(event_no);

  key = &block->key;
  tv  = handle;

  op = viewer_keymap_op(viewer_keymap_SECTION_THUMBVIEW, key->c);
  if (op < 0) /* unknown op */
  {
    wimp_process_key(key->c);
    return event_HANDLED;
  }

  thumbview__action(tv, op);

  return event_HANDLED;
}

static int thumbview__event_menu_selection(wimp_event_no event_no, wimp_block *block, void *handle)
{
#define PACK(a,b) (((a & 0xff) << 8) | (b & 0xff))

  static const struct
  {
    unsigned int items;
    int          op;
  }
  map[] =
  {
    { PACK(THUMBVIEW_DISPLAY,    DISPLAY_LARGE_THUMB),    Thumbview_LargeThumbs    },
    { PACK(THUMBVIEW_DISPLAY,    DISPLAY_SMALL_THUMB),    Thumbview_SmallThumbs    },
    { PACK(THUMBVIEW_DISPLAY,    DISPLAY_FULL_INFO_HORZ), Thumbview_FullInfoHorz   },
    { PACK(THUMBVIEW_DISPLAY,    DISPLAY_FULL_INFO_VERT), Thumbview_FullInfoVert   },
//    { PACK(THUMBVIEW_IMAGE,      TIMAGE_TAG,              0 },
//    { PACK(THUMBVIEW_IMAGE,      TIMAGE_ROTATE,           0 },
    { PACK(THUMBVIEW_SELECT_ALL, -1),                     Thumbview_SelectAll      },
    { PACK(THUMBVIEW_CLEAR_ALL,  -1),                     Thumbview_ClearSelection },
  };

  const size_t stride = sizeof(map[0]);
  const size_t nelems = sizeof(map) / stride;

  wimp_selection *selection;
  wimp_menu      *last;
  wimp_pointer    p;
  thumbview      *tv;
  unsigned int    item;
  int             i;

  NOT_USED(event_no);
  NOT_USED(handle);

  selection = &block->selection;

  last = menu_last();
  if (last != GLOBALS.thumbview_m)
    return event_NOT_HANDLED;

  item = PACK(selection->items[0], selection->items[1]);

  i = bsearch_uint(&map[0].items, nelems, stride, item);
  if (i >= 0)
  {
    tv = LOCALS.last_tv;
    if (tv == NULL)
      return event_HANDLED;

    thumbview__action(tv, map[i].op);
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    thumbview__menu_update();
    menu_reopen();
  }

  return event_HANDLED;

#undef PACK
}

static int thumbview__event_scroll_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_scroll *scroll;
  thumbview   *tv;

  NOT_USED(event_no);

  scroll = &block->scroll;
  tv     = handle;

  switch (scroll->xmin)
  {
  case wimp_SCROLL_PAGE_LEFT:
    scroll->xscroll -= scroll->visible.x1 - scroll->visible.x0;
    break;
  case wimp_SCROLL_PAGE_RIGHT:
    scroll->xscroll += scroll->visible.x1 - scroll->visible.x0;
    break;
  case wimp_SCROLL_COLUMN_LEFT:
    scroll->xscroll -= 2;
    break;
  case wimp_SCROLL_COLUMN_RIGHT:
    scroll->xscroll += 2;
    break;
  }

  switch (scroll->ymin)
  {
  case wimp_SCROLL_PAGE_UP:
    scroll->yscroll += scroll->visible.y1 - scroll->visible.y0;
    break;
  case wimp_SCROLL_PAGE_DOWN:
    scroll->yscroll -= scroll->visible.y1 - scroll->visible.y0;
    break;
  case wimp_SCROLL_LINE_UP:
    scroll->yscroll += 2;
    break;
  case wimp_SCROLL_LINE_DOWN:
    scroll->yscroll -= 2;
    break;
  }

  wimp_open_window((wimp_open *) scroll);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

typedef unsigned int thumbview__update_flags;
#define thumbview__UPDATE_COLOURS (1u << 0)
#define thumbview__UPDATE_SCALING (1u << 1)
#define thumbview__UPDATE_EXTENT  (1u << 2)
#define thumbview__UPDATE_REDRAW  (1u << 3)

static void thumbview__update(thumbview *tv, thumbview__update_flags flags)
{
  int i;

  for (i = 0; i < tv->nentries; i++)
  {
    drawable *drawable;

    drawable = tv->entries[i].drawable;

    if (flags & thumbview__UPDATE_COLOURS)
    {
      if (drawable->methods.update_colours)
      {
        drawable->methods.update_colours(drawable);
      }
    }

    if (flags & thumbview__UPDATE_SCALING)
    {
      if (drawable->methods.update_scaling)
      {
        int        s;
        os_factors factors;

        s = scale_for_box(drawable, tv->thumb_w, tv->thumb_h);
        if (s > 100)
          s = 100;

        os_factors_from_ratio(&factors, s, SCALE_100PC);

        drawable->methods.update_scaling(drawable, &factors);
      }
    }

    //if (flags & thumbview__UPDATE_EXTENT)
    //{
    //}
  }

  card__prepare(tv->item_w, tv->item_h);

  if (flags & thumbview__UPDATE_REDRAW)
    window_redraw(filerwin__get_window_handle(tv->fw));
}

static int update_all_callback(thumbview *tv, void *arg)
{
  thumbview__update(tv, (unsigned int) arg);

  return 0;
}

static int thumbview__update_all(thumbview__update_flags flags)
{
  thumbview__map(update_all_callback, (void *) flags);

  return 0;
}

static int thumbview__message_palette_change(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  thumbview__update_all(thumbview__UPDATE_COLOURS | thumbview__UPDATE_REDRAW);

  return event_PASS_ON;
}

static int thumbview__message_mode_change(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  thumbview__update_all(thumbview__UPDATE_COLOURS | thumbview__UPDATE_SCALING);

  return event_PASS_ON;
}

/* ----------------------------------------------------------------------- */

error thumbview_create(thumbview **new_tv)
{
  error      err;
  thumbview *tv   = NULL;
  dict_t    *text = NULL;
  filerwin  *fw   = NULL;

  tv = malloc(sizeof(*tv));
  if (tv == NULL)
    goto NoMem;

  text = dict__create();
  if (text == NULL)
    goto NoMem;

  fw = filerwin__create();
  if (fw == NULL)
    goto NoMem;

  filerwin__set_arg(fw, tv);

  /* fill out */

  tv->mode       = thumbview_display_mode_FULL_INFO_HORIZONTAL;
  tv->text       = text;
  tv->fw         = fw;
  tv->entries    = NULL;
  tv->nentries   = 0;
  tv->maxentries = 0;

  list__add_to_head(&LOCALS.list_anchor, &tv->list);

  thumbview__set_handlers(tv);

  *new_tv = tv;

  return error_OK;


NoMem:

  dict__destroy(text);

  free(tv);

  err = error_OOM;

  error__report(err);

  return err;
}

void thumbview_destroy(thumbview *doomed)
{
  int i;

  if (doomed == NULL)
    return;

  /* discard all entries */

  for (i = 0; i < doomed->nentries; i++)
  {
    drawable_destroy(doomed->entries[i].drawable);
    imagecache_destroy(doomed->entries[i].image);
  }

  free(doomed->entries);

  thumbview__release_handlers(doomed);

  list__remove(&LOCALS.list_anchor, &doomed->list);

  filerwin__destroy(doomed->fw);

  dict__destroy(doomed->text);

  free(doomed);
}

void thumbview_open(thumbview *tv)
{
  filerwin__open(tv->fw);
}

/* ----------------------------------------------------------------------- */

#define TEXTH 44

static error layout(thumbview *tv)
{
  error           err;

  os_box          pagedims;
  os_box          margins;

  int             thumbw, thumbh;
  int             infow, infoh;
  int             tagsw, tagsh;
  int             filew, fileh;

  unsigned int    flags;
  packer_t       *packer;
  layout_spec     spec;
  int             i;
  layout_element  els[8];
  os_box          boxes[8];
  os_box          used;
  int             y;

  /* choice values are multiplied by two to convert "pixels" to OS units */

  pagedims.x0 = 0;
  pagedims.y0 = 0;
  pagedims.x1 = GLOBALS.choices.thumbview.item_w * 2;
  pagedims.y1 = GLOBALS.choices.thumbview.item_h * 2;

  margins.x0  = margins.x1 = GLOBALS.choices.thumbview.padding_h * 2;
  margins.y0  = margins.y1 = GLOBALS.choices.thumbview.padding_v * 2;

  thumbw = GLOBALS.choices.thumbview.thumbnail_w * 2;
  thumbh = GLOBALS.choices.thumbview.thumbnail_h * 2;

  if (tv->mode == thumbview_display_mode_FULL_INFO_VERTICAL)
  {
    int t;

    t           = pagedims.x1;
    pagedims.x1 = pagedims.y1;
    pagedims.y1 = t;

    t      = thumbw;
    thumbw = thumbh;
    thumbh = t;
  }

  infow  = 1; /* expands */
  infoh  = thumbh;
  tagsw  = 1; /* expands */
  tagsh  = thumbh;
  filew  = 1; /* expands */
  fileh  = TEXTH;

#define THUMB (1u << 0)
#define SMALL (1u << 1)
#define FULL  (1u << 2)
#define TAGS  (1u << 3)
#define FILEN (1u << 4)

  flags = THUMB | FILEN;

  switch (tv->mode)
  {
  case thumbview_display_mode_LARGE_THUMBS:
    break;
  case thumbview_display_mode_SMALL_THUMBS:
    flags |= SMALL;
    thumbw /= 2;
    thumbh /= 2;
    // could adjust infow/h but we won't use that mode yet
    break;
  case thumbview_display_mode_FULL_INFO_HORIZONTAL:
  case thumbview_display_mode_FULL_INFO_VERTICAL:
    flags |= FULL;
    flags |= TAGS;
    break;
  }

  packer = packer__create(&pagedims);
  if (packer == NULL)
  {
    err = error_OOM;
    goto failure;
  }

  packer__set_margins(packer, &margins);

  spec.packer  = packer;
  spec.loc     = packer__LOC_TOP_LEFT;
  spec.clear   = packer__CLEAR_LEFT;
  spec.spacing = margins.x0 / 2; /* use padding config */
  spec.leading = margins.y0 / 2;

  i = 0;

  if (flags & THUMB)
  {
    els[i].type               = layout_BOX;
    els[i].data.box.min_width = thumbw;
    els[i].data.box.max_width = thumbw;
    els[i].data.box.height    = thumbh;
    i++;
  }

  if (flags & FULL)
  {
    els[i].type               = layout_BOX;
    els[i].data.box.min_width = infow;
    els[i].data.box.max_width = 512;
    els[i].data.box.height    = infoh;
    i++;
  }

  if (flags & TAGS)
  {
    els[i].type               = layout_BOX;
    els[i].data.box.min_width = 100;
    els[i].data.box.max_width = 100;
    els[i].data.box.height    = 100;
    i++;
  }

  if (flags & FILEN)
  {
    els[i].type               = layout_NEWLINE;
    i++;

    els[i].type               = layout_BOX;
    els[i].data.box.min_width = 1;
    els[i].data.box.max_width = INT_MAX;
    els[i].data.box.height    = 44;
    i++;
  }

  err = layout__place(&spec, els, i, boxes, NELEMS(boxes));
  if (err)
    goto failure;

  /* invalidate all layout elements */

  tv->elements_present = 0;

  for (i = 0; i < ElementIndex__Limit; i++)
  {
    tv->layout.elements[i].x0 = INT_MAX;
    tv->layout.elements[i].y0 = INT_MAX;
    tv->layout.elements[i].x1 = INT_MIN;
    tv->layout.elements[i].y1 = INT_MIN;
  }

  i = 0;

  if (flags & THUMB)
    tv->layout.elements[ElementIndex_Thumbnail] = boxes[i++];
  if (flags & FULL)
    tv->layout.elements[ElementIndex_Info]      = boxes[i++];
  if (flags & TAGS)
    tv->layout.elements[ElementIndex_Tags]      = boxes[i++];
  if (flags & FILEN)
    tv->layout.elements[ElementIndex_Filename]  = boxes[i++];

  used = *packer__get_consumed_area(packer);

  /* re-add the margins, which get_consumed_area does not account for */

  used.x0 -= margins.x0;
  used.y0 -= margins.y0;
  used.x1 += margins.x1;
  used.y1 += margins.y1;

  /* move all elements down by y0 */

  y = used.y0;

  for (i = 0; i < ElementIndex__Limit; i++)
  {
    os_box *b;

    b = &tv->layout.elements[i];

    if (box__is_empty(b))
      continue;

    b->y0 -= y;
    b->y1 -= y;

    box__round4(b);

    tv->elements_present |= 1u << i;
  }

  tv->item_w = used.x1 - used.x0; // want final values
  tv->item_h = used.y1 - used.y0;

  packer__destroy(packer);

  // min item size influenced by sizes of edges

  filerwin__set_nobjects(tv->fw, tv->nentries);

  filerwin__set_padding(tv->fw, GLOBALS.choices.thumbview.padding_h,
                                GLOBALS.choices.thumbview.padding_v);

  filerwin__set_dimensions(tv->fw, tv->item_w, tv->item_h);

  return error_OK;


failure:

  return err;
}

static error load_directory_cb(const char          *obj_name,
                               osgbpb_info_stamped *info,
                               void                *arg)
{
  error            err;
  thumbview       *tv = arg;
  thumbview_entry *entry;
  image           *image;
  drawable        *drawable;

  if (!image_is_loadable(info->file_type))
    return error_OK;

  if (tv->nentries == tv->maxentries)
  {
    int   newmaxentries;
    void *newentries;

    newmaxentries = tv->maxentries * 2;
    if (newmaxentries < 8)
      newmaxentries = 8;

    newentries = realloc(tv->entries, newmaxentries * sizeof(*tv->entries));
    if (newentries == NULL)
      return error_OOM;

    tv->entries    = newentries;
    tv->maxentries = newmaxentries;
  }

  entry = tv->entries + tv->nentries;
  tv->nentries++;

  /* image */

  /* is it in the cache? */

  err = imagecache_get(obj_name, info->load_addr, info->exec_addr, &image);
  if (err)
    goto Failure;

  if (image == NULL)
  {
    /* it's not in the cache */

    image = image_create_from_file(&GLOBALS.choices.image,
                                    obj_name,
                                    info->file_type);
    if (image == NULL)
    {
      err = error_OOM;
      goto Failure;
    }
  }

  // will need to cope better with failure to load/access images

  /* drawable */

  err = drawable_create(image, &drawable);
  if (err)
    goto Failure;

  entry->image    = image;
  entry->drawable = drawable;

  {
    const char *leaf;
    char        buf[256];
    int         w,h;

    /* filename */

    leaf = strrchr(obj_name, '.');
    if (leaf)
      leaf += 1;
    else
      leaf = obj_name;

    err = dict__add(tv->text, leaf, &entry->text[0]);
    if (err && err != error_DICT_NAME_EXISTS)
      goto Failure;

    /* file type */

    file_type_to_name(info->file_type, buf);

    err = dict__add(tv->text, buf, &entry->text[1]);
    if (err && err != error_DICT_NAME_EXISTS)
      goto Failure;

    /* resolution */

    w = image->source.dims.bm.width;
    h = image->source.dims.bm.height;

    comma_number(w, buf, 12);
    comma_number(h, buf + 12, 12);

    sprintf(buf + 24, "%s x %s", buf, buf + 12);

    err = dict__add(tv->text, buf + 24, &entry->text[2]);
    if (err && err != error_DICT_NAME_EXISTS)
      goto Failure;

    /* depth */

    sprintf(buf, "%dbpp", image->source.dims.bm.bpp);

    err = dict__add(tv->text, buf, &entry->text[3]);
    if (err && err != error_DICT_NAME_EXISTS)
      goto Failure;

    /* size */

    comma_number(image->source.file_size, buf, 12);

    err = dict__add(tv->text, buf, &entry->text[4]);
    if (err && err != error_DICT_NAME_EXISTS)
      goto Failure;
  }

  return error_OK;


Failure:

  return err;
}

void thumbview_load_dir(thumbview *tv, const char *dir_name)
{
  error err;

  tv->thumb_w = GLOBALS.choices.thumbview.thumbnail_w * 2;
  tv->thumb_h = GLOBALS.choices.thumbview.thumbnail_h * 2;

  dirscan(dir_name, load_directory_cb, dirscan_FILES, tv);

  filerwin__set_window_title(tv->fw, dir_name);

  // once we have all the directory details we can go about calculating what
  // the layout should be

  err = layout(tv);
  if (err)
    goto Failure;

  thumbview__update(tv, thumbview__UPDATE_COLOURS |
                        thumbview__UPDATE_SCALING |
                        thumbview__UPDATE_EXTENT);

  return;


Failure:

  return;
}

/* ----------------------------------------------------------------------- */

static error thumbview__set_display_mode(thumbview              *tv,
                                         thumbview_display_mode  mode)
{
  error err;

  tv->mode = mode;

  err = layout(tv);
  if (err)
    return err;

  thumbview__update(tv, thumbview__UPDATE_REDRAW);

  return err;
}

#else

extern int dummy;

#endif /* EYE_THUMBVIEW */
