/* --------------------------------------------------------------------------
 *    Name: effects.c
 * Purpose: Effects dialogue
 * Version: $Id: effects.c,v 1.45 2010-01-06 01:58:53 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "swis.h"

#include <limits.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/colourpicker.h"
#include "oslib/colourtrans.h"
#include "oslib/draganobject.h"
#include "oslib/hourglass.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/graphics/colour.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/gadgets/scroll-list.h"
#include "appengine/gadgets/slider.h"
#include "appengine/graphics/sprite-effects.h"
#include "appengine/vdu/sprite.h"
#include "appengine/gadgets/tonemap-gadget.h"
#include "appengine/graphics/tonemap.h"
#include "appengine/base/bsearch.h"

#include "globals.h"
#include "iconnames.h"          /* generated */
#include "iconnames2.h"         /* not generated */
#include "menunames.h"

#include "effects.h"

#define HEIGHT     44
#define ICONWIDTH  44
#define TEXTWIDTH 460 /* assuming 512 wide window */

/* ---------------------------------------------------------------------- */

typedef int effect_effect;
enum
{
  effect_CLEAR,
  effect_TONE,
  effect_GREY,
  effect_BLUR,
  effect_SHARPEN,
  effect_EXPAND,
  effect_EQUALISE,
  effect_EMBOSS,
};

static const char *effect_to_name(effect_effect e)
{
  static const char *names[] =
  {
    "effect.clear",
    "effect.tone",
    "effect.grey",
    "effect.blur",
    "effect.sharpen",
    "effect.dynamic",
    "effect.equalise",
    "effect.emboss",
  };

  return names[e];
}

static const char *effect_to_sprite(effect_effect e)
{
  static const char *names[] =
  {
    "fx-clear",
    "fx-tone",
    "fx-grey",
    "fx-blur",
    "fx-sharpen",
    "fx-dynamic",
    "fx-equalise",
    "fx-emboss",
  };

  return names[e];
}

/* ---------------------------------------------------------------------- */

typedef struct
{
  os_colour colour;
}
effect_clear;

typedef struct
{
  effects_blur_method method;
  int                 level;
}
effect_blur;

typedef struct
{
  tonemap          *map;
  tonemap_channels  channels;
  tonemap_spec      spec;
}
effect_tone;

typedef union
{
  effect_clear clear;
  effect_blur  blur;
  effect_tone  tone;
}
effect_args;

typedef struct
{
  effect_effect effect;
  effect_args   args;
}
effect_element;

typedef struct
{
  effect_element *entries;
  int             nentries;
  int             allocated;
}
effect_array;

/* ----------------------------------------------------------------------- */

static effect_array    effects;
static effect_element *editing_element;
static scroll_list    *sl;

