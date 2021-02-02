/* --------------------------------------------------------------------------
 *    Name: info.c
 * Purpose: Info window
 * ----------------------------------------------------------------------- */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"
#include "oslib/wimpspriteop.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/base/oserror.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"
#include "appengine/io/filing.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"
#include "appengine/types.h"
#include "appengine/wimp/window.h"

#include "appengine/dialogues/info.h"

/* ----------------------------------------------------------------------- */

static event_wimp_handler info_event_mouse_click;

/* ----------------------------------------------------------------------- */

dialogue_t *info_create(const char *template)
{
  info_t *s;

  s = calloc(1, sizeof(*s));
  if (s == NULL)
    return NULL;

  info_construct(s, template);

  return &s->dialogue;
}

void info_destroy(dialogue_t *d)
{
  info_t *s;

  s = (info_t *) d;

  info_destruct(s);

  free(s);
}

/* ----------------------------------------------------------------------- */

void info_construct(info_t *s, const char *template)
{
  wimp_w          w;
  wimp_icon_flags flags;
  wimp_i          which[32]; /* Careful Now */
  int             i;
  int             j;

  dialogue_construct(&s->dialogue, template, -1, -1);

  dialogue_set_mouse_click_handler(&s->dialogue,
                                   info_event_mouse_click);

  w = dialogue_get_window(&s->dialogue);

  /* discover the handle of the file type icon */

  flags = wimp_ICON_TEXT   |
          wimp_ICON_SPRITE |
          wimp_ICON_BORDER |
          wimp_ICON_INDIRECTED;
  wimp_which_icon(w, which, flags, flags);
  s->file_type_icon = which[0];
  /* if there's no file type icon, this will be -1 */

  /* discover the handles of the display icons */

  flags = wimp_ICON_TEXT | wimp_ICON_BORDER | wimp_ICON_INDIRECTED;
  wimp_which_icon(w, which, flags | wimp_ICON_SPRITE, flags);
  j = 0;
  for (i = 0; ; i++)
  {
    wimp_i icon;

    icon = which[i];
    if (icon < 0)
      break;

    if (icon == s->file_type_icon)
      continue; /* have already spotted this one */

    s->displays[j++] = icon;
  }
  s->ndisplays = j;

  s->padding = 0;
}

void info_destruct(info_t *s)
{
  dialogue_destruct(&s->dialogue);
}

/* ----------------------------------------------------------------------- */

