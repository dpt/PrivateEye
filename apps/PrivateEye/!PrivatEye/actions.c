/* --------------------------------------------------------------------------
 *    Name: actions.c
 * Purpose: Common actions
 * ----------------------------------------------------------------------- */

#include "oslib/help.h"
#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#include "actions.h"

result_t action_help(void)
{
  help_full_message_enable m;
  wimp_t                   t;

  /* FIXME: I'm a bit worried that 'X' won't be available on all systems,
   * though it should be in any 'modern' boot sequence. */
  xwimp_start_task("X <Help$Start>", &t);

  m.size     = sizeof(m);
  m.your_ref = 0;
  m.action   = message_HELP_ENABLE;
  m.flags    = 0;
  wimp_send_message(wimp_USER_MESSAGE, (wimp_message *) &m, wimp_BROADCAST);

  return result_OK;
}

result_t action_close_window(wimp_w w)
{
  wimp_close close;

  close.w = w;

  wimp_send_message_to_window(wimp_CLOSE_WINDOW_REQUEST,
            (wimp_message *) &close,
                              close.w,
                              wimp_ICON_WINDOW);

  return result_OK;
}