static struct
{
  image  *image;
  blender blender;
  int     blendval;
  int     open;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static void effects__close(void);
static void effects__apply(void);
static void effects__cancel(void);

/* ---------------------------------------------------------------------- */

typedef int (*Editor)(effect_element *e, int x, int y);
typedef error (*SetDefaults)(effect_element *e);
typedef error (*Apply)(osspriteop_area *area, osspriteop_header *src, osspriteop_header *dst, effect_element *e);

/* ---------------------------------------------------------------------- */

static error clear_apply(osspriteop_area *area, osspriteop_header *src, osspriteop_header *dst, effect_element *e)
{
  NOT_USED(src);

  return effects_clear_apply(area, src, dst, e->args.clear.colour);
}

static error clear_defaults(effect_element *e)
{
  e->args.clear.colour = 0x80808000;

  return error_OK;
}

static error tone_apply(osspriteop_area *area, osspriteop_header *src, osspriteop_header *dst, effect_element *e)
{
  return effects_tonemap_apply(area, src, dst, e->args.tone.map);
}

static void effect_tone_reset(effect_tone *t)
{
  /* reset all values to their defaults */

  // think this needs to take alpha into account
  tonemap_reset(t->map);

  t->channels        = tonemap_CHANNEL_RGB;
  t->spec.flags      = 0;
  t->spec.gamma      = 100;
  t->spec.brightness = 100;
  t->spec.contrast   = 100;
  t->spec.bias       = 50;
  t->spec.gain       = 50;
}

static error tone_defaults(effect_element *e)
{
  effect_tone *t;

  t = &e->args.tone;

  t->map = tonemap_create();
  if (t->map == NULL)
     return error_OOM;

  tonemap_draw_set_stroke_width(t->map, GLOBALS.choices.effects.curve_width);

  effect_tone_reset(t);

  return error_OK;
}

static error grey_apply(osspriteop_area *area, osspriteop_header *src,
                        osspriteop_header *dst, effect_element *e)
{
  NOT_USED(e);

  return effects_grey_apply(area, src, dst);
}

static error blur_defaults(effect_element *e)
{
  e->args.blur.method = effects_blur_GAUSSIAN;
  e->args.blur.level  = 2;

  return error_OK;
}

static error blur_apply(osspriteop_area *area, osspriteop_header *src,
                        osspriteop_header *dst, effect_element *e)
{
  NOT_USED(e);

  return effects_blur_apply(area, src, dst,
                            e->args.blur.method, e->args.blur.level);
}

static error sharpen_apply(osspriteop_area *area, osspriteop_header *src,
                           osspriteop_header *dst, effect_element *e)
{
  NOT_USED(e);

  return effects_sharpen_apply(area, src, dst, 0 /* unused */);
}

static error expand_apply(osspriteop_area *area, osspriteop_header *src,
                          osspriteop_header *dst, effect_element *e)
{
  NOT_USED(e);

  return effects_expand_apply(area, src, dst, 0 /* threshold */);
}

static error equalise_apply(osspriteop_area *area, osspriteop_header *src,
                            osspriteop_header *dst, effect_element *e)
{
  NOT_USED(e);

  return effects_equalise_apply(area, src, dst);
}

static error emboss_apply(osspriteop_area *area, osspriteop_header *src,
                          osspriteop_header *dst, effect_element *e)
{
  NOT_USED(area);
  NOT_USED(src);
  NOT_USED(dst);
  NOT_USED(e);

  return error_OK;
}

/* ---------------------------------------------------------------------- */

/* editors */
static int clear_edit(effect_element *effect, int x, int y);
static int blur_edit(effect_element *effect, int x, int y);
static int tone_edit(effect_element *effect, int x, int y);

static const struct
{
  Editor      editor;
  SetDefaults set_defaults;
  Apply       apply;
}
editors[] =
{
  { clear_edit, clear_defaults, clear_apply,   },
  { tone_edit,  tone_defaults,  tone_apply,    },
  { NULL,       NULL,           grey_apply     },
  { blur_edit,  blur_defaults,  blur_apply     },
  { NULL,       NULL,           sharpen_apply  },
  { NULL,       NULL,           expand_apply   },
  { NULL,       NULL,           equalise_apply },
  { NULL,       NULL,           emboss_apply   },
};

/* ---------------------------------------------------------------------- */

#define AppAcc_Status 0x58982
#define AppAcc_Copy   0x58985

typedef void *(memcpyfn)(void *dst, const void *src, size_t n);

static void *memcpy_appacc(void *dst, const void *src, size_t n)
{
  _swi(AppAcc_Copy, _INR(0,1)|_INR(5,6), 0, src, dst, n);

  return dst;
}

static memcpyfn *choose_memcpy(void)
{
  _kernel_oserror *err;
  int              busy;

  err = _swix(AppAcc_Status, _IN(0)|_OUT(0), 0, &busy);
  if (err == NULL && busy == 0) /* hmmm */
    return memcpy_appacc;
  else
    return memcpy;
}

/* ---------------------------------------------------------------------- */

static void apply_blend(int val)
{
  LOCALS.blender.make_lut(val, 0, &LOCALS.blender);
  LOCALS.blender.blend(&LOCALS.blender);

  image_preview(LOCALS.image);
}

static void copy_sprite_data(osspriteop_area *area,
                             int              src_index,
                             int              dst_index)
{
  static memcpyfn   *cpy = NULL;

  osspriteop_header *src;
  osspriteop_header *dst;
  void              *src_data;
  void              *dst_data;

  src = sprite_select(area, src_index);
  dst = sprite_select(area, dst_index);

  src_data = sprite_data(src);
  dst_data = sprite_data(dst);

  if (cpy == NULL)
    cpy = choose_memcpy();

  cpy(dst_data, src_data, src->size - sizeof(osspriteop_header));
}

static error apply_effects(void)
{
  error            err;
  effect_array    *v;
  image           *image;
  osspriteop_area *area;

  err = error_OK;

  hourglass_on();

  image = LOCALS.image;
  area  = (osspriteop_area *) image->image;

  v = &effects;

  if (v->nentries)
  {
    osspriteop_header *src;
    osspriteop_header *dst;
    int                i;

    /* apply each effect in turn */

    /* The initial pass processes the source image into the destination.
     * After that the degenerate is repeatedly reprocessed. */

    src = sprite_select(area, 0);
    dst = sprite_select(area, 1);

    for (i = 0; i < v->nentries; i++)
    {
      effect_element *e;

      e = v->entries + i;

      err = editors[e->effect].apply(area, src, dst, e);
      if (err)
        goto Exit;

      src = dst; /* use degenerate from now on */

      hourglass_percentage((i + 1) * 100 / v->nentries);
    }
  }
  else
  {
    /* There are no effects. without anything to apply the degenerate image
     * won't get filled in, so just do a straight copy. */

    /* FIXME: This is all wasted work which could be avoided by not creating
     *        the preview until an effect is applied. */

    copy_sprite_data(area, 0, 1);
  }

  /* kick the blender */

  apply_blend(LOCALS.blendval);

  /* FALLTHROUGH */

Exit:

  hourglass_off();

  return err;
}

static int insert_effect(effect_effect effect, int where)
{
  effect_array   *v;
  effect_element *e;
  size_t          shiftamt;

  v = &effects;

  if (v->nentries == v->allocated)
  {
    int   n;
    void *newentries;

    /* doubling strategy */

    n = v->allocated * 2;
    if (n < 1)
      n = 1;

    newentries = realloc(v->entries, sizeof(*v->entries) * n);
    if (newentries == NULL)
      return 1; /* failure: out of memory */

    v->entries = newentries;
    v->allocated = n;
  }

  if (where == -1) /* at end */
    where = v->nentries;

  shiftamt = (v->nentries - where) * sizeof(v->entries[0]);
  if (shiftamt)
    memmove(v->entries + where + 1, v->entries + where, shiftamt);

  e = &v->entries[where];

  e->effect = effect;
  if (editors[e->effect].set_defaults)
    editors[e->effect].set_defaults(e);

  v->nentries++;

  scroll_list__add_row(sl);

  apply_effects();

  return 0; /* success */
}

static int move_effect(int from, int to)
{
  effect_array   *v;
  effect_element  t; /* temporary element */
  int             down;
  void           *dst;
  void           *src;
  int             n;
  int             i;

  if (to == from)
    return 0; /* nothing to do */

  v = &effects;

  down = to > from; /* the direction we need to move elements */

  /* save the element which will be overwritten */
  t = v->entries[from];

  if (down)
  {
    /* element moving up; shifting down to make space */
    dst = v->entries + from;
    src = v->entries + from + 1;
    n   = to - from;
  }
  else
  {
    /* element moving down; shifting up to make space */
    dst = v->entries + to + 1;
    src = v->entries + to;
    n   = from - to;
  }

  if (n)
    memmove(dst, src, n * sizeof(t));

  /* restore the overwritten element in its new place */
  v->entries[to] = t;

  /* refresh all affected rows */
  {
    int min,max;

    if (down)
    {
      min = from;
      max = to;
    }
    else
    {
      min = to;
      max = from;
    }

    for (i = min; i <= max; i++)
      scroll_list__refresh_row(sl, i);
  }

  apply_effects();

  return 0; /* success */
}

static int is_editable(effect_element *e)
{
  return editors[e->effect].editor != NULL;
}

/* Shade the "Edit..." and "Delete" buttons. */
static void new_selection(void)
{
  int             sel;
  wimp_icon_flags edit;
  wimp_icon_flags del;

  edit = wimp_ICON_SHADED;
  del  = wimp_ICON_SHADED;

  sel = scroll_list__get_selection(sl);

  if (sel >= 0)
  {
    effect_array   *v;
    effect_element *e;

    v = &effects;
    e = &v->entries[sel];

    if (is_editable(e))
      edit = 0;

    del = 0;
  }

  icon_set_flags(GLOBALS.effects_w, EFFECTS_B_EDIT,  edit, wimp_ICON_SHADED);
  icon_set_flags(GLOBALS.effects_w, EFFECTS_B_DELETE, del, wimp_ICON_SHADED);
}

static void new_effects(void)
{
  static const wimp_i icons[] =
  {
    EFFECTS_B_APPLY,
    EFFECTS_S_BLEND_MARKER1,
    EFFECTS_S_BLEND_MARKER2,
    EFFECTS_S_BLEND_PIT,
    EFFECTS_S_BLEND_BACKGROUND,
    EFFECTS_S_BLEND_FOREGROUND
  };

  wimp_icon_flags eor, clear;

  eor   = (effects.nentries > 0) ? 0 : wimp_ICON_SHADED;
  clear = wimp_ICON_SHADED;

  icon_group_set_flags(GLOBALS.effects_w, icons, NELEMS(icons), eor, clear);
}

static void edit_effect(int index)
{
  effect_array   *v;
  effect_element *e;
  wimp_pointer    pointer;
  Editor          editor;

  v = &effects;

  e = &v->entries[index];

  wimp_get_pointer_info(&pointer);

  editing_element = e;

  editor = editors[e->effect].editor;
  if (editor)
    editor(e, pointer.pos.x, pointer.pos.y);
}

static void effect_edited(effect_element *e)
{
  effect_array *v;
  int           i;

  v = &effects;

  i = e - v->entries;

  scroll_list__refresh_row(sl, i);

  apply_effects();
}

static void remove_effect(int index)
{
  effect_array *v;
  size_t        s;

  if (index == -1)
    return;

  v = &effects;

  s = (v->nentries - index - 1) * sizeof(v->entries[0]);
  if (s)
    memmove(v->entries + index, v->entries + index + 1, s);

  v->nentries--;

  scroll_list__delete_rows(sl, index, index);

  new_selection();
  new_effects();

  apply_effects(); /* kick the effects + blender */
}

static void remove_all_effects(void)
{
  effect_array *v;

  v = &effects;

  /* Retain the block, but empty it. */

  v->nentries = 0;

  scroll_list__delete_rows(sl, 0, INT_MAX); /* all */

  // call new_selection()?

  apply_effects(); /* kick the effects + blender */
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler main_event_mouse_click,

                          add_event_mouse_click,

                          blur_event_mouse_click,

                          tone_event_redraw_window_request,
                          tone_event_mouse_click;

static event_message_handler clear_message_colour_picker_colour_choice;

/* ----------------------------------------------------------------------- */

static void redraw_element(wimp_draw *redraw, int wax, int way, int i, int sel);
static void redraw_leader(wimp_draw *redraw, int wax, int way, int i, int sel);
static void scroll_list_event(scroll_list__event *event);

static error init_main(void)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK, main_event_mouse_click },
  };

  error err;

  GLOBALS.effects_w = window_create("effects");

  err = help__add_window(GLOBALS.effects_w, "effects");
  if (err)
    return err;

  event_register_wimp_group(1,
                            wimp_handlers, NELEMS(wimp_handlers),
                            GLOBALS.effects_w, event_ANY_ICON,
                            NULL);

  sl = scroll_list__create(GLOBALS.effects_w, EFFECTS_I_PANE_PLACEHOLDER);
  if (sl == NULL)
    return error_OOM; // potentially inaccurate

  scroll_list__set_row_height(sl, HEIGHT, 4);
  scroll_list__set_handlers(sl, redraw_element, redraw_leader, scroll_list_event);

  err = help__add_window(scroll_list__get_window_handle(sl), "effects_list");
  if (err)
    return err;

  return error_OK;
}

