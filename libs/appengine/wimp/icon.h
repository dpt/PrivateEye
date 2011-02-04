/* --------------------------------------------------------------------------
 *    Name: icon.h
 * Purpose: Declarations for the icon library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_ICON_H
#define APPENGINE_ICON_H

#include "oslib/os.h"
#include "oslib/wimp.h"

/* ----------------------------------------------------------------------- */

/* Position */

void icon_get_bbox(wimp_w w, wimp_i i, os_box *bbox);

void icon_get_screen_bbox(wimp_w w, wimp_i i, os_box *bbox);

/* Moves the specified icon to have its bottom left at x,y.
 * Size is retained. */
void move_icon(wimp_w w, wimp_i i, int x, int y);

/* Resizes the specified icon to have the specified width and height. */
void size_icon(wimp_w w, wimp_i i, int width, int height);

/* ----------------------------------------------------------------------- */

/* State */

#define icon_is_selected(_W,_I) \
    ((icon_get_flags(_W, _I) & wimp_ICON_SELECTED) != 0)

void icon_set_selected(wimp_w w, wimp_i i, int select);

wimp_icon_flags icon_get_flags(wimp_w w, wimp_i i);

void icon_set_flags(wimp_w w, wimp_i i,
                    wimp_icon_flags eor, wimp_icon_flags clear);

/* Sets the flags of a linear range of icons. */
void icon_range_set_flags(wimp_w w, wimp_i i_low, wimp_i i_high,
                          wimp_icon_flags eor, wimp_icon_flags clear);

/* Sets the flags of a list of specified icons. (e.g. for shading groups of
 * icons.) This is different than icon_range_set_flags in that the range need
 * not be contiguous. */
void icon_group_set_flags(wimp_w w, const wimp_i *icons, int nicons,
                          wimp_icon_flags eor, wimp_icon_flags clear);

/* Finds the first icon in the window definition which matches the flags. */
wimp_i icon_find(wimp_window_info *defn,
                 wimp_icon_flags mask, wimp_icon_flags want);

/* Sets the specified icon, unsetting any others with the same ESG. */
void icon_set_radio(wimp_w w, wimp_i i);

/* ----------------------------------------------------------------------- */

/* Content */

/* Sets the text of the icon to the specified string. */
void icon_set_text(wimp_w w, wimp_i i, const char *text);

/* Reads the text of the icon into the specified buffer. */
void icon_get_text(wimp_w w, wimp_i i, char *buffer);

int icon_get_int(wimp_w w, wimp_i i);

void icon_set_int(wimp_w w, wimp_i i, int value);

double icon_get_double(wimp_w w, wimp_i i);

void icon_set_double(wimp_w w, wimp_i i, double value, int places);

void icon_printf(wimp_w w, wimp_i i, const char *fmt, ...);

/* ----------------------------------------------------------------------- */

/* Validation */

void icon_set_validation(wimp_w w, wimp_i i, const char *validation);

void icon_validation_printf(wimp_w w, wimp_i i, const char *fmt, ...);

const char *icon_get_name(wimp_w w, wimp_i i);

/* Scans the validation string for an R<n> command and returns n or -1 if not
 * found. */
int icon_button_type(const char *validation);

/* Scans the validation string for an S<n> command and returns n. */
void icon_sprite_name(const char *validation, char *name);

/* ----------------------------------------------------------------------- */

/* Dragging */

/* Drags the specified icon. */
void drag_icon(wimp_w w, wimp_i i, int x, int y, const char *sprite);
/* Called on the UserDragBox event to terminate the drag. */
void drag_icon_stop(void);

/* As above, but uses DragAnObject to drag the rendered object. */
typedef void (drag_object_renderer)(void *arg);
void drag_object(wimp_w w, wimp_i i, int x, int y,
                 drag_object_renderer *render, void *args);
/* A variation of the above routine for use when there is no 'real' icon to
 * base the drag upon. */
void drag_object_box(wimp_w w, const os_box *box, int x, int y,
                     drag_object_renderer *render, void *args);
void drag_object_stop(void);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_ICON_H */
