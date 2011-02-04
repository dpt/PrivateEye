/* --------------------------------------------------------------------------
 *    Name: menu.h
 * Purpose: Declarations for the menu library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_MENU_H
#define APPENGINE_MENU_H

#include "oslib/wimp.h"

#define T wimp_menu

T *menu_create(const char *);

/* Destroy menus recursively. */
void menu_destroy(T *);

void menu_title(T **, const char *);

void menu_entry(T **, const char *);

void menu_set_menu_flags(T               *menu,
                         int              entry,
                         wimp_menu_flags  eor_bits,
                         wimp_menu_flags  clear_bits);

void menu_set_submenu(T *, int, void *);

void menu_set_icon_flags(T               *menu,
                         int              entry,
                         wimp_icon_flags  eor_bits,
                         wimp_icon_flags  clear_bits);

/* ticks a single entry, unticking all others */
void menu_tick_exclusive(T *, int entry_to_tick);

void menu_open(T *, int, int);

void menu_reopen(void);

T *menu_last(void);

void menu_popup(wimp_w w, wimp_i i, T *menu);

/* Shades all menu entries. */
void menu_shade_all(T *menu);

T *menu_create_from_desc(const char *desc, ...);

const char *menu_desc_name_from_sel(const char           *desc,
                                    const wimp_selection *sel);

void menu_range_tick_exclusive(T   *menu,
                               int  entry_to_tick,
                               int  low,
                               int  high);

#undef T

#endif /* APPENGINE_MENU_H */