static void fin_main(void)
{
  help__remove_window(scroll_list__get_window_handle(sl));

  scroll_list__destroy(sl);

  help__remove_window(GLOBALS.effects_w);
}

static error init_add(void)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK, add_event_mouse_click },
  };

  error err;

  GLOBALS.effects_add_w = window_create("effects_add");

  err = help__add_window(GLOBALS.effects_add_w, "effects_add");
  if (err)
    return err;

  event_register_wimp_group(1,
                            wimp_handlers, NELEMS(wimp_handlers),
                            GLOBALS.effects_add_w, event_ANY_ICON,
                            NULL);

  return error_OK;
}

static void fin_add(void)
{
  help__remove_window(GLOBALS.effects_add_w);
}

#define MAGIC_PICKER ((wimp_w) 0x1)

static error init_clear(void)
{
  return error_OK;
}

static void fin_clear(void)
{
}

static error init_blur(void)
{
  error err;

  err = dialogue__construct(&GLOBALS.effects_blr_d, "effects_blr",
                            EFFECTS_BLR_B_APPLY, EFFECTS_BLR_B_CANCEL);
  if (err)
    return err;

  dialogue__set_handlers(&GLOBALS.effects_blr_d,
                          blur_event_mouse_click,
                          NULL,
                          NULL);

  return error_OK;
}

