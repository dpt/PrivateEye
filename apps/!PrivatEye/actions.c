/* --------------------------------------------------------------------------
 *    Name: actions.c
 * Purpose: Common actions
 * Version: $Id: actions.c,v 1.2 2009-05-20 21:38:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/help.h"
#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#include "actions.h"

error action_help(void)
{
  help_full_message_enable m;
  wimp_t                   t;

  /* I'm a bit worried about 'X' not being available everywhere. */
  xwimp_start_task("X <Help$Start>", &t);

  m.size     = sizeof(m);
  m.your_ref = 0;
  m.action   = message_HELP_ENABLE;
  m.flags    = 0;
  wimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &m, wimp_BROADCAST);

  return error_OK;
}

error action_close_window(wimp_w w)
{
  wimp_close close;

  close.w = w;

  wimp_send_message_to_window(wimp_CLOSE_WINDOW_REQUEST,
            (wimp_message *) &close,
                              close.w,
                              wimp_ICON_WINDOW);

  return error_OK;
}
