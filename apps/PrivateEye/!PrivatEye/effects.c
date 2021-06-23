/* --------------------------------------------------------------------------
 *    Name: effects.c
 * Purpose: Effects dialogue
 * ----------------------------------------------------------------------- */

// TODO: Use imageobwin?

#include "swis.h"

#include <assert.h>
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

#include "datastruct/list.h"

#include "appengine/types.h"
#include "appengine/base/bsearch.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/gadgets/scroll-list.h"
#include "appengine/gadgets/slider.h"
#include "appengine/gadgets/tonemap-gadget.h"
#include "appengine/graphics/colour.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/graphics/sprite-effects.h"
#include "appengine/graphics/tonemap.h"
#include "appengine/vdu/sprite.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"

#include "globals.h"
#include "iconnames.h"          /* generated */
#include "iconnames2.h"         /* not generated */
#include "menunames.h"

#include "effects.h"

#define HEIGHT    44
#define ICONWIDTH 44
#define TEXTWIDTH 460 /* assuming a 512 pixel wide window */

/* ---------------------------------------------------------------------- */

typedef int effects_type;
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
  effect__LIMIT
};

static const char *effectname(effects_type e)
{
  static const char *names[effect__LIMIT] =
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

static const char *spritename(effects_type e)
{
  static const char *names[effect__LIMIT] =
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
  tonemap         *map;
  tonemap_channels channels;
  tonemap_spec     spec;
}
effect_tone;

typedef union
{
  effect_clear clear;
  effect_blur  blur;
  effect_tone  tone;
}
effects_args;

typedef struct
{
  effects_type effect;
  effects_args args;
}
effects_element;

typedef struct
{
  effects_element *entries;
  int             nentries;
  int             allocated;
}
effects_array;

static void effectarray_init(effects_array *ea)
{
  ea->entries   = NULL;
  ea->nentries  = 0;
  ea->allocated = 0;
}

/* ---------------------------------------------------------------------- */

typedef struct effectswin effectswin_t;

/* ---------------------------------------------------------------------- */

typedef int effect_editor(effectswin_t    *ew,
                          effects_element *el,
                          int              x,
                          int              y);

typedef result_t effect_initialiser(effects_element *el);

typedef result_t effect_applier(osspriteop_area   *area,
                                osspriteop_header *src,
                                osspriteop_header *dst,
                                effects_element   *el);

/* ----------------------------------------------------------------------- */

static void effects_close(effectswin_t *ew);
static void effects_apply(effectswin_t *ew);
static void effects_cancel(effectswin_t *ew);

/* ----------------------------------------------------------------------- */

struct effectswin
{
  list_t           list; /* an effectwin is a linked list node */
  wimp_w           window;
  scroll_list     *sl;
  image_t         *image;
  blender          blender;
  int              blendval;
  effects_array    effects;
  effects_element *editing_element;
};

/* ---------------------------------------------------------------------- */

static result_t clear_apply(osspriteop_area   *area,
                            osspriteop_header *src,
                            osspriteop_header *dst,
                            effects_element   *el)
{
  return effects_clear_apply(area, src, dst, el->args.clear.colour);
}

static result_t clear_defaults(effects_element *el)
{
  el->args.clear.colour = 0x80808000;

  return result_OK;
}

static result_t tone_apply(osspriteop_area   *area,
                           osspriteop_header *src,
                           osspriteop_header *dst,
                           effects_element   *el)
{
  return effects_tonemap_apply(area, src, dst, el->args.tone.map);
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

static result_t tone_defaults(effects_element *el)
{
  effect_tone *t;

  t = &el->args.tone;

  t->map = tonemap_create();
  if (t->map == NULL)
    return result_OOM;

  tonemap_draw_set_stroke_width(t->map, GLOBALS.choices.effects.curve_width);

  effect_tone_reset(t);

  return result_OK;
}

static result_t grey_apply(osspriteop_area   *area,
                           osspriteop_header *src,
                           osspriteop_header *dst,
                           effects_element   *el)
{
  NOT_USED(el);

  return effects_grey_apply(area, src, dst);
}

static result_t blur_defaults(effects_element *el)
{
  el->args.blur.method = effects_blur_GAUSSIAN;
  el->args.blur.level  = 2;

  return result_OK;
}

static result_t blur_apply(osspriteop_area   *area,
                           osspriteop_header *src,
                           osspriteop_header *dst,
                           effects_element   *el)
{
  return effects_blur_apply(area,
                            src,
                            dst,
                            el->args.blur.method,
                            el->args.blur.level);
}

static result_t sharpen_apply(osspriteop_area   *area,
                              osspriteop_header *src,
                              osspriteop_header *dst,
                              effects_element   *el)
{
  NOT_USED(el);

  return effects_sharpen_apply(area, src, dst, 0 /* unused */);
}

static result_t expand_apply(osspriteop_area   *area,
                             osspriteop_header *src,
                             osspriteop_header *dst,
                             effects_element   *el)
{
  NOT_USED(el);

  return effects_expand_apply(area, src, dst, 0 /* threshold */);
}

static result_t equalise_apply(osspriteop_area   *area,
                               osspriteop_header *src,
                               osspriteop_header *dst,
                               effects_element   *el)
{
  NOT_USED(el);

  return effects_equalise_apply(area, src, dst);
}

static result_t emboss_apply(osspriteop_area   *area,
                             osspriteop_header *src,
                             osspriteop_header *dst,
                             effects_element   *el)
{
  NOT_USED(area);
  NOT_USED(src);
  NOT_USED(dst);
  NOT_USED(el);

  return result_OK;
}

/* ---------------------------------------------------------------------- */

/* declare editors */
static effect_editor clear_edit, blur_edit, tone_edit;

static const struct
{
  effect_editor      *editor;
  effect_initialiser *initialiser;
  effect_applier     *apply;
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

// FIXME: Hoist this out to a utility library.

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

static void apply_blend(effectswin_t *ew, int val)
{
  ew->blender.make_lut(val, 0, &ew->blender);
  ew->blender.blend(&ew->blender);

  image_preview(ew->image);
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

static result_t apply_effects(effectswin_t *ew)
{
  result_t         err;
  effects_array    *v;
  image_t         *image;
  osspriteop_area *area;

  err = result_OK;

  hourglass_on();

  image = ew->image;
  area  = (osspriteop_area *) image->image;

  v = &ew->effects;

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
      effects_element *e;

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
    /* There are no effects. Without anything to apply the degenerate image
     * won't get filled in, so just do a straight copy. */

    /* FIXME: This is all wasted work which could be avoided by not creating
     *        the preview until an effect is applied. */

    copy_sprite_data(area, 0, 1);
  }

  /* kick the blender */

  apply_blend(ew, ew->blendval);

  /* FALLTHROUGH */

Exit:

  hourglass_off();

  return err;
}

static int insert_effect(effectswin_t *ew, effects_type effect, int where)
{
  effects_array   *v;
  effects_element *e;
  size_t          shiftamt;

  v = &ew->effects;

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
  if (editors[e->effect].initialiser)
    editors[e->effect].initialiser(e);

  v->nentries++;

  scroll_list_add_row(ew->sl);

  apply_effects(ew);

  return 0; /* success */
}

static int move_effect(effectswin_t *ew, int from, int to)
{
  effects_array   *v;
  effects_element  t; /* temporary element */
  int             down;
  void           *dst;
  void           *src;
  int             n;
  int             i;

  if (to == from)
    return 0; /* nothing to do */

  v = &ew->effects;

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
      scroll_list_refresh_row(ew->sl, i);
  }

  apply_effects(ew);

  return 0; /* success */
}

static int is_editable(effects_element *e)
{
  return editors[e->effect].editor != NULL;
}

/* Shade the "Edit..." and "Delete" buttons. */
static void new_selection(effectswin_t *ew)
{
  int             sel;
  wimp_icon_flags edit;
  wimp_icon_flags del;

  edit = wimp_ICON_SHADED;
  del  = wimp_ICON_SHADED;

  sel = scroll_list_get_selection(ew->sl);

  if (sel >= 0)
  {
    effects_array   *v;
    effects_element *e;

    v = &ew->effects;
    e = &v->entries[sel];

    if (is_editable(e))
      edit = 0;

    del = 0;
  }

  icon_set_flags(ew->window, EFFECTS_B_EDIT,  edit, wimp_ICON_SHADED);
  icon_set_flags(ew->window, EFFECTS_B_DELETE, del, wimp_ICON_SHADED);
}

static void new_effects(effectswin_t *ew)
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

  eor   = (ew->effects.nentries > 0) ? 0 : wimp_ICON_SHADED;
  clear = wimp_ICON_SHADED;

  icon_group_set_flags(ew->window, icons, NELEMS(icons), eor, clear);
}

static void edit_effect(effectswin_t *ew, int index)
{
  effects_array   *v;
  effects_element *e;
  wimp_pointer     pointer;
  effect_editor   *editor;

  v = &ew->effects;

  e = &v->entries[index];

  wimp_get_pointer_info(&pointer);

  ew->editing_element = e;

  editor = editors[e->effect].editor;
  if (editor)
    editor(ew, e, pointer.pos.x, pointer.pos.y);
}

static void effect_edited(effectswin_t *ew, effects_element *e)
{
  effects_array *v;
  int           i;

  v = &ew->effects;

  i = e - v->entries;

  scroll_list_refresh_row(ew->sl, i);

  apply_effects(ew);
}

static void remove_effect(effectswin_t *ew, int index)
{
  effects_array *v;
  size_t        s;

  if (index == -1)
    return;

  v = &ew->effects;

  s = (v->nentries - index - 1) * sizeof(v->entries[0]);
  if (s)
    memmove(v->entries + index, v->entries + index + 1, s);

  v->nentries--;

  scroll_list_delete_rows(ew->sl, index, index);

  new_selection(ew);
  new_effects(ew);

  apply_effects(ew); /* kick the effects + blender */
}

static void remove_all_effects(effectswin_t *ew)
{
  effects_array *v;

  v = &ew->effects;

  /* Retain the block, but empty it. */

  v->nentries = 0;

  scroll_list_delete_rows(ew->sl, 0, INT_MAX); /* all */

  // call new_selection()?

  apply_effects(ew); /* kick the effects + blender */
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler main_event_mouse_click,

                          add_event_mouse_click,

                          blur_event_mouse_click,

                          tone_event_redraw_window_request,
                          tone_event_mouse_click;

static event_message_handler clear_message_colour_picker_colour_choice;

/* ----------------------------------------------------------------------- */

// FIXME: Introduce a typedef for this form of function? e.g. scroll_list_drawfn
static void scroll_list_draw_row(wimp_draw *redraw, int wax, int way, int i, int sel, void *opaque);
static void scroll_list_draw_leader(wimp_draw *redraw, int wax, int way, int i, int sel, void *opaque);
static void scroll_list_event_callback(scroll_list_event *event, void *opaque);

/* ----------------------------------------------------------------------- */

static result_t register_main(effectswin_t *ew)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK, main_event_mouse_click },
  };

  result_t err;

  event_register_wimp_group(1,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            ew->window,
                            event_ANY_ICON,
                            ew);

  ew->sl = scroll_list_create(ew->window, EFFECTS_I_PANE_PLACEHOLDER);
  if (ew->sl == NULL)
    return result_OOM; // FIXME potentially inaccurate

  scroll_list_set_row_height(ew->sl, HEIGHT, 4);
  scroll_list_set_handlers(ew->sl,
                           scroll_list_draw_row,
                           scroll_list_draw_leader,
                           scroll_list_event_callback,
                           ew);

  err = help_add_window(scroll_list_get_window_handle(ew->sl), "effects_list");
  if (err)
    return err;

  return result_OK;
}

