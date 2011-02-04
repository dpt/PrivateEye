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

dialogue_t *proginfo__create(void)
{
  proginfo_t *s;
  dialogue_t *d;
  wimp_w      w;

  s = calloc(1, sizeof(*s));
  if (s == NULL)
    return NULL;

  d = &s->info.dialogue;

  info__construct(&s->info, "prog_info");

  w = dialogue__get_window(d);

  icon_set_text(w, proginfo__VERSION_ICON, message0("version"));

  info__layout(d);

  return d;
}

void proginfo__destroy(dialogue_t *d)
{
  proginfo_t *s;

  s = (proginfo_t *) d;

  info__destruct(&s->info);

  free(d);
}
