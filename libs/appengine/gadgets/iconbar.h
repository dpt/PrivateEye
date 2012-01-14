/* --------------------------------------------------------------------------
 *    Name: iconbar.h
 * Purpose: Standard icon bar icon
 * ----------------------------------------------------------------------- */

#ifndef ICONBAR_H
#define ICONBAR_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

error icon_bar_init(void);
void icon_bar_fin(void);

typedef void (icon_bar_menu_pointerfn)(const wimp_pointer *pointer,
                                       void               *opaque);
typedef void (icon_bar_menu_selectionfn)(const wimp_selection *selection,
                                         void                 *opaque);
typedef void (icon_bar_menu_updatefn)(wimp_menu *menu, void *opaque);

void icon_bar_set_handlers(icon_bar_menu_pointerfn   *pointer,
                           icon_bar_menu_selectionfn *select,
                           icon_bar_menu_updatefn    *update,
                           void                      *opaque);

#endif /* ICONBAR_H */
