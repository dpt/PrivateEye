/* --------------------------------------------------------------------------
 *    Name: scroll-list.h
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_SCROLL_LIST_H
#define APPENGINE_SCROLL_LIST_H

#include "oslib/os.h"
#include "oslib/wimp.h"

#define T scroll_list

typedef struct T T;

#define scroll_list_SELECTION_CHANGED (0)
#define scroll_list_DRAG              (1)
#define scroll_list_DELETE            (2)

typedef unsigned int scroll_list_event_type;

typedef struct
{
  scroll_list_event_type type;
  int                    index;
  union
  {
    struct
    {
      wimp_pointer      *pointer;
      os_box             box;
    }
    drag;
  }
  data;
}
scroll_list_event;

/* Creates a new scroll list. The handles given identify the window and icon
 * in which to place the scrolling list. */
T *scroll_list_create(wimp_w w, wimp_i i);

void scroll_list_destroy(T *sl);

/* x and y are work area coordinates. */
typedef void (scroll_list_redrawfn)(wimp_draw *redraw,
                                    int        x,
                                    int        y,
                                    int        i,
                                    int        sel);

typedef void (scroll_list_eventfn)(scroll_list_event *event);

void scroll_list_set_handlers(T                    *sl,
                              scroll_list_redrawfn *redraw_elem,
                              scroll_list_redrawfn *redraw_lead,
                              scroll_list_eventfn  *event);

wimp_w scroll_list_get_window_handle(T *sl);

void scroll_list_set_row_height(T  *sl,
                                int height,
                                int leading);

void scroll_list_refresh_row(T *sl, int row);
void scroll_list_refresh_all_rows(T *sl);

void scroll_list_add_row(T *sl);
void scroll_list_delete_rows(T *sl, int min, int max);

int scroll_list_which(T *sl, wimp_pointer *pointer);
int scroll_list_where_to_insert(T *sl, wimp_pointer *pointer);

void scroll_list_get_bbox(T *sl, int row, os_box *box);

/* Scrolls the pane so that the specified row is visible. */
void scroll_list_make_visible(T *sl, int row);

void scroll_list_set_selection(T *sl, int row);
int scroll_list_get_selection(T *sl);
void scroll_list_clear_selection(T *sl);
/* Moves the selection to 'where'. Returns the new selection. */
int scroll_list_move_selection_absolute(T *sl, int where);
/* Moves the selection by 'delta'. Returns the new selection. */
int scroll_list_move_selection_relative(T *sl, int delta);

void scroll_list_set_marker(T *sl, int where);
void scroll_list_clear_marker(T *sl);

void scroll_list_autoscroll(T *sl, int on);

#undef T

#endif /* APPENGINE_SCROLL_LIST_H */
