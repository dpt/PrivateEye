/* --------------------------------------------------------------------------
 *    Name: keymap.c
 * Purpose: Allows clients to offer customisable keymaps
 * ----------------------------------------------------------------------- */

/* TODO
 *
 * Should entries be case sensitive? Configurable per client?
 * How to set default values?
 */

/*
# problem cases
#
# I:xxx       ok
# i:xxx       ok
# S_I:xxx     can't have shifted versions of ordinary characters (S_I == I)
# S_i:xxx     can't have shifted versions of ordinary characters (S_i == I)
# C_I:xxx     ok
# C_i         can't have ctrl+lowercase
# SC_I:xxx    can't have shift+ctrl+char
# SC_i:xxx    can't have shift+ctrl+lowercase
*/

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "oslib/types.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/bsearch.h"
#include "appengine/base/errors.h"

#include "appengine/app/keymap.h"

/* ----------------------------------------------------------------------- */

static int name_to_action_compare(const void *a, const void *b)
{
  const keymap_name_to_action *c = a;
  const keymap_name_to_action *d = b;

  return strcmp(c->name, d->name);
}

/* Searches the table the client passed in to find out what number should
 * be returned for the specified string. */
static keymap_action name_to_action(const keymap_name_to_action *map,
                                    int                          nelems,
                                    const char                  *s)
{
  const keymap_name_to_action *m;
  keymap_name_to_action        fake;

  /* have to bung it in a fake object for the compare function */
  fake.name   = s;
  fake.action = -1;

  m = bsearch(&fake, map, nelems, sizeof(*map), name_to_action_compare);
  if (m)
    return m->action;

  return -1;
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  const char  *name;
  wimp_key_no  no;
}
keymap_keyname_to_keynum;

static const keymap_keyname_to_keynum keys[] =
{
  { "Backspace",        wimp_KEY_BACKSPACE                      },
  { "Copy",             wimp_KEY_COPY                           },
  { "Delete",           wimp_KEY_DELETE                         },
  { "Down",             wimp_KEY_DOWN                           },
  { "Escape",           wimp_KEY_ESCAPE                         },
  { "F1",               wimp_KEY_F1                             },
  { "F10",              wimp_KEY_F10                            },
  { "F11",              wimp_KEY_F11                            },
  { "F12",              wimp_KEY_F12                            },
  { "F2",               wimp_KEY_F2                             },
  { "F3",               wimp_KEY_F3                             },
  { "F4",               wimp_KEY_F4                             },
  { "F5",               wimp_KEY_F5                             },
  { "F6",               wimp_KEY_F6                             },
  { "F7",               wimp_KEY_F7                             },
  { "F8",               wimp_KEY_F8                             },
  { "F9",               wimp_KEY_F9                             },
  { "Home",             wimp_KEY_HOME                           },
  { "Insert",           wimp_KEY_INSERT                         },
  { "Left",             wimp_KEY_LEFT                           },
  { "Logo",             wimp_KEY_LOGO                           },
  { "Menu",             wimp_KEY_MENU                           },
  { "PageDown",         wimp_KEY_PAGE_DOWN                      },
  { "PageUp",           wimp_KEY_PAGE_UP                        },
  { "Print",            wimp_KEY_PRINT                          },
  { "Return",           wimp_KEY_RETURN                         },
  { "Right",            wimp_KEY_RIGHT                          },
  { "Space",            ' '                                     },
  { "Tab",              wimp_KEY_TAB                            },
  { "Up",               wimp_KEY_UP                             },
};

static const keymap_keyname_to_keynum modifiers[] =
{
  { "C",                wimp_KEY_CONTROL                        },
  { "S",                wimp_KEY_SHIFT                          },
  { "SC",               wimp_KEY_CONTROL | wimp_KEY_SHIFT       },
};

static int keyname_to_keynum_compare(const void *a, const void *b)
{
  const keymap_keyname_to_keynum *c = a;
  const keymap_keyname_to_keynum *d = b;

  return strcmp(c->name, d->name);
}

static wimp_key_no keyname_to_keynum(const char                     *s,
                                     const keymap_keyname_to_keynum *tab,
                                     int                             nelems)
{
  keymap_keyname_to_keynum *m;
  keymap_keyname_to_keynum  fake;

  /* have to bung it in a fake object for the compare function */
  fake.name = s;
  fake.no   = -1;

  m = bsearch(&fake, tab, nelems, sizeof(*tab),
               keyname_to_keynum_compare);
  if (m)
    return m->no;

  /* else ... something not in the table like 'A' */

  if (s[1] == '\0')
    return s[0]; /* we only understand single character codes */

  return 0;
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  wimp_key_no                   key;
  keymap_action                 action;
}
keymap_key_to_action;

typedef struct
{
  keymap_key_to_action         *map;
  int                           allocated;
  int                           used;
}
keymap_section2;

struct keymap_t
{
  int                           nsections;
  keymap_section2               sections[UNKNOWN];
};