static void fin_blur(void)
{
  dialogue__destruct(&GLOBALS.effects_blr_d);
}

static error init_tone(void)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, tone_event_redraw_window_request },
    { wimp_MOUSE_CLICK,           tone_event_mouse_click           },
  };

  error err;

  GLOBALS.effects_crv_w  = window_create("effects_crv");

  err = help__add_window(GLOBALS.effects_crv_w, "effects_crv");
  if (err)
    return err;

  event_register_wimp_group(1,
                            wimp_handlers, NELEMS(wimp_handlers),
                            GLOBALS.effects_crv_w, event_ANY_ICON,
                            NULL);

  return error_OK;
}

static void fin_tone(void)
{
  help__remove_window(GLOBALS.effects_crv_w);
}

error effects__init(void)
{
  error err;

  /* dependencies */

  err = help__init();
  if (err)
    return err;

  /* modules */

  init_main();
  init_add();
  init_clear();
  init_blur();
  init_tone();

  return error_OK;
}

void effects__fin(void)
{
  /* modules */

  fin_tone();
  fin_blur();
  fin_clear();
  fin_add();
  fin_main();

  /* dependencies */

  help__fin();
}

/* ----------------------------------------------------------------------- */

#define blendslider_MIN     -65536
#define blendslider_MAX      131071
#define blendslider_DEFAULT  65535

static void main_slider_update(wimp_i i, int val)
{
  NOT_USED(i);

  LOCALS.blendval = val;
  apply_blend(LOCALS.blendval);
}

static void main_update_dialogue(void)
{
  new_selection();
  new_effects();

  LOCALS.blendval = blendslider_DEFAULT;

  slider_set(GLOBALS.effects_w, EFFECTS_S_BLEND_PIT,
             LOCALS.blendval, blendslider_MIN, blendslider_MAX);

  apply_blend(LOCALS.blendval);
}

static int main_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case EFFECTS_B_APPLY:
      effects__apply();
      break;

    case EFFECTS_B_CANCEL:
      effects__cancel();
      break;

    case EFFECTS_B_ADD:
    {
      wimp_window_state state;
      int               x,y;

      state.w = scroll_list__get_window_handle(sl);
      wimp_get_window_state(&state);

      x = state.visible.x0;
      y = state.visible.y0 - 2;

      // FIXME: Want to open the palette win so it avoids covering the pane.

      window_open_as_menu_here(GLOBALS.effects_add_w, x, y);
    }
      break;

    case EFFECTS_B_EDIT:
      edit_effect(scroll_list__get_selection(sl));
      break;

    case EFFECTS_B_DELETE:
      remove_effect(scroll_list__get_selection(sl));
      break;

    case EFFECTS_S_BLEND_FOREGROUND:
    case EFFECTS_S_BLEND_BACKGROUND:
      slider_start(pointer, main_slider_update,
                   blendslider_MIN, blendslider_MAX);
      break;
    }
  }

  if (pointer->i == EFFECTS_B_APPLY || pointer->i == EFFECTS_B_CANCEL)
  {
    if (pointer->buttons & wimp_CLICK_SELECT)
      effects__close();
    else
      main_update_dialogue();
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static void draw_element(effect_element *e, int x, int y, int highlight)
{
  wimp_icon   icon;
  const char *name;

  /* sprite */

  icon.extent.x0 = x;
  icon.extent.y0 = y;
  icon.extent.x1 = x + ICONWIDTH;
  icon.extent.y1 = y + HEIGHT;

  icon.flags = wimp_ICON_SPRITE |
               wimp_ICON_HCENTRED |
               wimp_ICON_VCENTRED |
               wimp_ICON_FILLED |
               wimp_ICON_INDIRECTED |
               (wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT) |
               (wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT);

  if (highlight)
    icon.flags |= wimp_ICON_SELECTED;

  name = effect_to_sprite(e->effect);

  icon.data.indirected_sprite.id   = (osspriteop_id) name;
  icon.data.indirected_sprite.area = window_get_sprite_area();
  icon.data.indirected_sprite.size = strlen(name);

  _swi(Wimp_PlotIcon, _INR(1,5), &icon, 0, 0, 0, 0);

  /* text */

  icon.extent.x0  = icon.extent.x1;
  icon.extent.x1 += TEXTWIDTH;

  icon.flags = wimp_ICON_TEXT |
               wimp_ICON_VCENTRED |
               wimp_ICON_FILLED |
               wimp_ICON_INDIRECTED |
               (wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT) |
               (wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT);

  if (highlight)
    icon.flags |= wimp_ICON_SELECTED;

  name = message0(effect_to_name(e->effect));

  icon.data.indirected_text_and_sprite.text       = (char *) name;
  icon.data.indirected_text_and_sprite.validation = "";
  icon.data.indirected_text_and_sprite.size       = strlen(name);

  _swi(Wimp_PlotIcon, _INR(1,5), &icon, 0, 0, 0, 0);
}

static void draw_marker(int x, int y)
{
  os_colour_number c;

  c = colourtrans_return_colour_number(os_COLOUR_MID_LIGHT_GREY);
  os_set_colour(0, c);

  os_plot(os_MOVE_TO, x, y);
  // the dimensions should ideally get passed in
  os_plot(os_PLOT_BY | os_PLOT_RECTANGLE, 512 - 1, 4 - 1); /* exclusive */
}

/* ----------------------------------------------------------------------- */

static event_wimp_handler drageffect_event_null_reason_code,
                          drageffect_event_user_drag_box;

static struct
{
  wimp_mouse_state buttons;
  int              effect;
  int              moving;
}
drageffect_state;

static void drageffect_set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_NULL_REASON_CODE, drageffect_event_null_reason_code },
    { wimp_USER_DRAG_BOX,    drageffect_event_user_drag_box    },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            NULL);

  scroll_list__autoscroll(sl, reg);
}