/* This isn't really used yet. */
static int info_event_mouse_click(wimp_event_no event_no,
                                  wimp_block   *block,
                                  void         *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

void info_set_file_type(dialogue_t *d, bits file_type)
{
  info_t *s;
  wimp_w  w;
  char    sprname[osspriteop_NAME_LIMIT];
  char    name[8];

  s = (info_t *) d;

  if (s->file_type_icon < 0)
    return;

  w = dialogue_get_window(d);

  file_type_to_sprite_name(file_type, sprname);

  /* WatchMe: Need space in template. */
  icon_validation_printf(w, s->file_type_icon, "Nd_icon;S%s;R2", sprname);

  file_type_to_name(file_type, name);

  icon_printf(w, s->displays[0], "%s (&%03x)", name, file_type);
}

void info_set_info(dialogue_t *d, info_spec_t *specs, int nspecs)
{
  info_t *s;
  wimp_w  w;
  wimp_i *displays;
  int     i;

  s = (info_t *) d;

  w = dialogue_get_window(d);

  displays = s->displays;

  /* if there's a file type icon present the first display field is used to
   * show the file type info, so skip that */
  if (s->file_type_icon >= 0)
    displays++;

  for (i = 0; i < nspecs; i++)
    icon_set_text(w, *displays++, specs[i].value);
}

static void resize_icon_x(wimp_w w, wimp_i i, int x0, int x1)
{
  if (wimpreadsysinfo_version() >= wimp_VERSION_RO35)
  {
    wimp_icon_state state;

    state.w = w;
    state.i = i;
    wimp_get_icon_state(&state);

    wimp_resize_icon(w, i, x0, state.icon.extent.y0,
                           x1, state.icon.extent.y1);
  }
}

/* Read the OS unit width of the sprite. */
static int get_sprite_width(const char *validation)
{
  os_error *err;
  char      buf[32]; /* validation could be "spritename,spritename" */
  int       width;
  os_mode   mode;
  int       xeig;

  icon_sprite_name(validation, buf);

  err = xwimpspriteop_read_sprite_info(buf, &width, NULL, NULL, &mode);
  if (err) // FIXME: Ought to be a specific error number tested here.
    err = xosspriteop_read_sprite_info(osspriteop_NAME,
                   (osspriteop_area *) window_get_sprite_area(),
                       (osspriteop_id) buf,
                                      &width,
                                       NULL,
                                       NULL,
                                      &mode);

  if (err)
    return 68; /* default */

  os_read_mode_variable(mode, os_MODEVAR_XEIG_FACTOR, &xeig);
  return width << xeig;
}

void info_layout(dialogue_t *d)
{
#define LABELGAP         (8)  /* gap between labels and displays */
#define SPRITEICONBORDER (22) /* border per-side for the sprite icon */
#define TEXTICONBORDER   (4)  /* border per-side at edges of text */

  osbool          have_textop;
  info_t         *s;
  wimp_w          w;
  wimp_icon_state istate;
  int             maxwidth;
  int             i;
  int             x0,x1;
  int             spriteiconwidth;
  int             spritewidth;
  os_box          box;

  have_textop = (wimpreadsysinfo_version() >= wimp_VERSION_RO35);
  if (!have_textop)
    return;

  s = (info_t *) d;

  w = dialogue_get_window(d);

  istate.w = w;

  if (s->file_type_icon >= 0)
  {
    istate.i = s->file_type_icon;
    wimp_get_icon_state(&istate);

    spritewidth = get_sprite_width(istate.icon.data.indirected_text_and_sprite.validation);

    spriteiconwidth = spritewidth + 2 * SPRITEICONBORDER;
  }
  else
  {
    spriteiconwidth = 0;
  }

  /* calculate how wide display icons need to be */
  maxwidth = 0;
  for (i = 0; i < s->ndisplays; i++)
  {
    const char *str;
    size_t      len;
    int         width;

    istate.i = s->displays[i];
    wimp_get_icon_state(&istate);

    str = istate.icon.data.indirected_text.text;

    len = str_len(str);
    width = wimptextop_string_width(str, len);
    width += 2 * TEXTICONBORDER;

    /* add on sprite width */
    if (s->file_type_icon >= 0 && i < 2)
      width += spriteiconwidth;

    if (width > maxwidth)
      maxwidth = width;
  }

  maxwidth += s->padding;

  /* calculate leftmost bound of display icons */
  istate.i = 0;
  wimp_get_icon_state(&istate);
  x0 = LABELGAP + (istate.icon.extent.x1 - istate.icon.extent.x0) + LABELGAP;

  /* calculate rightmost bound of display icons */
  x1 = x0 + maxwidth;

  if (s->file_type_icon >= 0)
  {
    resize_icon_x(w, s->file_type_icon,     x1 - spriteiconwidth, x1);
    resize_icon_x(w, s->file_type_icon + 1, x1 - spriteiconwidth, x1 - spriteiconwidth + 4);
  }

  for (i = 0; i < s->ndisplays; i++)
  {
    int r;

    /* The first two display icons have to have their size adjusted because
     * they're next to the icon field, where present. */
    r = x1;
    if (s->file_type_icon >= 0 && i < 2)
      r -= spriteiconwidth - 4; /* adjust for shim */

    resize_icon_x(w, s->displays[i], x0, r);
  }

  /* set window size (may need to reopen behind backwindow etc.) */

  box.x0 = 0;
  box.x1 = x1 + 8;
  window_set_submenu_extent(w, 5 /* flags */, &box);
}

void info_set_padding(dialogue_t *d, int padding)
{
  info_t *s;

  s = (info_t *) d;

  s->padding = padding;
}
