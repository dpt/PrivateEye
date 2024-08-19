/* --------------------------------------------------------------------------
 *    Name: choices.h
 * Purpose: Choices library interface
 * ----------------------------------------------------------------------- */

/* Overview:
 *
 * The choices library manages an application's choices (settings or
 * preferences) window. The application supplies a choices structure
 * hierarchy and from that this library provides the UI and the loading and
 * saving of those values.
 *
 * Individual choices can be colours, number ranges, options or string sets.
 *
 * Structure:
 *
 * - The root choices struct has sets of choices_groups and choices_panes
 * - Each choices_group has a set of choices_choices (the actual settings)
 * - Each choices_group associates with a single choices_pane
 *
 * This allows more than one group of choices to share a single window pane.
 */

#ifndef APPENGINE_CHOICES_H
#define APPENGINE_CHOICES_H

#include <stddef.h>

#include "appengine/base/errors.h"

#include "oslib/wimp.h"

typedef void choices_valbuf;

/* ----------------------------------------------------------------------- */

/* The type of an individual choice. */
typedef enum choices_type
{
  choices_TYPE_COLOUR,
  choices_TYPE_NUMBER_RANGE,
  choices_TYPE_OPTION,
  choices_TYPE_STRING_SET,
  choices_TYPE__LIMIT
}
choices_type;

/* The value associated with a stringset item. */
typedef struct choices_stringset_vals
{
  int val;
}
choices_stringset_vals;

/* A menu of items, of which one can be selected from a pop-up menu. */
typedef struct choices_stringset
{
  const char                   *name; /* name of menu in messages file */
  wimp_i                        icon_display, icon_popup;
  int                           nelems;
  const choices_stringset_vals *elems;
}
choices_stringset;

/* A bumpable integer clamped to min..max shown with specified precision. */
typedef struct choices_numberrange
{
  wimp_i icon_display, icon_down, icon_up;
  int    min, max;
  int    inc;  /* increment */
  int    prec; /* display precision (e.g. 2 for 2dp) */
}
choices_numberrange;

/* An on/off option. */
typedef struct choices_option
{
  wimp_i icon;
}
choices_option;

/* A colour. */
typedef struct choices_colour
{
  wimp_i icon;
}
choices_colour;

/* A single choice. */
typedef struct choices_choice
{
  const char                  *name;
  int                          offset;
  choices_type                 type;
  int                          defaultval;
  union
  {
    const void                *ui; /* used when no visible UI */
    const choices_colour      *colour;
    const choices_numberrange *number_range;
    const choices_option      *option;
    const choices_stringset   *string_set;
  }
  data;
}
choices_choice;

/* ----------------------------------------------------------------------- */

/* The window pane associated with a single radio button. */
typedef struct choices_pane choices_pane;

typedef result_t (choices_pane_initialise_handler)(const choices_pane *);
typedef void (choices_pane_finalise_handler)(const choices_pane *);
typedef result_t (choices_pane_changed_handler)(const choices_pane *);
typedef result_t (choices_pane_redraw_handler)(const choices_pane *,
                                               wimp_draw *);

/* These handlers deal with proposed choices. */
typedef struct choices_pane_handlers
{
  choices_pane_initialise_handler *initialise_callback;
  choices_pane_finalise_handler   *finalise_callback;
  /* Called when the proposed choices are changed. */
  choices_pane_changed_handler    *changed_callback;
  choices_pane_redraw_handler     *redraw_callback;
}
choices_pane_handlers;

struct choices_pane
{
  wimp_w                      *window;  /* pointer to window handle */
  const char                   name[4]; /* short identifier, eg. 'vwr' */
  wimp_i                       icon;    /* radio icon in main window */
  const choices_pane_handlers *handlers;
};

/* ----------------------------------------------------------------------- */

/* A grouping of choices. */
typedef struct choices_group choices_group;

typedef result_t (choices_group_changed_handler)(const choices_group *);

typedef struct choices_group_handlers
{
  /* Called when the choices are set and the group has had changes. */
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

/* Since everything else is const, keep the variables self-contained. */
typedef struct choices_vars choices_vars;

struct choices_vars
{
  unsigned int         *temporary_colour; /* stash var for ColourPicker */
  wimp_menu            *current_menu;
  const choices_group  *group_menu;
  const choices_choice *choice_menu; /* the choice an open menu belongs to */
};

/* ----------------------------------------------------------------------- */

/* A set of groups of choices (e.g. all the app's choices) */
typedef struct choices choices;

struct choices
{
  const char              *app;     /* eg. 'PrivateEye' (keep <= 10 chars) */

  int                      ngroups;
  const choices_group    **groups;

  wimp_w                  *window;  /* pointer to choices window handle */
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

result_t choices_init(void);
void choices_fin(void);
result_t choices_create_windows(const choices *);
void choices_destroy_windows(const choices *);
result_t choices_load(const choices *);
result_t choices_open(const choices *);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_CHOICES_H */
