/* $Id: test.c,v 1.5 2010-01-06 14:10:43 dpt Exp $ */

#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

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
    dict_index index;

    printf("adding '%s'... ", data[i]);

    err = dict__add(d, data[i], &index);
    if (err && err != error_DICT_NAME_EXISTS)
      return err;

    if (err == error_DICT_NAME_EXISTS)
      printf("already exists ");

    printf("as %d\n", index);
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
      printf("%d is '%.*s' (length %d)\n", i, len, string, len);
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
    dict_index index;

    printf("adding '%s'... ", data[i]);

    err = dict__add(d, data[i], &index);
    if (err && err != error_DICT_NAME_EXISTS)
      return err;

    if (err == error_DICT_NAME_EXISTS)
      printf("already exists ");

    printf("as %d\n", index);

    printf("renaming index %d to '%s'... ", index, newnames[i]);

    err = dict__rename(d, index, newnames[i]);
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

int dict_test(void)
{
  error   err;
  dict_t *d;

  d = dict__create();
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

  dict__destroy(d);

  return 0;
}
