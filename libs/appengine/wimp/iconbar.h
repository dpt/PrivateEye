/* --------------------------------------------------------------------------
 *    Name: iconbar.h
 * Purpose: Declarations for the iconbar library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_ICONBAR_H
#define APPENGINE_ICONBAR_H

#include "oslib/wimp.h"

wimp_i iconbar_create_icon(const char    *sprite,
                           wimp_w         w,
                           wimp_priority  priority);

wimp_i iconbar_create_device_icon(const char    *sprite,
                                  const char    *text,
                                  wimp_w         w,
                                  wimp_priority  priority);

#endif /* APPENGINE_ICONBAR_H */
