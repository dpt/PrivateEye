/* --------------------------------------------------------------------------
 *    Name: treeview.h
 * Purpose: Tree view
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TREEVIEW_H
#define APPENGINE_TREEVIEW_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"
#include "appengine/datastruct/ntree.h"

#define T treeview_t

typedef struct T T;

error treeview__init(void);
void treeview__fin(void);

error treeview__create(T **tr);
void treeview__destroy(T *tr);

error treeview__draw(T *tr);

error treeview__set_tree(T *tr, ntree_t *tree);

/* Make all nodes with children collapsible. */
error treeview__make_collapsible(T *tr);

/* Sends a click event to the treeview.
 * Returns redraw_y <= 0 (-ve or zero) if the tree needs redrawing. */
error treeview__click(T *tr, int x, int y, int *redraw_y);

/* Sets the required width of text, in characters. */
void treeview__set_text_width(T *tr, int width);

/* Sets the OS unit height of text use when painting. */
void treeview__set_line_height(T *tr, int line_height);

/* Returns the dimensions of a treeview in OS units. */
error treeview__get_dimensions(T *tr, int *width, int *height);

void treeview__set_highlight_background(T *tr, wimp_colour bgcolour);

typedef unsigned int treeview__mark;

#define treeview__mark_COLLAPSE (1u << 0)
#define treeview__mark_EXPAND   (1u << 1)

/* Marks all nodes collapsed/expanded. */
void treeview__mark_all(T *tr, treeview__mark mark);

#undef T

#endif /* APPENGINE_TREEVIEW_H */
