/* --------------------------------------------------------------------------
 *    Name: iconbar.h
 * Purpose: Standard icon bar icon
 * ----------------------------------------------------------------------- */

#ifndef ICONBAR_H
#define ICONBAR_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

error icon_bar__init(void);
void icon_bar__fin(void);

typedef void (icon_bar__menu_pointerfn)(const wimp_pointer *pointer,
                                        void               *opaque);
typedef void (icon_bar__menu_selectionfn)(const wimp_selection *selection,
                                          void                 *opaque);
typedef void (icon_bar__menu_updatefn)(wimp_menu *menu, void *opaque);

void icon_bar__set_handlers(icon_bar__menu_pointerfn   *pointer,
                            icon_bar__menu_selectionfn *select,
                            icon_bar__menu_updatefn    *update,
                            void                       *opaque);

#endif /* ICONBAR_H */