static void drageffect_renderer(void *args)
{
  effect_element e;

  e.effect = (effect_effect) args;

  draw_element(&e, 0, 0, 0);
}

// this looks like it belongs in scroll-list.c
static int drageffect_event_null_reason_code(wimp_event_no event_no,
                                             wimp_block   *block,
                                             void         *handle)
{
  static int   lastx = -1, lasty = -1;
  int          index;
  wimp_pointer pointer;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  wimp_get_pointer_info(&pointer);

  if (pointer.w != scroll_list__get_window_handle(sl))
    return event_HANDLED;

  if (pointer.pos.x == lastx &&
      pointer.pos.y == lasty)
    return event_HANDLED;

  index = scroll_list__where_to_insert(sl, &pointer);

  /* If you try to move an effect next to itself, that has no effect, so
   * if we're moving (not adding) then don't draw the marker if we're either
   * side of the effect's current position. */

  if (drageffect_state.moving)
  {
    if (index == drageffect_state.effect ||
        index == drageffect_state.effect + 1)
    {
      scroll_list__clear_marker(sl);
    }
    else
    {
      scroll_list__set_marker(sl, index);
    }
  }
  else
  {
    scroll_list__set_marker(sl, index);
  }

  lastx = pointer.pos.x;
  lasty = pointer.pos.y;

  return event_HANDLED;
}

static int drageffect_event_user_drag_box(wimp_event_no event_no,
                                          wimp_block   *block,
                                          void         *handle)
{
  wimp_pointer pointer;
  int          index;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  drag_object_stop();

  drageffect_set_handlers(0);

  scroll_list__clear_marker(sl);

  wimp_get_pointer_info(&pointer);

  if (pointer.w != scroll_list__get_window_handle(sl))
    return event_HANDLED;

  /* We need to work this out from scratch in case the drag was too quick
   * to actually get a null event or never went near the pane window. */

  /* convert work area to insertion point */
  index = scroll_list__where_to_insert(sl, &pointer);

  if (drageffect_state.moving)
  {
    if (index == drageffect_state.effect ||
        index == drageffect_state.effect + 1)
    {
      /* dropped either side of the present index -- no effect */
    }
    else
    {
      if (index > drageffect_state.effect)
        index--;

      move_effect(drageffect_state.effect, index);
    }
  }
  else
  {
    insert_effect(drageffect_state.effect, index);
    new_effects();
  }

  // FIXME this is invalid now (drag may start anywhere, not just a pop-up panel)
  if (drageffect_state.buttons & wimp_CLICK_SELECT) /* buttons from start of drag */
    wimp_create_menu(wimp_CLOSE_MENU, 0, 0);

  return event_HANDLED;
}

// called to drag virtual icons, those without actual icons
static void drageffect_box(wimp_pointer *pointer, int effect, int moving, const os_box *box)
{
  drageffect_state.buttons = pointer->buttons;
  drageffect_state.effect  = effect;
  drageffect_state.moving  = moving;

  drageffect_set_handlers(1);

  if (moving)
  {
    // turn this index into an effect number
    effect = effects.entries[effect].effect; // holy crap
  }

  drag_object_box(pointer->w, box, pointer->pos.x, pointer->pos.y,
                  drageffect_renderer, (void *) effect);
}

// if not moving: effect is the number? of the new effect
// if moving: effect is the index of the existing effect
static void drageffect_icon(wimp_pointer *pointer, int effect, int moving)
{
  drageffect_state.buttons = pointer->buttons;
  drageffect_state.effect  = effect;
  drageffect_state.moving  = moving;

  drageffect_set_handlers(1);

  // needs if (moving) code from above; but it's not used atm

  drag_object(pointer->w, pointer->i, pointer->pos.x, pointer->pos.y,
              drageffect_renderer, (void *) effect);
}

/* ----------------------------------------------------------------------- */

/* scroll list stuff */

static void redraw_element(wimp_draw *redraw, int wax, int way, int i, int sel)
{
  effect_element *e;

  NOT_USED(redraw);

  e = effects.entries + i;

  draw_element(e, wax, way, sel);
}

static void redraw_leader(wimp_draw *redraw, int wax, int way, int i, int sel)
{
  int x,y;

  NOT_USED(i);

  if (!sel)
    return;

  x = (redraw->box.x0 - redraw->xscroll) + wax;
  y = (redraw->box.y1 - redraw->yscroll) + way;

  draw_marker(x, y);
}

static void scroll_list_event(scroll_list__event *event)
{
  switch (event->type)
  {
  case scroll_list__SELECTION_CHANGED:
    new_selection();
    break;

  case scroll_list__DRAG:
    drageffect_box(event->data.drag.pointer,
                   event->index,
                   1,
                  &event->data.drag.box);
    break;

  case scroll_list__DELETE:
    remove_effect(event->index);
    break;
  }
}

/* ----------------------------------------------------------------------- */

static int add_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case EFFECTS_ADD_B_CANCEL:
      wimp_create_menu(wimp_CLOSE_MENU, 0, 0);
      break;

    default:
      drageffect_icon(pointer, pointer->i - 3, 0);
      break;
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static void clear_set_handlers(int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_COLOUR_PICKER_COLOUR_CHOICE, clear_message_colour_picker_colour_choice },
  };

  event_register_message_group(reg,
                               message_handlers, NELEMS(message_handlers),
                               event_ANY_WINDOW, event_ANY_ICON,
                               NULL);
}

static int clear_edit(effect_element *e, int x, int y)
{
  colourpicker_dialogue dialogue;
  wimp_w                picker_w;

  dialogue.flags      = 0;
  dialogue.title      = (char *) message0("effect.clear");
  dialogue.visible.x0 = x;
  dialogue.visible.y0 = INT_MIN;
  dialogue.visible.x1 = INT_MAX;
  dialogue.visible.y1 = y;
  dialogue.xscroll    = 0;
  dialogue.yscroll    = 0;
  dialogue.colour     = e->args.clear.colour;
  dialogue.size       = 0;

  colourpicker_open_dialogue(colourpicker_OPEN_TRANSIENT,
                            &dialogue,
                            &picker_w);

  clear_set_handlers(1);

  return 0;
}

