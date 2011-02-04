/* --------------------------------------------------------------------------
 *    Name: window.h
 * Purpose: Declarations for the window library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_WINDOW_H
#define APPENGINE_WINDOW_H

#include "oslib/types.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "appengine/base/oserror.h" /* for EC() */

/* Windows */

void window_use_fonts(byte *);
void window_lose_fonts(void);
void window_set_font_array(byte *);
byte *window_get_font_array(void);

void window_load_sprites(const char *filename);
void window_set_sprite_area(osspriteop_area *area);
osspriteop_area *window_get_sprite_area(void);

int window_find_template_size(const char *template_name,
                              int        *window_size,
                              int        *indirected_size);
int window_load_template(const char  *template_name,
                         wimp_window *window_block,
                         char        *indirected_block,
                         int          indirected_size);
wimp_w window_create(const char *);
wimp_w window_create_from_memory(wimp_window *);
wimp_w window_clone(wimp_w);
void window_delete_cloned(wimp_w);

typedef unsigned int window_open_at_flags;

enum
{
  AT_DEF,    /* default        */
  AT_CEN,    /* centre         */
  AT_LEFTOP, /* left  | top    */
  AT_RIGBOT, /* right | bottom */
  AT_PTR,    /* pointer        */
  AT_STG,    /* stagger        */

  AT_V     = 0,
  AT_H     = 3,

  AT_VMASK = 0x07,
  AT_HMASK = 0x38
};

/* 00000000 000000000 000000000 F0VVVHHH */

enum
{
  AT_DEFAULT            = (AT_DEF    << AT_V) | (AT_DEF    << AT_H),

  AT_TOPLEFT            = (AT_LEFTOP << AT_V) | (AT_LEFTOP << AT_H),
  AT_TOPCENTRE          = (AT_LEFTOP << AT_V) | (AT_CEN    << AT_H),
  AT_TOPRIGHT           = (AT_LEFTOP << AT_V) | (AT_RIGBOT << AT_H),

  AT_CENTRELEFT         = (AT_CEN    << AT_V) | (AT_LEFTOP << AT_H),
  AT_CENTRE             = (AT_CEN    << AT_V) | (AT_CEN    << AT_H),
  AT_CENTRERIGHT        = (AT_CEN    << AT_V) | (AT_RIGBOT << AT_H),

  AT_BOTTOMLEFT         = (AT_RIGBOT << AT_V) | (AT_LEFTOP << AT_H),
  AT_BOTTOMCENTRE       = (AT_RIGBOT << AT_V) | (AT_CEN    << AT_H),
  AT_BOTTOMRIGHT        = (AT_RIGBOT << AT_V) | (AT_RIGBOT << AT_H),

  AT_BOTTOMPOINTER      = (AT_RIGBOT << AT_V) | (AT_PTR    << AT_H),

  AT_STAGGER            = (AT_STG    << AT_V) | (AT_STG    << AT_H),

  AT_FORCE              = 1 << 7, /* flag: force re-open */
  AT_NOCOVERICONBAR     = 1 << 8, /* flag: avoid covering icon bar */
  AT_USEVISIBLEAREA     = 1 << 9, /* flag: use visible area, not extent */
};

void window_open_at(wimp_w w, window_open_at_flags where);
#define window_open(window) window_open_at(window, AT_DEFAULT)

void window_open_here_flags(wimp_w                w,
                      const os_box               *box,
                            window_open_at_flags  flags);
#define window_open_here(window) window_open_here_flags(window,0)

void window_open_as_menu(wimp_w w);
void window_open_as_menu_here(wimp_w w, int x, int y);
#define window_close(_w) wimp_close_window(_w)
#define window_set_extent(h,x0,y0,x1,x2) ((void)window_set_extent2(h,x0,y0,x1,x2))
int window_set_extent2(wimp_w w, int xmin, int ymin, int xmax, int ymax);

/* Slightly odd names to avoid a clash with OSLib's Toolbox methods. */
char *window_get_title_text(wimp_w w);
void window_set_title_text(wimp_w w, const char *text);

void window_redraw(wimp_w w);

/* Icon bar metrics */
enum
{
  IconBarHeight  = 134,
  IconBarOverlap = 14,  /* same overlap the Wimp allows */
  IconBarVisible = IconBarHeight - IconBarOverlap
};

/* Reads the window furniture dimensions */
void read_furniture_dimensions(wimp_w w, os_box *furn);

/* Reads the maximum visible area, in OS units, that the specified window
 * could have given the current screen resolution */
void read_max_visible_area(wimp_w win, int *w, int *h);

/* Two-part window position/scale preservation thing */
typedef struct
{
  int               cx, cy; /* centre */
  wimp_window_flags flags;
}
WindowCapture;
void window_capture(wimp_w w, WindowCapture *capture, osbool covericonbar);
void window_restore(wimp_w w, WindowCapture *capture, osbool covericonbar);

wimp_window_info *window_get_defn(wimp_w w);

/* Sets the extent of a window which is used in a submenu. flags is a
 * bitmask of which elements of box to use. */
void window_set_submenu_extent(wimp_w w, unsigned int flags, os_box *box);

/* Templates */

#define templates_open(filename) EC(_swix(Wimp_OpenTemplate,_IN(1),filename))
#define templates_close() EC(_swix(Wimp_CloseTemplate,0))

#endif /* APPENGINE_WINDOW_H */