static void deregister_main(effectswin_t *ew)
{
    // todo: deregister events (elsewhere too)

  help_remove_window(scroll_list_get_window_handle(ew->sl));

  scroll_list_destroy(ew->sl);
}

static result_t register_add(effectswin_t *ew)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK, add_event_mouse_click },
  };

  event_register_wimp_group(1,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            GLOBALS.effects_add_w,
                            event_ANY_ICON,
                            ew);
  return result_OK;
}

static void deregister_add(effectswin_t *ew)
{
}

// blur would go here

static result_t register_tone(effectswin_t *ew)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, tone_event_redraw_window_request },
    { wimp_MOUSE_CLICK,           tone_event_mouse_click           },
  };

  event_register_wimp_group(1,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            GLOBALS.effects_crv_w,
                            event_ANY_ICON,
                            ew);

  return result_OK;
}

static void deregister_tone(effectswin_t *ew)
{
}

/* ----------------------------------------------------------------------- */

static result_t init_main(void)
{
  result_t err;

  GLOBALS.effects_w = window_create("effects");

  // this won't work will it? it's attaching help to the base window
  err = help_add_window(GLOBALS.effects_w, "effects");
  if (err)
    return err;

  return result_OK;
}

static void fin_main(void)
{
  help_remove_window(GLOBALS.effects_w);
}

