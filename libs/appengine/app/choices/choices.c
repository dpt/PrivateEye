/* --------------------------------------------------------------------------
 *    Name: choices.c
 * Purpose: Choices library
 * ----------------------------------------------------------------------- */

/*
 * Design ideas:
 *
 * 1) Keep all the data static.
 * 2) Keep all the actual choices elsewhere, the static array only has
 *    offsets/locations it knows about.
 *
 * Icon numbers are constant, so they can go in static data, but window
 * handles aren't. But I suppose an *address of* a window handle would be.
 *
 * Create menus on the fly.
 *
 *
 * Notes:
 *
 * Any place where VALINT is written through there should be an update
 * callback.
 */

#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/colourpicker.h"
#include "oslib/fileswitch.h"
#include "oslib/help.h"
#include "oslib/jpeg.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/wimp/colourpick.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/window.h"

#include "appengine/app/choices.h"

/* ----------------------------------------------------------------------- */

/* macros for accessing chosen values */

#define VALADDR(o)  ((char *) cs->valbuf + (o)) /* uses 'cs' */
#define VAL(T,o)    (*((T *) VALADDR(o)))
#define VALINT(o)   VAL(int, o)

/* macros for accessing proposed values */

#define PVALADDR(o) ((char *) cs->proposed_valbuf + (o)) /* uses 'cs' */
#define PVAL(T,o)   (*((T *) PVALADDR(o)))
#define PVALINT(o)  PVAL(int, o)

/* ----------------------------------------------------------------------- */

static event_wimp_handler choices_event_redraw_window_request,
                          choices_event_open_window_request,
                          choices_event_mouse_click_pane,
                          choices_event_mouse_click_main,
                          choices_event_menu_selection;

static event_message_handler choices_message_menus_deleted,
                             choices_message_colour_picker_colour_choice;

/* ----------------------------------------------------------------------- */

static unsigned int choices_refcount = 0;

result_t choices_init(void)
{
  result_t err;

  if (choices_refcount == 0)
  {
    /* dependencies */

    err = help_init();
    if (err)
      return err;
  }

  choices_refcount++;

  return result_OK;
}

void choices_fin(void)
{
  if (choices_refcount == 0)
    return;

  if (--choices_refcount == 0)
  {
    /* dependencies */

    help_fin();
  }
}

/* ----------------------------------------------------------------------- */

typedef result_t (for_every_callback)(const choices        *cs,
                                      const choices_group  *g,
                                      const choices_choice *c,
                                      void                 *opaque);

/* calls back the user-supplied callback function for every choices_choice in
 * the choices */

/* callback function returns an error to terminate early */

