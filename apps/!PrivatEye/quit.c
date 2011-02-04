/* --------------------------------------------------------------------------
 *    Name: quit.c
 * Purpose: Quit
 * ----------------------------------------------------------------------- */

#include "appengine/dialogues/dcs-quit.h"

#include "viewer.h"

#include "quit.h"

int can_quit(void)
{
  int count;

  count = viewer_count_edited();
  if (count == 0)
    return 1;

  return dcs_quit__quit_query("quit.modified", count) == dcs_quit_DISCARD;
}
