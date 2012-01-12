/* --------------------------------------------------------------------------
 *    Name: dcs-quit.h
 * Purpose: Discard/Cancel/Save and Quit dialogues
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

error dcs_quit_init(void);
void dcs_quit_fin(void);
int dcs_quit_dcs_query(const char *message);
int dcs_quit_quit_query(const char *message, int count);

#endif /* APPENGINE_DIALOGUE_DCS_QUIT_H */
