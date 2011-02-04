/* --------------------------------------------------------------------------
 *    Name: event-wimp-group.c
 * Purpose: Event library
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/event.h"

void event_register_wimp_group(int                            reg,
                               const event_wimp_handler_spec *specs,
                               int                            nspecs,
                               wimp_w                         w,
                               wimp_i                         i,
                               const void                    *handle)
{
  event_register_wimp_handler_prototype *wimp_fn;
  int j;

  wimp_fn = (reg) ? event_register_wimp_handler :
                    event_deregister_wimp_handler;

  for (j = 0; j < nspecs; j++)
    wimp_fn(specs[j].event_no, w, i, specs[j].handler, handle);
}