static int clear_message_colour_picker_colour_choice(wimp_message *message, void *handle)
{
  colourpicker_message_colour_choice *choice;

  NOT_USED(handle);

  choice = (colourpicker_message_colour_choice *) &message->data;

  editing_element->args.clear.colour = choice->colour;

  effect_edited(editing_element);

  /* I don't think this is going to unregister in the case when 'Cancel' is
   * clicked. */
  clear_set_handlers(0);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  wimp_i icon;
  int    min, max;
  int    defaultval;
  int   *pval;
}
slider_rec;

/* ----------------------------------------------------------------------- */

static effect_tone tone; /* transient effect_tone for editing */

static void tone_set_values_and_redraw(void)
{
  tonemap_set(tone.map,
              tone.channels,
             &tone.spec);

  tonemapgadget_update(tone.map, GLOBALS.effects_crv_w, EFFECTS_CRV_D_CURVE);
}

static const slider_rec tone_sliders[] =
{
  { EFFECTS_CRV_S_GAMMA_PIT,  1, 300, 100, &tone.spec.gamma      },
  { EFFECTS_CRV_S_BRIGHT_PIT, 0, 200, 100, &tone.spec.brightness },
  { EFFECTS_CRV_S_CONT_PIT,   0, 200, 100, &tone.spec.contrast   },
  { EFFECTS_CRV_S_SASG_PIT,   0, 100,  50, &tone.spec.midd       },
  { EFFECTS_CRV_S_BIAS_PIT,   0, 100,  50, &tone.spec.bias       },
  { EFFECTS_CRV_S_GAIN_PIT,   0, 100,  50, &tone.spec.gain       },
};

static void tone_slider_update(wimp_i i, int val)
{
  int  j;
  int  min,max;
  int *pval;

  j = bsearch_int(&tone_sliders[0].icon,
                   NELEMS(tone_sliders),
                   sizeof(tone_sliders[0]),
                   i);
  if (j == -1)
    return;

  min  = tone_sliders[j].min;
  max  = tone_sliders[j].max;
  pval = tone_sliders[j].pval;

  val = CLAMP(val, min, max);

  *pval = val;

  tone_set_values_and_redraw();
}

static void tone_reset_dialogue(void)
{
  static const struct
  {
    tonemap_channels channels;
    wimp_i           i;
  }
  map[] =
  {
    { tonemap_CHANNEL_RGB,   EFFECTS_CRV_R_ALL   },
    { tonemap_CHANNEL_RED,   EFFECTS_CRV_R_RED   },
    { tonemap_CHANNEL_GREEN, EFFECTS_CRV_R_GREEN },
    { tonemap_CHANNEL_BLUE,  EFFECTS_CRV_R_BLUE  },
    { tonemap_CHANNEL_ALPHA, EFFECTS_CRV_R_ALPHA },
  };

  const slider_rec *r;
  int               i;

  /* reset tone.* to values from tonemap */

  tonemap_get_values(tone.map,
                     tone.channels,
                    &tone.spec);

  /* set radio buttons */

  for (i = 0; i < NELEMS(map); i++)
    if (map[i].channels == tone.channels)
      break;

  icon_set_radio(GLOBALS.effects_crv_w, map[i].i);

  /* disable 'alpha' if required */

  icon_set_flags(GLOBALS.effects_crv_w, EFFECTS_CRV_R_ALPHA,
                 (LOCALS.image->flags & image_FLAG_HAS_ALPHA) ? 0 : wimp_ICON_SHADED, wimp_ICON_SHADED);

  /* set slider values */

  r = &tone_sliders[0];
  for (i = 0; i < NELEMS(tone_sliders); i++)
  {
    slider_set(GLOBALS.effects_crv_w, r->icon, *r->pval, r->min, r->max);
    r++;
  }

  /* set option buttons */

  icon_set_selected(GLOBALS.effects_crv_w,
                    EFFECTS_CRV_O_INVERT,
                    tone.spec.flags & tonemap_FLAG_INVERT);

  icon_set_selected(GLOBALS.effects_crv_w,
                    EFFECTS_CRV_O_REFLECT,
                    tone.spec.flags & tonemap_FLAG_REFLECT);
}

static void tone_start_editing(void)
{
  tonemap_destroy(tone.map);

  /* copy everything across */

  tone = editing_element->args.tone;

  /* clone the tonemap so we have a transient one to play with */

  tone.map = tonemap_copy(tone.map);

  tone_reset_dialogue();

  /* recalculate tonemap, refresh gadget */

  tone_set_values_and_redraw();
}

static int tone_edit(effect_element *e, int x, int y)
{
  NOT_USED(e);

  tone_start_editing();

  window_open_as_menu_here(GLOBALS.effects_crv_w, x, y);

  return 0;
}

static int tone_event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_draw *redraw;

  redraw = &block->redraw;

  NOT_USED(event_no);
  NOT_USED(handle);

  tonemapgadget_redraw(tone.map,
                       GLOBALS.effects_crv_w, EFFECTS_CRV_D_CURVE,
                       redraw);

  return event_HANDLED;
}