static result_t for_every_choice(const choices      *cs,
                                 for_every_callback *fn,
                                 void               *opaque)
{
  result_t err;
  int      i;

  for (i = 0; i < cs->ngroups; i++)
  {
    const choices_group *g;
    int                  j;

    g = cs->groups[i];

    for (j = 0; j < g->nchoices; j++)
    {
      const choices_choice *c;

      c = &g->choices[j];

      err = fn(cs, g, c, opaque);
      if (err == result_STOP_WALK)
        return result_OK;
      else if (err)
        return err;
    }
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

typedef result_t (for_every_pane_callback)(const choices      *cs,
                                           const choices_pane *p,
                                           void               *opaque);

static result_t for_every_pane(const choices           *cs,
                               for_every_pane_callback *fn,
                               void                    *opaque)
{
  result_t            err;
  const choices_pane *p;

  for (p = &cs->panes[0]; p < &cs->panes[cs->npanes]; p++)
  {
    err = fn(cs, p, opaque);
    if (err == result_STOP_WALK)
      return result_OK;
    else if (err)
      return err;
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

/* mapping is indexed by menu entry, so must do a linear search... */
static int stringset_index_of_val(const choices_stringset *string_set,
                                  int                      val)
{
  int i;

  for (i = 0; i < string_set->nelems; i++)
    if (string_set->elems[i].val == val)
      break;

  return (i < string_set->nelems) ? i : -1;
}

/* ----------------------------------------------------------------------- */

static result_t init_callback(const choices        *cs,
                              const choices_group  *g,
                              const choices_choice *c,
                              void                 *opaque)
{
  NOT_USED(g);
  NOT_USED(opaque);

  PVALINT(c->offset) = c->defaultval;

  return result_OK;
}

static result_t init(const choices *cs)
{
  result_t err;

  cs->vars->temporary_colour = NULL;
  cs->vars->current_menu     = NULL;

  /* initialise the *proposed* choices */

  err = for_every_choice(cs, init_callback, NULL);
  if (err)
    return err;

  return result_OK;
}

/* ----------------------------------------------------------------------- */

result_t choices_load(const choices *cs)
{
  char  buf[256]; /* Careful Now */
  FILE *f;

  init(cs); /* set up defaults */

  sprintf(buf, "Choices:%s.Choices", cs->app);

  f = fopen(buf, "r");
  if (f != NULL)
  {
    while (fgets(buf, sizeof(buf), f) != NULL)
    {
      char        *bufptr;
      unsigned int value;
      int          i;

      if (isspace(*buf) || *buf == '#')
        continue;

      for (bufptr = buf; *bufptr != ':'; bufptr++)
        ;
      *bufptr++ = '\0';

      sscanf(bufptr, "%x", &value);

      for (bufptr = buf; *bufptr != '.'; bufptr++)
        ;
      *bufptr++ = '\0';

      /* buf points to group name */
      /* bufptr points to choice name (within group) */

      for (i = 0; i < cs->ngroups; i++)
        if (strcmp(buf, cs->groups[i]->name) == 0)
          break;

      if (i < cs->ngroups)
      {
          const choices_group *g;

          g = cs->groups[i];

          for (i = 0; i < g->nchoices; i++)
            if (strcmp(bufptr, g->choices[i].name) == 0)
              break;

          if (i < g->nchoices)
          {
            const choices_choice *c;

            c = &g->choices[i];
            PVALINT(c->offset) = value;
          }
          else
          {
            /* printf("Choice '%s' is unknown\n", buf); */
          }
      }
      else
      {
        /* printf("Group '%s' is unknown\n", buf); */
      }
    }
    fclose(f);
  }

  /* copy from the proposed_choices we've just set up into the 'real' choices
   * which the application will see and call any callbacks */
  return choices_set(cs);
}

/* ----------------------------------------------------------------------- */

struct save_callback_args
{
  FILE *f;
};

static result_t save_callback(const choices        *cs,
                              const choices_group  *g,
                              const choices_choice *c,
                              void                 *opaque)
{
  struct save_callback_args *args = opaque;
  int                        val;

  val = PVALINT(c->offset);
  if (val != c->defaultval) /* save only differences from the defaults */
    fprintf(args->f, "%s.%s:%08x\n", g->name, c->name, val);

  return result_OK;
}

result_t choices_save(const choices *cs)
{
  char                      buf[256]; /* Careful Now */
  struct save_callback_args args;
  fileswitch_object_type    type;

  sprintf(buf, "<Choices$Write>.%s", cs->app);

  /* Does the choices directory exist? */
  type = osfile_read_no_path(buf, NULL, NULL, NULL, NULL);
  if (type != fileswitch_IS_DIR)
  {
    /* it's not a directory - try making one */
    osfile_create_dir(buf, 0);
  }

  sprintf(buf, "<Choices$Write>.%s.Choices", cs->app);

  args.f = fopen(buf, "w");
  if (args.f == NULL)
    return result_FILE_OPEN_FAILED;

  for_every_choice(cs, save_callback, &args);

  fclose(args.f);

  choices_set(cs); /* choosing 'Save' includes a 'Set' */

  return result_OK;
}

/* ----------------------------------------------------------------------- */

/* entry points */

/*
static result_t update_icons_colour(const choices        *cs,
                                    const choices_group  *g,
                                    const choices_choice *c)
{
  return result_OK;
}
*/

static result_t update_icons_numberrange(const choices        *cs,
                                         const choices_group  *g,
                                         const choices_choice *c)
{
  static const double        precs[] = { 10.0, 100.0, 1000.0 };

  const choices_numberrange *number_range;
  int                        val;

  number_range = c->data.number_range;

  if (number_range == NULL)
    return result_OK; /* let ranges exist internally but have no display */

  val = PVALINT(c->offset);

  if (number_range->prec == 0)
  {
    icon_set_int(*cs->panes[g->pane_index].window,
                  number_range->icon_display,
                  val);
  }
  else
  {
    assert(number_range->prec >= 1 && number_range->prec <= 3);

    icon_set_double(*cs->panes[g->pane_index].window,
                     number_range->icon_display,
                     val / precs[number_range->prec - 1],
                     number_range->prec);
  }

  return result_OK;
}

static result_t update_icons_option(const choices        *cs,
                                    const choices_group  *g,
                                    const choices_choice *c)
{
  const choices_option *option;
  int                   val;

  option = c->data.option;

  val = PVALINT(c->offset);

  icon_set_selected(*cs->panes[g->pane_index].window,
                     option->icon,
                     val != 0);

  return result_OK;
}

static result_t update_icons_stringset(const choices        *cs,
                                       const choices_group  *g,
                                       const choices_choice *c)
{
  const choices_stringset *string_set;
  char                     buf[32]; /* Careful Now */
  int                      val;
  int                      i;
  wimp_selection           sel;
  const char              *text;

  string_set = c->data.string_set;

  val = PVALINT(c->offset);

  i = stringset_index_of_val(string_set, val);

  sprintf(buf, "menu.%s", string_set->name);

  sel.items[0] = i;
  sel.items[1] = -1;
  text = menu_desc_name_from_sel(message0(buf), &sel);

  icon_set_text(*cs->panes[g->pane_index].window,
                 string_set->icon_display,
                 text);

  return result_OK;
}

static result_t update_icons_callback(const choices        *cs,
                                      const choices_group  *g,
                                      const choices_choice *c,
                                      void                 *opaque)
{
  NOT_USED(opaque);

  switch (c->type)
  {
  case choices_TYPE_COLOUR:
    /* update_icons_colour(cs, g, c); */ /* has no effect presently */
    break;

  case choices_TYPE_NUMBER_RANGE:
    update_icons_numberrange(cs, g, c);
    break;

  case choices_TYPE_OPTION:
    update_icons_option(cs, g, c);
    break;

  case choices_TYPE_STRING_SET:
    update_icons_stringset(cs, g, c);
    break;

  default:
    assert(0);
    break;
  }

  return result_OK;
}

/* Populate display icons from stores choices */
result_t choices_update_icons(const choices *cs)
{
  return for_every_choice(cs, update_icons_callback, NULL);
}

/* ----------------------------------------------------------------------- */

static void choices_set_pane_handlers(int                 reg,
                                      const choices      *cs,
                                      const choices_pane *p)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, choices_event_redraw_window_request },
    { wimp_MOUSE_CLICK,           choices_event_mouse_click_pane      },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                           *p->window,
                            event_ANY_ICON,
                            cs);
}

static result_t create_windows_callback(const choices      *cs,
                                        const choices_pane *p,
                                        void               *opaque)
{
  result_t err;
  char     buf[13];

  NOT_USED(opaque);

  if (p->window == NULL)
    return result_OK; /* this group has no associated window */

  sprintf(buf, "choices_%s", p->name);

  *p->window = window_create(buf);
  if (*p->window == 0)
    return result_OOM; /* potentially inaccurate */

  err = help_add_window(*p->window, buf);
  if (err)
    return err;

  choices_set_pane_handlers(1, cs, p);

  if (p->handlers && p->handlers->initialise_callback)
    p->handlers->initialise_callback(p);

  return result_OK;
}

static void choices_set_handlers(int reg, const choices *cs)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_OPEN_WINDOW_REQUEST, choices_event_open_window_request },
    { wimp_MOUSE_CLICK,         choices_event_mouse_click_main    },
    { wimp_MENU_SELECTION,      choices_event_menu_selection      },
  };

  static const event_message_handler_spec message_handlers[] =
  {
    { message_MENUS_DELETED,               choices_message_menus_deleted               },
    { message_COLOUR_PICKER_COLOUR_CHOICE, choices_message_colour_picker_colour_choice },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                           *cs->window,
                            event_ANY_ICON,
                            cs);

  event_register_message_group(reg,
                               message_handlers,
                               NELEMS(message_handlers),
                               event_ANY_WINDOW,
                               event_ANY_ICON,
                               cs);
}

