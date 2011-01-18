/* --------------------------------------------------------------------------
 *    Name: choices.h
 * Purpose: Choices library interface
 * Version: $Id: choices.h,v 1.1 2009-06-11 20:49:28 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_CHOICES_H
#define APPENGINE_CHOICES_H

#include <stddef.h>

#include "appengine/base/errors.h"

#include "oslib/wimp.h"

typedef void choices_valbuf;

/* ----------------------------------------------------------------------- */

typedef enum choices_type
{
  choices_TYPE_COLOUR,
  choices_TYPE_NUMBER_RANGE,
  choices_TYPE_OPTION,
  choices_TYPE_STRING_SET
}
choices_type;

typedef struct choices_stringset_vals
{
  int val;
}
choices_stringset_vals;

typedef struct choices_stringset
{
  const char                   *name; /* name of menu in messages file */
  wimp_i                        icon_display, icon_popup;
  int                           nelems;
  const choices_stringset_vals *elems;
}
choices_stringset;

typedef struct choices_numberrange
{
  wimp_i icon_display, icon_down, icon_up;
  int    min, max;
  int    inc;  /* increment */
  int    prec; /* precision */
}
choices_numberrange;

typedef struct choices_option
{
  wimp_i icon;
}
choices_option;

typedef struct choices_colour
{
  wimp_i icon;
}
choices_colour;

/* A single choice */
typedef struct choices_choice
{
  const char                  *name;
  int                          offset;
  choices_type                 type;
  int                          defaultval;
  union
  {
    const choices_colour      *colour;
    const choices_numberrange *number_range;
    const choices_option      *option;
    const choices_stringset   *string_set;
  }
  data;
}
choices_choice;

/* ----------------------------------------------------------------------- */

error choices_init(void);
void choices_fin(void);

/* ----------------------------------------------------------------------- */

typedef struct choices_pane choices_pane;

typedef error (choices_pane_initialise_handler)(const choices_pane *);
typedef void (choices_pane_finalise_handler)(const choices_pane *);
typedef error (choices_pane_changed_handler)(const choices_pane *);
typedef error (choices_pane_redraw_handler)(const choices_pane *,
                                                  wimp_draw    *);

/* these handlers deal with 'proposed' choices */
typedef struct choices_pane_handlers
{
  choices_pane_initialise_handler *initialise_callback;
  choices_pane_finalise_handler   *finalise_callback;
  /* called when the proposed choices are changed */
  choices_pane_changed_handler    *changed_callback;
  choices_pane_redraw_handler     *redraw_callback;
}
choices_pane_handlers;

struct choices_pane
{
  wimp_w                      *window;  /* pointer to window handle */
  const char                   name[4]; /* eg. 'vwr' */
  wimp_i                       icon;    /* radio icon in main window */
  const choices_pane_handlers *handlers;
};

/* ----------------------------------------------------------------------- */

/* A grouping of choices */
typedef struct choices_group choices_group;

typedef error (choices_group_changed_handler)(const choices_group *);

typedef struct choices_group_handlers
{
  /* called when the choices are set and the group has changes */
  choices_group_changed_handler *changed_callback;
}
choices_group_handlers;

struct choices_group
{
  const char                   *name;      /* eg. 'viewer' */
  int                           nchoices;
  const choices_choice         *choices;
  int                           pane_index;
  const choices_group_handlers *handlers;
};

/* ----------------------------------------------------------------------- */

/* Since everything else is const, keep the variables self-contained */
typedef struct choices_vars choices_vars;

struct choices_vars
{
  unsigned int         *temporary_colour; /* stash var for ColourPicker */
  wimp_menu            *current_menu;
  const choices_group  *group_menu;
  const choices_choice *choice_menu; /* which choice does it relate to? */
};

/* ----------------------------------------------------------------------- */

/* A set of groups of choices (e.g. all the app's choices) */
typedef struct choices choices;

struct choices
{
  const char              *app;     /* eg. 'PrivateEye' (keep <= 10 chars) */
  int                      ngroups;
  const choices_group    **groups;

  wimp_w                  *window;  /* pointer to window handle */
  wimp_w                  *current; /* pointer to current pane handle */
  wimp_i                   icon_set;
  wimp_i                   icon_cancel;
  wimp_i                   icon_save;

  choices_valbuf          *valbuf;
  choices_valbuf          *proposed_valbuf;
  size_t                   valbufsz;

  choices_vars            *vars;

  const choices_pane      *panes;
  int                      npanes;
};

/* ----------------------------------------------------------------------- */

error choices_init(void);
void choices_fin(void);
error choices_create_windows(const choices *);
void choices_destroy_windows(const choices *);
error choices_load(const choices *);
error choices_save(const choices *);
error choices_update_icons(const choices *);
error choices_open(const choices *);
error choices_set(const choices *);
error choices_cancel(const choices *);

#endif /* APPENGINE_CHOICES_H */