static int tone_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    int kick;

    kick = 0;

    switch (pointer->i)
    {
    case EFFECTS_CRV_B_APPLY:
      {
        /* throw away the previous tonemap */

        tonemap_destroy(editing_element->args.tone.map);

        /* copy everything across */

        editing_element->args.tone = tone;

        /* clone a new tonemap, because we need to keep ours in case we're
         * _not_ closing the dialogue */
        editing_element->args.tone.map = tonemap_copy(tone.map);

        effect_edited(editing_element);
      }
      break;

    case EFFECTS_CRV_B_CANCEL:
      break;

    case EFFECTS_CRV_O_INVERT:
      tone.spec.flags ^= tonemap_FLAG_INVERT;
      kick = 1;
      break;

    case EFFECTS_CRV_O_REFLECT:
      tone.spec.flags ^= tonemap_FLAG_REFLECT;
      kick = 1;
      break;

    case EFFECTS_CRV_R_ALL:
    case EFFECTS_CRV_R_RED:
    case EFFECTS_CRV_R_GREEN:
    case EFFECTS_CRV_R_BLUE:
    case EFFECTS_CRV_R_ALPHA:
    {
      tonemap_channels new_channels;

      new_channels = tone.channels;

      switch (pointer->i)
      {
      case EFFECTS_CRV_R_ALL:
        new_channels = tonemap_CHANNEL_RGB;
        break;
      case EFFECTS_CRV_R_RED:
        new_channels = tonemap_CHANNEL_RED;
        break;
      case EFFECTS_CRV_R_GREEN:
        new_channels = tonemap_CHANNEL_GREEN;
        break;
      case EFFECTS_CRV_R_BLUE:
        new_channels = tonemap_CHANNEL_BLUE;
        break;
      case EFFECTS_CRV_R_ALPHA:
        new_channels = tonemap_CHANNEL_ALPHA;
        break;
      }

      if (new_channels != tone.channels)
      {
        /* retrieve values for newly-selected channel from tonemap and store
         * in 'tone' */

        tone.channels = new_channels;

        tonemap_get_values(tone.map,
                           tone.channels,
                          &tone.spec);

        tone_reset_dialogue();
        kick = 1;
      }

      if (pointer->buttons == wimp_CLICK_ADJUST)
        icon_set_radio(pointer->w, pointer->i);
    }
      break;

    case EFFECTS_CRV_B_RESET:
      effect_tone_reset(&tone);
      tone_reset_dialogue();
      kick = 1;
      break;

    default:
    {
      const slider_rec *end;
      const slider_rec *r;

      end = &tone_sliders[NELEMS(tone_sliders)];

      for (r = tone_sliders; r < end; r++)
      {
        /* allow either grey or white icon, but not the 'pit' */
        if (r->icon + 1 != pointer->i && r->icon + 2 != pointer->i)
          continue;

        slider_start(pointer, tone_slider_update, r->min, r->max);
      }
    }
      break;
    }

    if (kick)
      tone_set_values_and_redraw();

    if (pointer->i == EFFECTS_CRV_B_APPLY ||
        pointer->i == EFFECTS_CRV_B_CANCEL)
    {
      if (pointer->buttons & wimp_CLICK_SELECT)
        // need to throw away any remaining tonemap from tone.map
        wimp_create_menu(wimp_CLOSE_MENU, 0, 0);
      else
        tone_start_editing();
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static effect_blur blur; /* transient effect_blur for editing */

static const slider_rec blur_sliders[] =
{
  { EFFECTS_BLR_S_EFFECT_PIT, 2, 24, 2, &blur.level },
};

//static void blur_set_values_and_redraw(void)
//{
//}

// callback from slider code
static void blur_slider_update(wimp_i i, int val)
{
  const slider_rec *r;
  int               min,max;
  int              *pval;

  NOT_USED(i);

  r = &blur_sliders[0];

  min  = r->min;
  max  = r->max;
  pval = r->pval;

  val = CLAMP(val, min, max);

  *pval = val;

  icon_set_int(dialogue__get_window(&GLOBALS.effects_blr_d), EFFECTS_BLR_W_VAL, val);

  //blur_set_values_and_redraw();
}

static void blur_reset_dialogue(void)
{
  static const struct
  {
    effects_blur_method method;
    wimp_i              i;
  }
  map[] =
  {
    { effects_blur_BOX,      EFFECTS_BLR_R_BOX  },
    { effects_blur_GAUSSIAN, EFFECTS_BLR_R_BELL },
  };

  const slider_rec *r;
  int               i;

  /* set radio buttons */

  for (i = 0; i < NELEMS(map); i++)
    if (map[i].method == blur.method)
      break;

  icon_set_radio(dialogue__get_window(&GLOBALS.effects_blr_d), map[i].i);

  /* set slider values */

  r = &blur_sliders[0];
  slider_set(dialogue__get_window(&GLOBALS.effects_blr_d), r->icon, *r->pval, r->min, r->max);

  icon_set_int(dialogue__get_window(&GLOBALS.effects_blr_d), EFFECTS_BLR_W_VAL, *r->pval);
}

static void blur_start_editing(void)
{
  /* copy everything across */

  blur = editing_element->args.blur;

  blur_reset_dialogue();

  //blur_set_values_and_redraw();
}

static int blur_edit(effect_element *e, int x, int y)
{
  NOT_USED(e);

  blur_start_editing();

  dialogue__show_here(&GLOBALS.effects_blr_d, x, y);

  return 0;
}

static int blur_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;
  int           val;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    /* allow either grey or white icon, but not the 'pit' */
    if (EFFECTS_BLR_S_EFFECT_BACKGROUND == pointer->i ||
        EFFECTS_BLR_S_EFFECT_FOREGROUND == pointer->i)
    {
      slider_start(pointer, blur_slider_update, 2, 24);
    }
    else
    {
      switch (pointer->i)
      {
      case EFFECTS_BLR_B_APPLY:
        val = icon_get_int(dialogue__get_window(&GLOBALS.effects_blr_d), EFFECTS_BLR_W_VAL);
        blur.level = CLAMP(val, 2, 24);

        editing_element->args.blur = blur;

        effect_edited(editing_element);
        break;

      case EFFECTS_BLR_B_CANCEL:
        break;

      case EFFECTS_BLR_R_BOX:
      case EFFECTS_BLR_R_BELL:
        switch (pointer->i)
        {
        case EFFECTS_BLR_R_BOX:
          blur.method = effects_blur_BOX;
          break;
        case EFFECTS_BLR_R_BELL:
          blur.method = effects_blur_GAUSSIAN;
          break;
        }

        if (pointer->buttons == wimp_CLICK_ADJUST)
          icon_set_radio(pointer->w, pointer->i);
        break;
      }
    }

//    if (kick)
//      blur_set_values_and_redraw();

    if (pointer->i == EFFECTS_BLR_B_APPLY ||
        pointer->i == EFFECTS_BLR_B_CANCEL)
    {
      if (pointer->buttons & wimp_CLICK_SELECT)
        wimp_create_menu(wimp_CLOSE_MENU, 0, 0);
      else
        blur_start_editing();
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static error create_images(void)
{
  image             *image;
  osspriteop_area   *area;
  int                areasize;
  osspriteop_header *header1;
  osspriteop_header *header2;
  osspriteop_header *header3;
  int                r;

  image = LOCALS.image;
  area  = (osspriteop_area *) image->image;

  areasize = area->size;

  /* stretch the image's heap block to three times its existing size and
   * add to it two duplicates of the existing image */

  r = flex_extend((flex_ptr) &image->image, areasize * 3);
  if (r == 0)
    return error_OOM;

  area    = (osspriteop_area *) image->image;
  header1 = sprite_select(area, 0);

  area->size = areasize * 3;

  osspriteop_copy_sprite(osspriteop_PTR,
                         area,
         (osspriteop_id) header1,
                         "degenerate");

  osspriteop_copy_sprite(osspriteop_PTR,
                         area,
         (osspriteop_id) header1,
                         "result");

  header2 = sprite_select(area, 1);
  header3 = sprite_select(area, 2);

  blender_create(&LOCALS.blender, area);

  /* point the blender at the image data */
  LOCALS.blender.src = sprite_data(header1);
  LOCALS.blender.deg = sprite_data(header2);
  LOCALS.blender.dst = sprite_data(header3);

  return error_OK;
}

static int delete_images(void)
{
  osspriteop_area   *area;
  int                areasize;
  osspriteop_header *header;
  int                r;

  area     = (osspriteop_area *) LOCALS.image->image;
  areasize = area->size;

  /* Delete the last two sprites. The first is our result. */

  header = sprite_select(area, 2);
  osspriteop_delete_sprite(osspriteop_PTR, area, (osspriteop_id) header);

  header = sprite_select(area, 1);
  osspriteop_delete_sprite(osspriteop_PTR, area, (osspriteop_id) header);

  r = flex_extend((flex_ptr) &LOCALS.image->image, areasize / 3);
  if (r == 0)
  {
    oserror__report(0, "error.no.mem");
    return 1;
  }

  area->size = flex_size((flex_ptr) &LOCALS.image->image);

  return 0;
}

static void image_changed_callback(image                *image,
                                   imageobserver_change  change,
                                   imageobserver_data   *data)
{
  NOT_USED(image);
  NOT_USED(data);

  if (change == imageobserver_CHANGE_ABOUT_TO_DESTROY)
    effects__close();
}

void effects__open(image *image)
{
  error err;

  if (LOCALS.open && LOCALS.image == image)
  {
    window_open_at(GLOBALS.effects_w, AT_BOTTOMPOINTER);
    return;
  }

  if (!effects__available(image))
  {
    beep();
    return;
  }

  if (LOCALS.open && LOCALS.image != image)
  {
    /* opened for a different image */
    effects__close();
  }

  if (LOCALS.open)
    return; /* already open */

  image_start_editing(image);

  LOCALS.image = image;

  err = create_images();
  if (err)
  {
    error__report(err);
    return;
  }

  image_select(image, 2);

  /* watch for changes */
  imageobserver_register(image, image_changed_callback);

  /* open as a proper window */
  window_open_at(GLOBALS.effects_w, AT_BOTTOMPOINTER);

  LOCALS.open = 1;

  main_update_dialogue();
}

static void effects__close(void)
{
  /* this kicks apply_effects, so is doing more work than necessary */
  remove_all_effects();

  imageobserver_deregister(LOCALS.image, image_changed_callback);

  image_select(LOCALS.image, 0);
  image_preview(LOCALS.image); /* force an update */

  delete_images();

  image_stop_editing(LOCALS.image);

  window_close(GLOBALS.effects_w);

  LOCALS.open  = 0;
  LOCALS.image = NULL;
}

/* Make the effects permanent, overwriting source. */
static void effects__apply(void)
{
  image           *image;
  osspriteop_area *area;

  image = LOCALS.image;
  area  = (osspriteop_area *) image->image;

  image_about_to_modify(LOCALS.image);

  /* copy result to source */

  copy_sprite_data(area, 2, 0);

  image_modified(LOCALS.image, image_MODIFIED_DATA);

  remove_all_effects();
}

static void effects__cancel(void)
{
  remove_all_effects();
}

int effects__available(const image *image)
{
  const osspriteop_area *area;
  osspriteop_header     *header;
  osspriteop_mode_word   mode;
  int                    log2bpp;

  if (image == NULL ||
      image_is_editing(image) ||
      image->display.file_type != osfile_TYPE_SPRITE)
    return 0;

  area   = (const osspriteop_area *) image->image;
  header = sprite_select(area, 0);

  sprite_info(area, header, NULL, NULL, NULL, (os_mode *) &mode, &log2bpp);

  switch (log2bpp)
  {
#if 0 /* not yet fully implemented */
  case 3:
    {
      int size;

      osspriteop_read_palette_size(osspriteop_PTR,
                                   area,
             (const osspriteop_id) header,
                                  &size,
                                   NULL,
                                   NULL);

      // explicit check for greyscale palette

      return size == 256;
    }
#endif

  case 5:
    return 1;

  default:
    return 0;
  }
}