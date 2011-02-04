/* --------------------------------------------------------------------------
 *    Name: event.h
 * Purpose: Event library interface
 * ----------------------------------------------------------------------- */

/* TODO
 * - Fix insertion order. Presently we append to the end of the list. We
 *   should insert at the start.
 * - May need message handlers which know about window handles.
 * - Warn about unhandled events (somehow)
 */

#ifndef APPENGINE_EVENT_H
#define APPENGINE_EVENT_H

#include "oslib/wimp.h"

#define event_ANY_WINDOW  wimp_BACKGROUND
#define event_ANY_ICON    wimp_ICON_WINDOW

/* Return this if you've handled the event. */
#define event_HANDLED     1
/* Return this if you've not handled the event. */
#define event_NOT_HANDLED 0
/* Return this if you've handled the event and you want further claimants to
 * also see it. */
#define event_PASS_ON     0

/* ----------------------------------------------------------------------- */

void event_initialise(void);
void event_finalise(void);

/* ----------------------------------------------------------------------- */

void event_set_mask(wimp_poll_flags mask);

void event_zero_pollword(void);
void *event_get_pollword(void);

void event_set_interval(os_t t);

/* ----------------------------------------------------------------------- */

wimp_event_no event_poll(wimp_block *block);

/* ----------------------------------------------------------------------- */

int event_dispatch(wimp_event_no event_no, wimp_block *block);

/* ----------------------------------------------------------------------- */

/* Note that const is lost on the callback! */

typedef int (event_wimp_handler)(wimp_event_no event_no,
                                 wimp_block   *block,
                                 void         *handle);

typedef int (event_register_wimp_handler_prototype)(wimp_event_no       event_no,
                                                    wimp_w              w,
                                                    wimp_i              i,
                                                    event_wimp_handler *handler,
                                                    const void         *handle);

event_register_wimp_handler_prototype event_register_wimp_handler,
                                      event_deregister_wimp_handler;

int event_deregister_wimp_handlers_for_window(wimp_w w);

/* ----------------------------------------------------------------------- */

typedef int (event_message_handler)(wimp_message *message,
                                    void         *handle);

typedef int (event_register_message_handler_prototype)(bits                   msg_no,
                                                       wimp_w                 w,
                                                       wimp_i                 i,
                                                       event_message_handler *handler,
                                                       const void            *handle);

event_register_message_handler_prototype event_register_message_handler,
                                         event_deregister_message_handler;

/* ----------------------------------------------------------------------- */

typedef struct
{
  wimp_event_no       event_no;
  event_wimp_handler *handler;
}
event_wimp_handler_spec;

/* Utility function to register or deregister groups of Wimp handlers for the
 * same window/icon pair. */
/* reg - register or not. */
void event_register_wimp_group(int                            reg,
                               const event_wimp_handler_spec *specs,
                               int                            nspecs,
                               wimp_w                         w,
                               wimp_i                         i,
                               const void                    *handle);

typedef struct
{
  bits                   msg_no;
  event_message_handler *handler;
}
event_message_handler_spec;

/* Utility function to register or deregister groups of message handlers for
 * the same window/icon pair. */
/* reg - register or not. */
void event_register_message_group(int                               reg,
                                  const event_message_handler_spec *specs,
                                  int                               nspecs,
                                  wimp_w                            w,
                                  wimp_i                            i,
                                  const void                       *handle);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_EVENT_H */