static result_t init_add(void)
{
  result_t err;

  GLOBALS.effects_add_w = window_create("effects_add");

  err = help_add_window(GLOBALS.effects_add_w, "effects_add");
  if (err)
    return err;

  return result_OK;
}

static void fin_add(void)
{
  help_remove_window(GLOBALS.effects_add_w);
}

static result_t init_blur(void)
{
  result_t err;

  err = dialogue_construct(&GLOBALS.effects_blr_d,
                            "effects_blr",
                            EFFECTS_BLR_B_APPLY,
                            EFFECTS_BLR_B_CANCEL);
  if (err)
    return err;

  dialogue_set_mouse_click_handler(&GLOBALS.effects_blr_d,
                                    blur_event_mouse_click);
  // DPT THIS IS BROKEN .. what gets set for the opaque handle by the above?

  return result_OK;
}

static void fin_blur(void)
{
  dialogue_destruct(&GLOBALS.effects_blr_d);
}

static result_t init_tone(void)
{
  result_t err;

  GLOBALS.effects_crv_w = window_create("effects_crv");

  err = help_add_window(GLOBALS.effects_crv_w, "effects_crv");
  if (err)
    return err;

  return result_OK;
}

static void fin_tone(void)
{
  help_remove_window(GLOBALS.effects_crv_w);
}

/* ----------------------------------------------------------------------- */

// todo
// divide resources into persistent (e.g. effects window) and transient (dialogues)
// any persistent resources need to be allocated per-image
// any transient resources need to be allocated once, and registered when used (and deregistered afterwards)

result_t effects_init(void)
{
  result_t err;

  /* dependencies */

  err = help_init();
  if (err)
    return err;

  /* modules */

  err = init_main(); // TODO handle
  err = init_add();
  err = init_blur();
  err = init_tone();

  return result_OK;
}

void effects_fin(void)
{
  /* modules */

  fin_tone();
  fin_blur();
  fin_add();
  fin_main();

  /* dependencies */

  help_fin();
}

/* ----------------------------------------------------------------------- */

