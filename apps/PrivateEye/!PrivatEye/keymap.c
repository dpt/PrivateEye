/* --------------------------------------------------------------------------
 *    Name: keymap.c
 * Purpose: Keymap
 * ----------------------------------------------------------------------- */

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/app/keymap.h"
#include "appengine/app/wire.h"

#include "actions.h"
#include "privateeye.h"

#include "keymap.h"

/* ----------------------------------------------------------------------- */

typedef struct
{
  keymap_section *sections;
  int             nsections;
  int             allocated;
}
viewer_keymap_section_array;

static struct
{
  viewer_keymap_section_array array;
  int                         id; /* counter for ids we assign */
}
LOCALS;

static keymap_t *keymap;

static unsigned int viewer_keymap_refcount = 0;

/* ----------------------------------------------------------------------- */

/* Keep these sorted by name */
static const keymap_name_to_action common[] =
{
  { "Close", Close },
  { "Help",  Help  },
};

/* ----------------------------------------------------------------------- */

result_t viewer_keymap_init(void)
{
  result_t err;

  if (viewer_keymap_refcount == 0)
  {
    wire_message_t m;

    /* initialise */

    /* add our mappings first - guaranteeing an id of zero */

    viewer_keymap_add("Common", common, NELEMS(common), NULL);

    /* invite all interested components to declare their mappings now */

    m.event   = wire_event_DECLARE_KEYMAP;
    m.payload = NULL;
    wire_dispatch(&m);

    /* parse the keymap */

    err = keymap_create("Choices:" APPNAME ".Keys",
                        LOCALS.array.sections,
                        LOCALS.array.nsections,
                       &keymap);
    if (err)
      return err;
  }

  viewer_keymap_refcount++;

  return result_OK;
}

void viewer_keymap_fin(void)
{
  if (viewer_keymap_refcount == 0)
    return;

  if (viewer_keymap_refcount == 1)
  {
    /* finalise */

    keymap_destroy(keymap);
  }

  viewer_keymap_refcount--;
}

result_t viewer_keymap_add(const char                  *name,
                           const keymap_name_to_action *mappings,
                           int                          nmappings,
                           viewer_keymap_id            *id)
{
  viewer_keymap_section_array *array = &LOCALS.array;
  keymap_section              *section;

  section = array->sections + array->nsections;
  if (section == array->sections + array->allocated)
  {
    int             n;
    keymap_section *newsections;

    /* doubling strategy */

    n = array->allocated * 2;
    if (n < 8)
      n = 8;

    newsections = realloc(array->sections, sizeof(*array->sections) * n);
    if (newsections == NULL)
      return result_OOM;

    array->sections  = newsections;
    array->allocated = n;

    section = array->sections + array->nsections;
  }

  section->name      = name;
  section->mappings  = mappings;
  section->nmappings = nmappings;

  array->nsections++;

  if (id)
    *id = LOCALS.id;

  LOCALS.id++;

  return result_OK;
}

int viewer_keymap_op(viewer_keymap_id id, wimp_key_no key_no)
{
  keymap_action act;

  act = keymap_get_action(keymap, id, key_no);
  if (act >= 0 || id == 0) // common id
    return act;

  return viewer_keymap_op(0, key_no);
}
