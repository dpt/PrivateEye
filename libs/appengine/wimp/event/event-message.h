/* --------------------------------------------------------------------------
 *    Name: event-message.h
 * Purpose: Event library
 * Version: $Id: event-message.h,v 1.1 2009-05-20 20:58:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_EVENT_MESSAGE_H
#define APPENGINE_EVENT_MESSAGE_H

#include "oslib/wimp.h"

int event_initialise_message(void);
void event_finalise_message(void);
int event_dispatch_message(wimp_block *block);

#endif /* APPENGINE_EVENT_MESSAGE_H */
