/* --------------------------------------------------------------------------
 *    Name: event-wimp.h
 * Purpose: Event library
 * Version: $Id: event-wimp.h,v 1.1 2009-05-20 20:58:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_EVENT_WIMP_H
#define APPENGINE_EVENT_WIMP_H

#include "oslib/wimp.h"

void event_finalise_wimp(void);
int event_dispatch_wimp(wimp_event_no event_no, wimp_block *block);
wimp_poll_flags event_wimp_get_mask(void);

#endif /* APPENGINE_EVENT_WIMP_H */