static error parse_line(const keymap_section *section,
                        char                 *buf,
                        keymap_key_to_action *mapping)
{
  char          *delimiter;
  char          *key;
  char          *act;
  wimp_key_no    modifier_key_no;
  wimp_key_no    key_no;
  keymap_action  action;

  delimiter = strchr(buf, ':');
  if (delimiter == NULL)
    goto SyntaxError;

  *delimiter = '\0';

  key = buf;
  act = delimiter + 1;

  delimiter = strchr(buf, '_');
  if (delimiter)
  {
    /* parse the modifier */

    *delimiter = '\0';

    modifier_key_no = keyname_to_keynum(key, modifiers, NELEMS(modifiers));
    if (modifier_key_no == 0)
      goto UnknownModifier;

    key = delimiter + 1;
  }
  else
  {
    modifier_key_no = 0;
  }

  key_no = keyname_to_keynum(key, keys, NELEMS(keys));
  if (key_no == 0)
    goto SyntaxError;

  if (key_no >= 0x100) /* a wimp special key */
  {
    key_no |= modifier_key_no; /* can take any modifier */
  }
  else
  {
    switch (modifier_key_no)
    {
    default:
      break;

    case wimp_KEY_CONTROL:
      if (key_no >= '@' && key_no <= '_')
        key_no -= 64; /* Ctrl+<X> */
      else
        goto BadKeySpec;
      break;

    case wimp_KEY_SHIFT:
    case wimp_KEY_CONTROL | wimp_KEY_SHIFT:
      goto BadKeySpec;
    }
  }

  action = name_to_action(section->mappings, section->nmappings, act);
  if (action == -1)
    goto UnknownAction;

  mapping->key    = key_no;
  mapping->action = action;

  return error_OK;


SyntaxError:

  return error_KEYMAP_SYNTAX_ERROR;


UnknownModifier:

  return error_KEYMAP_UNKNOWN_MODIFIER;


UnknownAction:

  return error_KEYMAP_UNKNOWN_ACTION;


BadKeySpec:

  return error_KEYMAP_BAD_KEY;
}

static int key_to_action_compare(const void *a, const void *b)
{
  const keymap_key_to_action *c = a;
  const keymap_key_to_action *d = b;

  if (c->key < d->key) return -1;
  if (c->key > d->key) return 1;

  return 0;
}

error keymap__create(const char           *filename,
                     const keymap_section *sections,
                     int                   nsections,
                     keymap_t            **keymap_out)
{
  error                 err;
  FILE                 *f;
  keymap_t             *keymap;
  int                   i;
  char                  buf[256];
  const keymap_section *input_section;
  keymap_section2      *output_section;
  keymap_section2      *s;

  f = fopen(filename, "r");
  if (f == NULL)
    return error_KEYMAP_NOT_FOUND;

  keymap = malloc(offsetof(keymap_t, sections) +
                  sizeof(keymap_section2) * nsections);
  if (keymap == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  keymap->nsections = nsections;

  for (i = 0; i < nsections; i++)
  {
    keymap->sections[i].map       = NULL;
    keymap->sections[i].allocated = 0;
    keymap->sections[i].used      = 0;
  }

  input_section  = NULL;
  output_section = NULL;

  while (fgets(buf, sizeof(buf), f))
  {
    int len;

    len = strlen(buf);

    /* strip newlines, etc. */

    while (isspace(buf[len - 1]))
      len--;
    buf[len] = '\0'; /* we always strip at least \n so this should be safe */

    /* skip "empty" lines (we need >= 3 chars: 'X:X' or '[X]') */

    if (len < 3)
      continue;

    /* skip comments */

    if (buf[0] == '#')
      continue;

    /* detect sections */

    if (buf[0] == '[' && buf[len - 1] == ']')
    {
      int sectionlen;

      sectionlen = len - 2; /* exclude square brackets */

      for (i = 0; i < nsections; i++)
      {
        int namelen;

        namelen = strlen(sections[i].name);

        if (namelen == sectionlen &&
            memcmp(sections[i].name, buf + 1, namelen) == 0)
          break;
      }

      if (i < nsections)
      {
        input_section  = &sections[i];
        output_section = &keymap->sections[i];
      }
      else
      {
        return error_KEYMAP_UNKNOWN_SECTION; // cleanup
      }

      continue;
    }

    if (input_section == NULL)
      continue; /* skip until we've encountered a valid section */

    /* expand buffer where required */

    if (output_section->used >= output_section->allocated)
    {
      int                   newallocated;
      keymap_key_to_action *newmap;

      newallocated = output_section->allocated * 2;
      if (newallocated <= 0)
        newallocated = 8;

      newmap = realloc(output_section->map,
                       sizeof(*newmap) * newallocated);
      if (newmap == NULL)
        return error_OOM;

      output_section->map       = newmap;
      output_section->allocated = newallocated;
    }

    err = parse_line(input_section, buf,
                    &output_section->map[output_section->used]);
    if (err)
      goto Failure;

    output_section->used++;
  }

  fclose(f);
  f = NULL;

  /* sort all of the tables */

  for (s = &keymap->sections[0]; s < &keymap->sections[nsections]; s++)
    qsort(s->map, s->used, sizeof(*s->map), key_to_action_compare);

  /* return */

  *keymap_out = keymap;

  return error_OK;


Failure:

  fclose(f);

  return err;
}

int keymap__action(keymap_t *keymap, int section, wimp_key_no key_no)
{
  int              i;
  keymap_section2 *s;

  assert(keymap);

  if (section >= keymap->nsections)
    return -1; /* unknown */

  s = &keymap->sections[section];

  i = bsearch_int(&s->map[0].key, s->used, sizeof(*s->map), key_no);

  return (i >= 0) ? s->map[i].action : -1; /* known : unknown */
}

void keymap__destroy(keymap_t *keymap)
{
  keymap_section2 *s;

  for (s = &keymap->sections[0]; s < &keymap->sections[keymap->nsections]; s++)
    free(s->map);

  free(keymap);
}
