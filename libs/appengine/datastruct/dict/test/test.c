
#include <stdio.h>

#include "fortify/fortify.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/datastruct/dict.h"

static const char *data[] =
{
  "deckard", "batty", "rachael", "gaff", "bryant", "pris", "sebastian",

  /* dupes */
  "batty", "bryant", "deckard", "gaff", "pris", "rachael", "sebastian",
};

static const char *newnames[] =
{
  /* last is a dupe */
  "fred", "barney", "wilma", "betty", "pebbles", "bamm bamm", "fred",
};

static error test_add(dict_t *d)
{
  error err;
  int   i;

  printf("test: add\n");

  for (i = 0; i < NELEMS(data); i++)
  {
    dict_index idx;

    printf("adding '%s'... ", data[i]);

    err = dict__add(d, data[i], &idx);
    if (err && err != error_DICT_NAME_EXISTS)
      return err;

    if (err == error_DICT_NAME_EXISTS)
      printf("already exists ");

    printf("as %d\n", idx);
  }

  return error_OK;
}

static error test_to_string(dict_t *d)
{
  int i;

  printf("test: to string\n");

  for (i = 0; i < NELEMS(data); i++)
  {
    const char *string;

    string = dict__string(d, i);
    if (string)
      printf("%d is '%s'\n", i, string);
  }

  return error_OK;
}

static error test_to_string_and_len(dict_t *d)
{
  int i;

  printf("test: to string and length\n");

  for (i = 0; i < NELEMS(data); i++)
  {
    const char *string;
    size_t      len;

    string = dict__string_and_len(d, &len, i);
    if (string)
      printf("%d is '%.*s' (length %u)\n", i, (int) len, string, len);
  }

  return error_OK;
}

static error test_delete(dict_t *d)
{
  int i;

  printf("test: delete\n");

  for (i = 0; i < NELEMS(data); i++)
    dict__delete(d, data[i]);

  return error_OK;
}

static error test_rename(dict_t *d)
{
  error err;
  int   i;

  printf("test: rename\n");

  for (i = 0; i < NELEMS(newnames); i++)
  {
    dict_index idx;

    printf("adding '%s'... ", data[i]);

    err = dict__add(d, data[i], &idx);
    if (err && err != error_DICT_NAME_EXISTS)
      return err;

    if (err == error_DICT_NAME_EXISTS)
      printf("already exists ");

    printf("as %d\n", idx);

    printf("renaming index %d to '%s'... ", idx, newnames[i]);

    err = dict__rename(d, idx, newnames[i]);
    if (err == error_DICT_NAME_EXISTS)
      printf("already exists!");
    else if (err)
      return err;
    else
      printf("ok");

    printf("\n");
  }

  return error_OK;
}

static int rnd(int mod)
{
  return (rand() % mod) + 1;
}

static const char *randomname(void)
{
  static char buf[5 + 1];

  int length;
  int i;

  length = rnd(NELEMS(buf) - 1);

  for (i = 0; i < length; i++)
    buf[i] = 'a' + rnd(26) - 1;

  buf[i] = '\0';

  return buf;
}

static error test_random(dict_t *d)
{
  error err;
  int   i;

  printf("test: random\n");

  for (i = 0; i < 1000; i++)
  {
    const char *name;
    dict_index  idx;

    name = randomname();

    printf("adding '%s'... ", name);

    err = dict__add(d, name, &idx);
    if (err && err != error_DICT_NAME_EXISTS)
      return err;

    if (err == error_DICT_NAME_EXISTS)
      printf("already exists ");

    printf("as %d\n", idx);
  }

  return error_OK;
}

int dict_test(void)
{
  error   err;
  dict_t *d;

  d = dict__create_tuned(1, 12);
  if (d == NULL)
    return 1;

  err = test_add(d);
  if (err)
    return 1;

  err = test_to_string(d);
  if (err)
    return 1;

  err = test_to_string_and_len(d);
  if (err)
    return 1;

  err = test_delete(d);
  if (err)
    return 1;

  err = test_to_string_and_len(d);
  if (err)
    return 1;

  err = test_add(d);
  if (err)
    return 1;

  err = test_to_string(d);
  if (err)
    return 1;

  err = test_rename(d);
  if (err)
    return 1;

  err = test_to_string(d);
  if (err)
    return 1;

  err = test_to_string_and_len(d);
  if (err)
    return 1;

  err = test_random(d);
  if (err)
    return 1;

  dict__destroy(d);

  return 0;
}
