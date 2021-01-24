/* --------------------------------------------------------------------------
 *    Name: treeview.h
 * Purpose: Tree view
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TREEVIEW_H
#define APPENGINE_TREEVIEW_H

#include "oslib/wimp.h"

#include "datastruct/ntree.h"

#include "appengine/base/errors.h"

#define T treeview_t

typedef struct T T;

error treeview_init(void);
void treeview_fin(void);

error treeview_create(T **tr);
void treeview_destroy(T *tr);

error treeview_draw(T *tr);

error treeview_set_tree(T *tr, ntree_t *tree);

/* Make all nodes with children collapsible. */
error treeview_make_collapsible(T *tr);

/* Sends a click event to the treeview.
 * Returns redraw_y <= 0 (-ve or zero) if the tree needs redrawing. */
error treeview_click(T *tr, int x, int y, int *redraw_y);

/* Sets the required width of text, in characters. */
void treeview_set_text_width(T *tr, int width);

/* Sets the OS unit height of text use when painting. */
void treeview_set_line_height(T *tr, int line_height);

/* Returns the dimensions of a treeview in OS units. */
error treeview_get_dimensions(T *tr, int *width, int *height);

void treeview_set_highlight_background(T *tr, wimp_colour bgcolour);

typedef unsigned int treeview_mark;

#define treeview_mark_COLLAPSE (1u << 0)
#define treeview_mark_EXPAND   (1u << 1)

/* Marks all nodes collapsed/expanded. */
void treeview_mark_all(T *tr, treeview_mark mark);

#undef T

#endif /* APPENGINE_TREEVIEW_H */