/* create panes */
result_t choices_create_windows(const choices *cs)
{
  result_t err;

  *cs->window = window_create("choices");
  if (*cs->window == 0)
    return result_OOM; /* potentially inaccurate */

  err = help_add_window(*cs->window, "choices");
  if (err)
    return err;

  err = for_every_pane(cs, create_windows_callback, NULL);
  if (err)
    return err;

  choices_set_handlers(1, cs);

  *cs->current = wimp_TOP;

  return result_OK;
}

static result_t destroy_windows_callback(const choices      *cs,
                                         const choices_pane *p,
                                         void               *opaque)
{
  NOT_USED(opaque);

  if (p->handlers && p->handlers->finalise_callback)
    p->handlers->finalise_callback(p);

  if (p->window == NULL)
    return result_OK; /* this group has no associated window */

  help_remove_window(*p->window);

  choices_set_pane_handlers(0, cs, p);

  // Can't use this just now as it expects the window to have been
  // window_cloned first.
  //
  // window_delete(*p->window);

  return result_OK;
}

void choices_destroy_windows(const choices *cs)
{
  choices_set_handlers(0, cs);

  (void) for_every_pane(cs, destroy_windows_callback, NULL);

  // window_delete(*cs->window);
}

/* ----------------------------------------------------------------------- */

