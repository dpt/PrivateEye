/* --------------------------------------------------------------------------
 *    Name: dcs-quit.h
 * Purpose: Discard/Cancel/Save and Quit dialogues
 * Version: $Id: dcs-quit.h,v 1.2 2009-05-18 22:07:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_DCS_QUIT_H
#define APPENGINE_DIALOGUE_DCS_QUIT_H

#include "appengine/base/errors.h"

enum
{
  dcs_quit_DISCARD = 0,
  dcs_quit_CANCEL,
  dcs_quit_SAVE
};

error dcs_quit__init(void);
void dcs_quit__fin(void);
int dcs_quit__dcs_query(const char *message);
int dcs_quit__quit_query(const char *message, int count);

#endif /* APPENGINE_DIALOGUE_DCS_QUIT_H */
