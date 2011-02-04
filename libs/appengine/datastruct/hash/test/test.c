
#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/types.h"
#include "appengine/datastruct/hash.h"

static char *my_strdup(const char *s)
{
  size_t l;
  char  *d;

  l = strlen(s);

  d = malloc(l + 1);
  if (d == NULL)
    return NULL;

  memcpy(d, s, l + 1);

  return d;
}

static int my_walk_fn(const void *key, const void *value, void *arg)
{
  const char *sk = key;
  const char *sv = value;

  NOT_USED(arg);

  printf("walk '%s':'%s'...\n", sk, sv);

  return 0;
}

int hash_test(void)
{
  static const struct
  {
    const char *name;
    const char *value;
  }
  data[] =
  {
    { "deckard",   "rick" },
    { "batty",     "roy" },
    { "tyrell",    "rachael" },
    { "gaff",      "n/a" },
    { "bryant",    "n/a" },
    { "pris",      "n/a" },
    { "sebastian", "jf" },
  };

  error   err;
  hash_t *d;
  int     i;

  printf("test: create\n");

  /* use default string handling */
  err = hash__create(20, NULL, NULL, NULL, NULL, &d);
  if (err)
    return 1;

  printf("test: insert\n");

  for (i = 0; i < NELEMS(data); i++)
  {
    char *s;
    char *v;

    printf("adding '%s':'%s'...\n", data[i].name, data[i].value);

    s = my_strdup(data[i].name);
    v = my_strdup(data[i].value);

    if (!s || !v)
      return 1;

    err = hash__insert(d, s, v);
    if (err)
      return 1;
  }

  printf("test: iterate\n");

  hash__walk(d, my_walk_fn, NULL);

  printf("test: remove\n");

  for (i = 0; i < NELEMS(data); i++)
    hash__remove(d, data[i].name);

  printf("test: destroy\n");

  hash__destroy(d);

  return 0;
}