static result_t redraw_window_callback(const choices      *cs,
                                       const choices_pane *p,
                                       void               *opaque)
{
  result_t   err;
  wimp_draw *redraw = opaque;
  int        more;

  NOT_USED(cs);

  if (redraw->w != *p->window)
    return result_OK; /* not our window */

  if (p->handlers && p->handlers->redraw_callback)
  {
    err = p->handlers->redraw_callback(p, redraw);
    if (err)
      return err;
  }
  else
  {
    /* This might not get used in the common case, but will at least stop the
     * application locking up if a required redraw handler is omitted. */
    for (more = wimp_redraw_window(redraw);
         more;
         more = wimp_get_rectangle(redraw))
      ;
  }

  return result_STOP_WALK; /* terminate early */
}

int choices_event_redraw_window_request(wimp_event_no event_no,
                                        wimp_block   *block,
                                        void         *handle)
{
  const choices *cs = handle;

  NOT_USED(event_no);

  (void) for_every_pane(cs, redraw_window_callback, &block->redraw);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

int choices_event_open_window_request(wimp_event_no event_no,
                                      wimp_block   *block,
                                      void         *handle)
{
  const choices *cs = handle;

  NOT_USED(event_no);

  if (block->open.w != *cs->window)
    return event_NOT_HANDLED;

  wimp_open_window(&block->open);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static result_t attach_child(const choices *cs, wimp_w w)
{
  wimp_window_state         state;
  wimp_window_nesting_flags linkage;

  state.w = w;
  wimp_get_window_state(&state);

  state.w = *cs->current;

  /* to be honest, this nested opening stuff baffles me */
  /* the 4s account for 3D window border effects */

  state.visible.x0 = state.visible.x0 - state.xscroll + 4;
  state.visible.y0 = state.visible.y0 - state.yscroll + 4;
  state.visible.x1 = state.visible.x1 - state.xscroll - 4;
  state.visible.y1 = state.visible.y1 - state.yscroll - 4;

  linkage = (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_LS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_BS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_RS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_TS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_XORIGIN_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_YORIGIN_SHIFT);

  wimp_open_window_nested((wimp_open *) &state, w, linkage);

  return result_OK;
}

/* Opening a choiceset for the first time. */
/* i.e. 'Choices...' selected from icon bar menu. */
result_t choices_open(const choices *cs)
{
  wimp_w            w;
  wimp_window_state state;

  w = *cs->window;

  state.w = w;
  wimp_get_window_state(&state);
  if (state.flags & wimp_WINDOW_OPEN)
  {
    window_open(w);
  }
  else
  {
    if (*cs->current == wimp_TOP) /* not previously opened */
    {
      /* Select the appropriate radio button */

      icon_set_radio(w, cs->panes[0].icon);

      *cs->current = *cs->panes[0].window;
    }

    /* strictly here we should copy the existing choices to the proposed
     * choices, but it's not really necessary right now */

    choices_update_icons(cs);

    attach_child(cs, w);

    /* Open main above icon bar, horizontally centred on pointer */

    window_open_at(w, AT_BOTTOMPOINTER);
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

result_t choices_set(const choices *cs)
{
  result_t err;
  int      i;

  for (i = 0; i < cs->ngroups; i++)
  {
    const choices_group *g;
    int                  j;
    int                  changes = 0;

    g = cs->groups[i];

    for (j = 0; j < g->nchoices; j++)
    {
      const choices_choice *c;

      c = &g->choices[j];

      if (VALINT(c->offset) != PVALINT(c->offset))
      {
        VALINT(c->offset) = PVALINT(c->offset);
        changes = 1;
      }
    }

    if (changes && g->handlers && g->handlers->changed_callback)
    {
      err = g->handlers->changed_callback(g);
      if (err)
        return err;
    }
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

static result_t call_changed_callback(const choices      *cs,
                                      const choices_pane *p,
                                      void               *opaque)
{
  result_t err;

  NOT_USED(cs);
  NOT_USED(opaque);

  if (p->handlers && p->handlers->changed_callback)
  {
    err = p->handlers->changed_callback(p);
    if (err)
      return err;
  }

  return result_OK;
}

result_t choices_cancel(const choices *cs)
{
  result_t err;

  memcpy(cs->proposed_valbuf, cs->valbuf, cs->valbufsz);

  err = choices_update_icons(cs);
  if (err)
    return err;

  /* all the proposed values have been reset: need to call ALL update
   * handlers */

  err = for_every_pane(cs, call_changed_callback, NULL);
  if (err)
    return err;

  return result_OK;
}

/* ----------------------------------------------------------------------- */

static result_t mouse_click_pane_select(const choices *cs,
                                        wimp_pointer  *pointer)
{
  const choices_pane *p;
  wimp_w              mainw;
  wimp_w              old;
  wimp_w              pane;

  for (p = &cs->panes[0]; p < &cs->panes[cs->npanes]; p++)
    if (pointer->i == p->icon)
      break;

  if (p == &cs->panes[cs->npanes])
    return result_OK; /* not a radio click (e.g. work area) */

  mainw = *cs->window;

  /* Switch radio icons back on if they are ADJUST-clicked */
  if (pointer->buttons & wimp_CLICK_ADJUST)
    icon_set_selected(mainw, p->icon, TRUE);

  pane = *p->window;
  old  = *cs->current;

  if (pane != old)
  {
    wimp_close_window(old); /* close last pane */

    *cs->current = pane;

    attach_child(cs, mainw);
  }

  return result_OK;
}

static result_t mouse_click_colour(const choices        *cs,
                                   const choices_group  *g,
                                   const choices_choice *c,
                                   wimp_pointer         *pointer)
{
  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_MENU))
  {
    unsigned int *pval;

    /* stash in 'cs->temporary_colour' the address of a var to receive the
     * colour, when the colour picker returns it. */

    pval = (unsigned int *) PVALADDR(c->offset);

    cs->vars->temporary_colour = pval;

    colourpick_popup(*cs->panes[g->pane_index].window,
                      c->data.colour->icon,
                     *pval);

    /* no callback here */
  }

  return result_OK;
}

static result_t mouse_click_numberrange(const choices        *cs,
                                        const choices_group  *g,
                                        const choices_choice *c,
                                        wimp_pointer         *pointer)
{
  result_t err;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    const choices_numberrange *number_range;
    int                        n;
    int                        inc;
    int                        old;

    number_range = c->data.number_range;

    n = PVALINT(c->offset);

    inc = number_range->inc;

    if ((pointer->i == number_range->icon_up   && pointer->buttons == wimp_CLICK_ADJUST) ||
        (pointer->i == number_range->icon_down && pointer->buttons == wimp_CLICK_SELECT))
    {
      inc = -inc;
    }

    old = n;

    n += inc;
    if (n < number_range->min)
      n = number_range->min;
    else if (n > number_range->max)
      n = number_range->max;

    if (n != old)
    {
      PVALINT(c->offset) = n;

      err = update_icons_numberrange(cs, g, c);
      if (err)
        return err;

      err = call_changed_callback(cs, &cs->panes[g->pane_index], NULL);
      if (err)
        return err;
    }
  }

  return result_OK;
}

static result_t mouse_click_option(const choices        *cs,
                                   const choices_group  *g,
                                   const choices_choice *c,
                                   wimp_pointer         *pointer)
{
  result_t err;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    const choices_option *option;

    option = c->data.option;

    PVALINT(c->offset) = !PVALINT(c->offset);

    err = call_changed_callback(cs, &cs->panes[g->pane_index], NULL);
    if (err)
      return err;
  }

  return result_OK;
}

static result_t mouse_click_stringset(const choices        *cs,
                                      const choices_group  *g,
                                      const choices_choice *c,
                                      wimp_pointer         *pointer)
{
  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_MENU))
  {
    result_t                 err;
    const choices_stringset *string_set;
    char                     buf[32]; /* Careful Now */
    wimp_menu               *menu;
    int                      val;
    int                      i;

    /* destroy any previous menu (which hasn't been reaped by the
     * menus_deleted handler) */

    if (cs->vars->current_menu)
    {
      help_remove_menu(cs->vars->current_menu);
      menu_destroy(cs->vars->current_menu);
      cs->vars->current_menu = NULL;
    }

    string_set = c->data.string_set;

    sprintf(buf, "menu.%s", c->data.string_set->name);

    menu = menu_create_from_desc(message(buf));
    if (menu == NULL)
      return result_OOM; /* potentially inaccurate */

    err = help_add_menu(menu, c->data.string_set->name);
    if (err)
      return err;

    val = PVALINT(c->offset);

    i = stringset_index_of_val(string_set, val);

    menu_tick_exclusive(menu, i);

    menu_popup(*cs->panes[g->pane_index].window,
                string_set->icon_popup,
                menu);

    cs->vars->current_menu = menu;
    cs->vars->group_menu   = g;
    cs->vars->choice_menu  = c;

    /* callback here */
  }

  return result_OK;
}