#define blendslider_MIN    -65536
#define blendslider_MAX     131071
#define blendslider_DEFAULT 65535

static void main_slider_update(wimp_i i, int val, void *opaque)
{
  effectswin_t *ew = opaque;

  NOT_USED(i);

  ew->blendval = val;
  apply_blend(ew, ew->blendval);
}

static void reset_main_dialogue(effectswin_t *ew)
{
  int blendval;

  new_selection(ew);
  new_effects(ew);

  blendval = ew->blendval = blendslider_DEFAULT;

  slider_set(ew->window,
             EFFECTS_S_BLEND_PIT,
             blendval,
             blendslider_MIN,
             blendslider_MAX);

  apply_blend(ew, blendval);
}

static int main_event_mouse_click(wimp_event_no event_no,
                                  wimp_block   *block,
                                  void         *handle)
{
  wimp_pointer *pointer;
  effectswin_t  *ew = handle;

  NOT_USED(event_no);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case EFFECTS_B_APPLY:
      effects_apply(ew);
      break;

    case EFFECTS_B_CANCEL:
      effects_cancel(ew);
      break;

    case EFFECTS_B_ADD:
      {
        wimp_window_state state;
        int               x,y;

        state.w = scroll_list_get_window_handle(ew->sl);
        wimp_get_window_state(&state);

        x = state.visible.x0;
        y = state.visible.y0 - 2;

        // FIXME: Want to open the palette win so it avoids covering the
        // pane.

        window_open_as_menu_here(GLOBALS.effects_add_w, x, y);
      }
      break;

    case EFFECTS_B_EDIT:
      edit_effect(ew, scroll_list_get_selection(ew->sl));
      break;

    case EFFECTS_B_DELETE:
      remove_effect(ew, scroll_list_get_selection(ew->sl));
      break;

    case EFFECTS_S_BLEND_FOREGROUND:
    case EFFECTS_S_BLEND_BACKGROUND:
      slider_start(pointer,
                   blendslider_MIN,
                   blendslider_MAX,
                   main_slider_update,
                   ew);
      break;
    }
  }

  if (pointer->i == EFFECTS_B_APPLY || pointer->i == EFFECTS_B_CANCEL)
  {
    if (pointer->buttons & wimp_CLICK_SELECT)
      effects_close(ew);
    else
      reset_main_dialogue(ew);
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static void draw_element(effects_element *e, int x, int y, int highlight)
{
  wimp_icon   icon;
  const char *name;

  /* sprite */

  icon.extent.x0 = x;
  icon.extent.y0 = y;
  icon.extent.x1 = x + ICONWIDTH;
  icon.extent.y1 = y + HEIGHT;

  icon.flags = wimp_ICON_SPRITE     |
               wimp_ICON_HCENTRED   |
               wimp_ICON_VCENTRED   |
               wimp_ICON_FILLED     |
               wimp_ICON_INDIRECTED |
               (wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT) |
               (wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT);

  if (highlight)
    icon.flags |= wimp_ICON_SELECTED;

  name = spritename(e->effect);
  icon.data.indirected_sprite.id   = (osspriteop_id) name;
  icon.data.indirected_sprite.area = window_get_sprite_area();
  icon.data.indirected_sprite.size = strlen(name);

  // FIXME: Not using OSLib here because...
  _swi(Wimp_PlotIcon, _INR(1,5), &icon, 0, 0, 0, 0);

  /* text */

  icon.extent.x0 = icon.extent.x1;
  icon.extent.x1 += TEXTWIDTH;

  icon.flags = wimp_ICON_TEXT       |
               wimp_ICON_VCENTRED   |
               wimp_ICON_FILLED     |
               wimp_ICON_INDIRECTED |
               (wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT) |
               (wimp_COLOUR_WHITE << wimp_ICON_BG_COLOUR_SHIFT);

  if (highlight)
    icon.flags |= wimp_ICON_SELECTED;

  name = message0(effectname(e->effect));
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

static void drageffect_set_handlers(effectswin_t *ew, int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_NULL_REASON_CODE, drageffect_event_null_reason_code },
    { wimp_USER_DRAG_BOX,    drageffect_event_user_drag_box    },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            event_ANY_WINDOW,
                            event_ANY_ICON,
                            ew);

  scroll_list_autoscroll(ew->sl, reg);
}

static void drageffect_renderer(void *opaque)
{
  effects_element e;

  e.effect = (effects_type) opaque;

  draw_element(&e, 0, 0, 0);
}

