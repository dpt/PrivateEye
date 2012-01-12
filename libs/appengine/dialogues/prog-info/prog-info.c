/* --------------------------------------------------------------------------
 *    Name: prog-info.c
 * Purpose: ProgInfo window
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "appengine/wimp/icon.h"
#include "appengine/dialogues/info.h"
#include "appengine/base/messages.h"

#include "appengine/dialogues/prog-info.h"

typedef struct proginfo_t
{
  info_t info; /* base class */
}
proginfo_t;

dialogue_t *proginfo_create(void)
{
  proginfo_t *s;
  dialogue_t *d;
  wimp_w      w;

  s = calloc(1, sizeof(*s));
  if (s == NULL)
    return NULL;

  d = &s->info.dialogue;

  info_construct(&s->info, "prog_info");

  w = dialogue_get_window(d);

  icon_set_text(w, proginfo_VERSION_ICON, message0("version"));

  info_layout(d);

  return d;
}

void proginfo_destroy(dialogue_t *d)
{
  proginfo_t *s;

  s = (proginfo_t *) d;

  info_destruct(&s->info);

  free(d);
}
