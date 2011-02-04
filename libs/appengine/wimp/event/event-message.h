/* --------------------------------------------------------------------------
 *    Name: event-message.h
 * Purpose: Event library
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_EVENT_MESSAGE_H
#define APPENGINE_EVENT_MESSAGE_H

#include "oslib/wimp.h"

int event_initialise_message(void);
void event_finalise_message(void);
int event_dispatch_message(wimp_block *block);

#endif /* APPENGINE_EVENT_MESSAGE_H */