// this looks like it belongs in scroll-list.c
static int drageffect_event_null_reason_code(wimp_event_no event_no,
                                             wimp_block   *block,
                                             void         *handle)
{
  static int   lastx = -1, lasty = -1;
  wimp_pointer pointer;
  effectswin_t *ew = handle;
  int          index;

  NOT_USED(event_no);
  NOT_USED(block);

  wimp_get_pointer_info(&pointer);

  if (pointer.w != scroll_list_get_window_handle(ew->sl))
    return event_HANDLED;

  if (pointer.pos.x == lastx && pointer.pos.y == lasty)
    return event_HANDLED;

  index = scroll_list_where_to_insert(ew->sl, &pointer);

  /* If you try to move an effect next to itself, that has no effect, so
   * if we're moving (not adding) then don't draw the marker if we're either
   * side of the effect's current position. */

  if (drageffect_state.moving)
  {
    if (index == drageffect_state.effect ||
        index == drageffect_state.effect + 1)
      scroll_list_clear_marker(ew->sl);
    else
      scroll_list_set_marker(ew->sl, index);
  }
  else
  {
    scroll_list_set_marker(ew->sl, index);
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
  effectswin_t *ew = handle;

  NOT_USED(event_no);
  NOT_USED(block);

  drag_object_stop();

  drageffect_set_handlers(ew, 0);

  scroll_list_clear_marker(ew->sl);

  wimp_get_pointer_info(&pointer);

  if (pointer.w != scroll_list_get_window_handle(ew->sl))
    return event_HANDLED;

  /* We need to work this out from scratch in case the drag was too quick
   * to actually get a null event or never went near the pane window. */

  /* convert work area to insertion point */
  index = scroll_list_where_to_insert(ew->sl, &pointer);

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

      move_effect(ew, drageffect_state.effect, index);
    }
  }
  else
  {
    assert(index < 10);
    insert_effect(ew, drageffect_state.effect, index);
    new_effects(ew);
  }

  // FIXME This is invalid now (drag may start anywhere, not just a pop-up
  // panel)
  if (drageffect_state.buttons & wimp_CLICK_SELECT) /* buttons from start of drag */
    wimp_create_menu(wimp_CLOSE_MENU, 0, 0);

  return event_HANDLED;
}

/* called to drag virtual icons, those without actual icons */
static void drageffect_box(effectswin_t  *ew,
                           wimp_pointer *pointer,
                           int           effect,
                           int           moving,
                           const os_box *box)
{
  drageffect_state.buttons = pointer->buttons;
  drageffect_state.effect  = effect;
  drageffect_state.moving  = moving;

  drageffect_set_handlers(ew, 1);

  if (moving)
    /* turn this index into an effect number */
    effect = ew->effects.entries[effect].effect;

  drag_object_box(pointer->w, box,
                  pointer->pos.x,
                  pointer->pos.y,
                  drageffect_renderer,
         (void *) effect);
}

/* if not moving: effect is the number? of the new effect
 * if moving: effect is the index of the existing effect */
static void drageffect_icon(effectswin_t  *ew,
                            wimp_pointer *pointer,
                            int           effect,
                            int           moving)
{
  drageffect_state.buttons = pointer->buttons;
  drageffect_state.effect  = effect;
  drageffect_state.moving  = moving;

  drageffect_set_handlers(ew, 1);

  // FIXME: This needs if (moving) code from above; but it's not used atm

  drag_object(pointer->w,
              pointer->i,
              pointer->pos.x,
              pointer->pos.y,
              drageffect_renderer,
     (void *) effect);
}

/* ----------------------------------------------------------------------- */

/* scroll list stuff */

static void scroll_list_draw_row(wimp_draw *redraw,
                                 int        wax,
                                 int        way,
                                 int        i,
                                 int        sel,
                                 void      *opaque)
{
  effectswin_t   *ew = opaque;
  effects_element *e;

  NOT_USED(redraw);

  e = ew->effects.entries + i;

  draw_element(e, wax, way, sel);
}

static void scroll_list_draw_leader(wimp_draw *redraw,
                                    int        wax,
                                    int        way,
                                    int        i,
                                    int        sel,
                                    void      *opaque)
{
  int x,y;

  NOT_USED(i);
  NOT_USED(opaque);

  if (!sel)
    return;

  x = (redraw->box.x0 - redraw->xscroll) + wax;
  y = (redraw->box.y1 - redraw->yscroll) + way;

  draw_marker(x, y);
}

static void scroll_list_event_callback(scroll_list_event *event,
                                       void              *opaque)
{
  effectswin_t *ew = opaque;

  switch (event->type)
  {
  case scroll_list_SELECTION_CHANGED:
    new_selection(ew);
    break;

  case scroll_list_DRAG:
    drageffect_box(ew,
                   event->data.drag.pointer,
                   event->index,
                   1,
                  &event->data.drag.box);
    break;

  case scroll_list_DELETE:
    remove_effect(ew, event->index);
    break;
  }
}

/* ----------------------------------------------------------------------- */

static int add_event_mouse_click(wimp_event_no event_no,
                                 wimp_block   *block,
                                 void         *handle)
{
  wimp_pointer *pointer;
  effectswin_t  *ew = handle;

  NOT_USED(event_no);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case EFFECTS_ADD_B_CANCEL:
      wimp_create_menu(wimp_CLOSE_MENU, 0, 0);
      break;

    default:
      drageffect_icon(ew, pointer, pointer->i - 3, 0);
      break;
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static void clear_set_handlers(effectswin_t *ew, int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_COLOUR_PICKER_COLOUR_CHOICE, clear_message_colour_picker_colour_choice },
  };

  event_register_message_group(reg,
                               message_handlers,
                               NELEMS(message_handlers),
                               event_ANY_WINDOW,
                               event_ANY_ICON,
                               ew);
}

