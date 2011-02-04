/* --------------------------------------------------------------------------
 *    Name: event-message-group.c
 * Purpose: Event library
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/event.h"

void event_register_message_group(int                               reg,
                                  const event_message_handler_spec *specs,
                                  int                               nspecs,
                                  wimp_w                            w,
                                  wimp_i                            i,
                                  const void                       *handle)
{
  event_register_message_handler_prototype *message_fn;
  int j;

  message_fn = (reg) ? event_register_message_handler :
                       event_deregister_message_handler;

  for (j = 0; j < nspecs; j++)
    message_fn(specs[j].msg_no, w, i, specs[j].handler, handle);
}
