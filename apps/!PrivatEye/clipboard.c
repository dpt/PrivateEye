/* --------------------------------------------------------------------------
 *    Name: clipboard.c
 * Purpose: Clipboard
 * Version: $Id: clipboard.c,v 1.4 2007-01-30 00:44:08 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/wimp.h"

#include "globals.h"
#include "viewer.h"
#include "clipboard.h"

void clipboard_claim(wimp_w w)
{
  wimp_message message;

  /* We always broadcast this, even if we already own the clipboard (see
   * RISCOS Ltd. Clipboard TechNote). */

  /* Claim the clipboard, even if we already own it */
  message.size     = sizeof(wimp_full_message_claim_entity);
  message.your_ref = 0;
  message.action   = message_CLAIM_ENTITY;
  message.data.claim_entity.flags = wimp_CLAIM_CLIPBOARD;
  wimp_send_message(wimp_USER_MESSAGE, &message, wimp_BROADCAST);

  GLOBALS.clipboard_viewer = viewer_find(w);
}

void clipboard_release(void)
{
  GLOBALS.clipboard_viewer = NULL;
}

osbool clipboard_own(void)
{
  return GLOBALS.clipboard_viewer != NULL;
}