static int clear_edit(effectswin_t *ew, effects_element *e, int x, int y)
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

  clear_set_handlers(ew, 1);

  return 0;
}

static int clear_message_colour_picker_colour_choice(wimp_message *message,
                                                     void         *opaque)
{
  effectswin_t                        *ew = opaque;
  colourpicker_message_colour_choice *choice;

  choice = (colourpicker_message_colour_choice *) &message->data;

  ew->editing_element->args.clear.colour = choice->colour;

  effect_edited(ew, ew->editing_element);

  /* I don't think this is going to unregister in the case when 'Cancel' is
   * clicked. */
  clear_set_handlers(ew, 0);

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
  tonemap_set(tone.map, tone.channels, &tone.spec);

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

static void tone_slider_update(wimp_i i, int val, void *opaque)
{
  int  j;
  int  min,max;
  int *pval;

  NOT_USED(opaque);

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

static void tone_reset_dialogue(effectswin_t *ew)
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

  tonemap_get_values(tone.map, tone.channels, &tone.spec);

  /* set radio buttons */

  for (i = 0; i < NELEMS(map); i++)
    if (map[i].channels == tone.channels)
      break;

  icon_set_radio(GLOBALS.effects_crv_w, map[i].i);

  /* disable 'alpha' if required */

  icon_set_flags(GLOBALS.effects_crv_w,
                 EFFECTS_CRV_R_ALPHA,
                 (ew->image->flags & image_FLAG_HAS_ALPHA) ? 0 : wimp_ICON_SHADED,
                 wimp_ICON_SHADED);

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

static void tone_start_editing(effectswin_t *ew)
{
  tonemap_destroy(tone.map);

  /* copy everything across */

  tone = ew->editing_element->args.tone;

  /* clone the tonemap so we have a transient one to play with */

  tone.map = tonemap_copy(tone.map);

  tone_reset_dialogue(ew);

  /* recalculate tonemap, refresh gadget */

  tone_set_values_and_redraw();
}

static int tone_edit(effectswin_t *ew, effects_element *el, int x, int y)
{
  NOT_USED(ew);
  NOT_USED(el);

  tone_start_editing(ew);

  window_open_as_menu_here(GLOBALS.effects_crv_w, x, y);

  return 0;
}

static int tone_event_redraw_window_request(wimp_event_no event_no,
                                            wimp_block   *block,
                                            void         *handle)
{
  wimp_draw *redraw;

  redraw = &block->redraw;

  NOT_USED(event_no);
  NOT_USED(handle);

  tonemapgadget_redraw(tone.map,
                       GLOBALS.effects_crv_w,
                       EFFECTS_CRV_D_CURVE,
                       redraw);

  return event_HANDLED;
}

static int tone_event_mouse_click(wimp_event_no event_no,
                                  wimp_block   *block,
                                  void         *handle)
{
  effectswin_t  *ew = handle;
  wimp_pointer *pointer;

  NOT_USED(event_no);

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

        tonemap_destroy(ew->editing_element->args.tone.map);

        /* copy everything across */

        ew->editing_element->args.tone = tone;

        /* clone a new tonemap, because we need to keep ours in case we're
         * _not_ closing the dialogue */
        ew->editing_element->args.tone.map = tonemap_copy(tone.map);

        effect_edited(ew, ew->editing_element);
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

        tonemap_get_values(tone.map, tone.channels, &tone.spec);

        tone_reset_dialogue(ew);
        kick = 1;
      }

      if (pointer->buttons == wimp_CLICK_ADJUST)
        icon_set_radio(pointer->w, pointer->i);
    }
      break;

    case EFFECTS_CRV_B_RESET:
      effect_tone_reset(&tone);
      tone_reset_dialogue(ew);
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

        slider_start(pointer, r->min, r->max, tone_slider_update, ew);
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
        tone_start_editing(ew);
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

#define BLURMIN  2 /*  5-pt kernel */
#define BLURMAX 48 /* 95-pt kernel */

static effect_blur blur; /* transient effect_blur for editing */

static const slider_rec blur_sliders[] =
{
  { EFFECTS_BLR_S_EFFECT_PIT, BLURMIN, BLURMAX, 2, &blur.level },
};

//static void blur_set_values_and_redraw(void)
//{
//}

// callback from slider code
static void blur_slider_update(wimp_i i, int val, void *opaque)
{
  const slider_rec *r;
  int               min, max;
  int              *pval;

  NOT_USED(i);
  NOT_USED(opaque);

  r = &blur_sliders[0];

  min  = r->min;
  max  = r->max;
  pval = r->pval;

  val = CLAMP(val, min, max);

  *pval = val;

  icon_set_int(dialogue_get_window(&GLOBALS.effects_blr_d),
               EFFECTS_BLR_W_VAL,
               val);

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
    { effects_blur_BOX,      EFFECTS_BLR_R_BOX      },
    { effects_blur_GAUSSIAN, EFFECTS_BLR_R_GAUSSIAN },
  };

  const slider_rec *r;
  int               i;

  /* set radio buttons */

  for (i = 0; i < NELEMS(map); i++)
    if (map[i].method == blur.method)
      break;

  icon_set_radio(dialogue_get_window(&GLOBALS.effects_blr_d), map[i].i);

  /* set slider values */

  r = &blur_sliders[0];
  slider_set(dialogue_get_window(&GLOBALS.effects_blr_d),
             r->icon,
            *r->pval,
             r->min,
             r->max);

  icon_set_int(dialogue_get_window(&GLOBALS.effects_blr_d),
               EFFECTS_BLR_W_VAL,
              *r->pval);
}