int choices_event_mouse_click_pane(wimp_event_no  event_no,
                                   wimp_block    *block,
                                   void          *handle)
{
  const choices *cs      = handle;
  wimp_pointer  *pointer = &block->pointer;
  int            i;

  NOT_USED(event_no);

  for (i = 0; i < cs->ngroups; i++)
  {
    const choices_group *g;
    int                  j;

    g = cs->groups[i];

    if (g->pane_index < 0)
      continue; /* no UI */

    if (pointer->w != *cs->panes[g->pane_index].window)
      continue;

    for (j = 0; j < g->nchoices; j++)
    {
      const choices_choice *c;

      c = &g->choices[j];

      if (c->data.ui == NULL)
        continue; /* no UI */

      switch (c->type)
      {
      case choices_TYPE_COLOUR:
        if (pointer->i == c->data.colour->icon)
        {
          (void) mouse_click_colour(cs, g, c, pointer);
          return event_HANDLED;
        }
        break;

      case choices_TYPE_NUMBER_RANGE:
        if (pointer->i == c->data.number_range->icon_down ||
            pointer->i == c->data.number_range->icon_up)
        {
          (void) mouse_click_numberrange(cs, g, c, pointer);
          return event_HANDLED;
        }
        break;

      case choices_TYPE_OPTION:
        if (pointer->i == c->data.option->icon)
        {
          (void) mouse_click_option(cs, g, c, pointer);
          return event_HANDLED;
        }
        break;

      case choices_TYPE_STRING_SET:
        if (pointer->i == c->data.string_set->icon_popup)
        {
          (void) mouse_click_stringset(cs, g, c, pointer);
          return event_HANDLED;
        }
        break;
      }
    }
  }

  return event_HANDLED;
}