static void blur_start_editing(effectswin_t *ew)
{
  /* copy everything across */

  blur = ew->editing_element->args.blur;

  blur_reset_dialogue();

  //blur_set_values_and_redraw();
}

static int blur_edit(effectswin_t *ew, effects_element *e, int x, int y)
{
  NOT_USED(ew);
  NOT_USED(e);

  blur_start_editing(ew);

  dialogue_show_here(&GLOBALS.effects_blr_d, x, y);

  return 0;
}

static int blur_event_mouse_click(wimp_event_no event_no,
                                  wimp_block   *block,
                                  void         *handle)
{
  effectswin_t  *ew = handle; // DPT THIS WON'T WORK WITH CURRENT DIALOGUE CODE WHICH RETURNS handlew pointing to the relevant dialogue_t
  wimp_pointer *pointer;
  int           val;

  NOT_USED(event_no);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    /* allow either grey or white icon, but not the 'pit' */
    if (EFFECTS_BLR_S_EFFECT_BACKGROUND == pointer->i ||
        EFFECTS_BLR_S_EFFECT_FOREGROUND == pointer->i)
    {
      slider_start(pointer, BLURMIN, BLURMAX, blur_slider_update, ew);
    }
    else
    {
      switch (pointer->i)
      {
      case EFFECTS_BLR_B_APPLY:
        val = icon_get_int(dialogue_get_window(&GLOBALS.effects_blr_d),
                           EFFECTS_BLR_W_VAL);
        blur.level = CLAMP(val, BLURMIN, BLURMAX);

        ew->editing_element->args.blur = blur;

        effect_edited(ew, ew->editing_element);
        break;

      case EFFECTS_BLR_B_CANCEL:
        break;

      case EFFECTS_BLR_R_BOX:
      case EFFECTS_BLR_R_GAUSSIAN:
        switch (pointer->i)
        {
        case EFFECTS_BLR_R_BOX:
          blur.method = effects_blur_BOX;
          break;

        case EFFECTS_BLR_R_GAUSSIAN:
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
        blur_start_editing(ew);
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* Allocates and clones images for editing, then sets up the blender. */
static result_t create_images(effectswin_t *ew)
{
  image_t           *image;
  osspriteop_area   *area;
  int                areasize;
  osspriteop_header *header1;
  osspriteop_header *header2;
  osspriteop_header *header3;
  int                r;

  image = ew->image;
  area  = (osspriteop_area *) image->image;

  areasize = area->size;

  /* stretch the image's heap block to three times its existing size and
   * add to it two duplicates of the existing image */

  r = flex_extend((flex_ptr) &image->image, areasize * 3);
  if (r == 0)
    return result_OOM;

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

  blender_create(&ew->blender, area);

  /* point the blender at the image data */
  ew->blender.src = sprite_data(header1);
  ew->blender.deg = sprite_data(header2);
  ew->blender.dst = sprite_data(header3);

  return result_OK;
}

static int delete_images(effectswin_t *ew)
{
  osspriteop_area   *area;
  int                areasize;
  osspriteop_header *header;
  int                r;

  area     = (osspriteop_area *) ew->image->image;
  areasize = area->size;

  /* Delete the last two sprites. The first is our result. */

  header = sprite_select(area, 2);
  osspriteop_delete_sprite(osspriteop_PTR, area, (osspriteop_id) header);

  header = sprite_select(area, 1);
  osspriteop_delete_sprite(osspriteop_PTR, area, (osspriteop_id) header);

  r = flex_extend((flex_ptr) &ew->image->image, areasize / 3);
  if (r == 0)
  {
    oserror_report(0, "error.no.mem");
    return 1;
  }

  area->size = flex_size((flex_ptr) &ew->image->image);

  return 0;
}

static void image_changed_callback(image_t             *image,
                                   imageobserver_change change,
                                   imageobserver_data  *data,
                                   void                *opaque)
{
  effectswin_t *ew = opaque;
 
  NOT_USED(image);
  NOT_USED(data);

  switch (change)
  {
  case imageobserver_CHANGE_HIDDEN:
  case imageobserver_CHANGE_ABOUT_TO_DESTROY:
    effects_close(ew);
    break;
  }
}

/* ----------------------------------------------------------------------- */

/* set handlers for per-window events */
static void effectswin_register_window_handlers(int reg, effectswin_t *ew)
{
//  static const event_wimp_handler_spec wimp_handlers[] =
//  {
//    { wimp_REDRAW_WINDOW_REQUEST, effectswin__event_redraw_window_request },
//    { wimp_CLOSE_WINDOW_REQUEST,  effectswin__event_close_window_request  },
//    { wimp_MOUSE_CLICK,           effectswin__event_mouse_click           },
//    { wimp_KEY_PRESSED,           effectswin__event_key_pressed           },
//    { wimp_SCROLL_REQUEST,        effectswin__event_scroll_request        },
//    { wimp_LOSE_CARET,            effectswin__event_lose_caret            },
//    { wimp_GAIN_CARET,            effectswin__event_gain_caret            },
//  };
//
//  event_register_wimp_group(reg,
//                            wimp_handlers,
//                            NELEMS(wimp_handlers),
//                            ew->window,
//                            event_ANY_ICON,
//                            ew);
}

static result_t effectswin_set_window_handlers(effectswin_t *ew)
{
  result_t err;

  effectswin_register_window_handlers(1, ew);

  err = help_add_window(ew->window, "effectwin");

  err = register_main(ew); // sets up scroll list
  err = register_add(ew);
  err = register_tone(ew);

  return err;
}

static void effectswin_unset_window_handlers(effectswin_t *ew)
{
  deregister_tone(ew);
  deregister_add(ew);
  deregister_main(ew);

  help_remove_window(ew->window);

  effectswin_register_window_handlers(0, ew);
}

/* ----------------------------------------------------------------------- */

/* Make the effects permanent, overwriting source. */
static void effects_apply(effectswin_t *ew)
{
  image_t         *image;
  osspriteop_area *area;

  image = ew->image;
  area  = (osspriteop_area *) image->image;

  image_about_to_modify(ew->image);

  /* copy result to source */

  copy_sprite_data(area, 2, 0);

  image_modified(ew->image, image_MODIFIED_DATA);

  remove_all_effects(ew);
}

static void effects_cancel(effectswin_t *ew)
{
  remove_all_effects(ew);
}

/* ---------------------------------------------------------------------- */

static struct
{
  list_t list_anchor; /* linked list of effectwins */
}
LOCALS;

/* ----------------------------------------------------------------------- */

static result_t effectswin_create(effectswin_t **new_ew, image_t *image)
{
  result_t     err;
  effectswin_t *ew = NULL;
  wimp_w       w  = wimp_ICON_BAR;

  *new_ew = NULL;

  ew = malloc(sizeof(*ew));
  if (ew == NULL)
    goto NoMem;

  /* Clone ourselves a window */
  w = window_clone(GLOBALS.effects_w);
  if (w == NULL)
    goto NoMem;

  ew->window = w;
  
  /* fill out */

  ew->image = image;
  effectarray_init(&ew->effects);
  ew->editing_element = NULL;

  err = effectswin_set_window_handlers(ew);
  if (err)
    goto Failure;


  image_start_editing(image);

  err = create_images(ew);
  if (err)
    goto Failure;

  image_select(image, 2);

  /* watch for changes */
  imageobserver_register(image, image_changed_callback, ew);

  /* open as a proper window */
  window_open_at(ew->window, AT_BOTTOMPOINTER);

  reset_main_dialogue(ew);


  // add to list
  list_add_to_head(&LOCALS.list_anchor, &ew->list);


  *new_ew = ew;

  return result_OK;


NoMem:
  err = result_OOM;
  goto Failure;


Failure:
  if (w != wimp_ICON_BAR)
    window_delete_cloned(w);

  free(ew);

  return err;
}

static void effectswin_destroy(effectswin_t *ew)
{
  if (ew == NULL)
    return;

  list_remove(&LOCALS.list_anchor, &ew->list);

  /* this kicks apply_effects, so is doing more work than necessary */
  remove_all_effects(ew);

  imageobserver_deregister(ew->image, image_changed_callback, NULL);

  image_select(ew->image, 0);
  image_preview(ew->image); /* force an update */

  delete_images(ew);

  image_stop_editing(ew->image);

  ew->image = NULL;

  window_close(ew->window);


  // standard stuff

  effectswin_unset_window_handlers(ew);

  wimp_delete_window(ew->window);
  // todo: destroy scrollist too here??

  free(ew);
}

/* ----------------------------------------------------------------------- */

void effects_open(image_t *image)
{
  result_t     err;
  effectswin_t *ew;

  if (!effects_available(image))
  {
    beep();
    return;
  }

  // search list for an effectwin on that image
  ew = (effectswin_t *) list_find(&LOCALS.list_anchor,
                                   offsetof(effectswin_t, image),
                             (int) image);
  if (ew)
  {
    /* pop it to the front */
    window_open_at(ew->window, AT_BOTTOMPOINTER);
    return;
  }

  err = effectswin_create(&ew, image);
  if (err)
    goto Failure;
  
  return;


Failure:
  result_report(err);
}

// is this needed?
static void effects_close(effectswin_t *ew)
{
  effectswin_destroy(ew);
}

/* ----------------------------------------------------------------------- */

int effects_available(const image_t *image)
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