int choices_event_mouse_click_main(wimp_event_no event_no,
                                   wimp_block   *block,
                                   void         *handle)
{
  const choices *cs      = handle;
  wimp_pointer  *pointer = &block->pointer;

  NOT_USED(event_no);

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    int set, cancel, save;

    set    = pointer->i == cs->icon_set;
    cancel = pointer->i == cs->icon_cancel;
    save   = pointer->i == cs->icon_save;

    if (set || cancel || save)
    {
      if (set)
        choices_set(cs);
      else if (cancel)
        choices_cancel(cs);
      else if (save)
        choices_save(cs);

      if (pointer->buttons == wimp_CLICK_SELECT)
      {
        window_close(*cs->window);
        window_close(*cs->current);
      }
    }
    else /* possibly a radio button to select a choices pane */
    {
      mouse_click_pane_select(cs, pointer);
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* menu selection handler */

int choices_event_menu_selection(wimp_event_no event_no,
                                 wimp_block   *block,
                                 void         *handle)
{
  const choices           *cs        = handle;
  wimp_selection          *selection = &block->selection;
  wimp_menu               *last;
  const choices_choice    *c;
  const choices_group     *g;
  const choices_stringset *string_set;
  int                      i;

  NOT_USED(event_no);

  last = menu_last();
  if (last != cs->vars->current_menu)
    return event_NOT_HANDLED;

  c = cs->vars->choice_menu; /* the choice this menu selection relates to */
  g = cs->vars->group_menu;

  string_set = c->data.string_set;

  i = selection->items[0];
  if (i < string_set->nelems)
  {
    const choices_pane *p;

    PVALINT(c->offset) = string_set->elems[i].val;

    update_icons_stringset(cs, g, c);

    p = &cs->panes[g->pane_index];
    if (p->handlers && p->handlers->changed_callback)
      (void) p->handlers->changed_callback(p);
  }

  {
    wimp_pointer p;

    wimp_get_pointer_info(&p);
    if (p.buttons & wimp_CLICK_ADJUST)
    {
      if (i < string_set->nelems)
        menu_tick_exclusive(cs->vars->current_menu, i);

      menu_reopen();
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* menus deleted handler */

static int choices_message_menus_deleted(wimp_message *message, void *handle)
{
  const choices *cs = handle;

  NOT_USED(message);

  if (cs->vars->current_menu == NULL)
    return event_NOT_HANDLED;

  help_remove_menu(cs->vars->current_menu);

  menu_destroy(cs->vars->current_menu);

  cs->vars->current_menu = NULL;
  cs->vars->group_menu   = NULL;
  cs->vars->choice_menu  = NULL;

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* colour choice handler */

static int choices_message_colour_picker_colour_choice(wimp_message *message,
                                                       void         *handle)
{
  const choices *cs = handle;

  colourpicker_message_colour_choice *choice;

  if (cs->vars->temporary_colour == NULL)
    return event_NOT_HANDLED;

  choice = (colourpicker_message_colour_choice *) &message->data;

  if (choice->flags & colourpicker_COLOUR_TRANSPARENT)
    *cs->vars->temporary_colour = os_COLOUR_TRANSPARENT;
  else
    *cs->vars->temporary_colour = choice->colour;

  /* should raise an update callback here, but have no |choices_pane *| */

  return event_HANDLED;
}
