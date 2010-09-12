/* $Id: test.c,v 1.2 2010-01-10 00:11:21 dpt Exp $ */

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "oslib/types.h"
#include "oslib/os.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/test/txtscr.h"

#include "appengine/geom/packer.h"

/* ----------------------------------------------------------------------- */

static error drawbox(const os_box *box, void *arg)
{
  txtscr_t *scr = arg;

  printf("drawbox: %d %d %d %d\n", box->x0, box->y0, box->x1, box->y1);

  txtscr_addbox(scr, box);

  return error_OK;
}

static error dumppacker(packer_t *packer, txtscr_t *scr)
{
  error err;

  txtscr_clear(scr);

  err = packer__map(packer, drawbox, scr);
  if (err)
    return err;

  txtscr_print(scr);

  return error_OK;
}

static error dumpboxlist(const os_box *list, int nelems, txtscr_t *scr)
{
  error         err;
  const os_box *b;

  txtscr_clear(scr);

  for (b = list; b < list + nelems; b++)
  {
    err = drawbox(b, scr);
    if (err)
      return err;
  }

  txtscr_print(scr);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static error subtest1(packer_t *packer, txtscr_t *scr)
{
  static const os_box areas[] =
  {
    {  0,  0, 10, 20 },
    { 40,  0, 50, 20 },
    { 10, 20, 40, 25 },
  };

  error err;
  int   i;

  /* place a large area */

  for (i = 0; i < NELEMS(areas); i++)
  {
    err = packer__place_at(packer, &areas[i]);
    if (err)
      goto failure;
  }

  printf("\n");

  for (i = 0; i < 10; i++)
  {
    const os_box *placed;

    dumppacker(packer, scr);

    err = packer__place_by(packer, packer__LOC_TOP_LEFT, 3,3, &placed);
    if (err == error_PACKER_DIDNT_FIT)
    {
      printf("could not place\n");
      break;
    }

    printf("%d placed at: {%d %d %d %d}\n", i, placed->x0, placed->y0,
                                               placed->x1, placed->y1);
  }

  dumppacker(packer, scr);

  packer__clear(packer, packer__CLEAR_LEFT);

  dumppacker(packer, scr);

  return error_OK;


failure:

  return err;
}

static error test1(void)
{
  static const os_box pagedims = { 0, 0, 50, 25 };

  error     err;
  txtscr_t *scr;
  packer_t *packer;
  os_box    margins;

  printf("test1\n");

  scr = txtscr_create(50, 25);
  if (scr == NULL)
  {
    err = error_OOM;
    goto failure;
  }

  packer = packer__create(&pagedims);
  if (packer == NULL)
  {
    err = error_OOM;
    goto failure;
  }

  err = subtest1(packer, scr);
  if (err)
    goto failure;

  packer__destroy(packer);

  printf("\n\nwith margins\n\n\n");

  packer = packer__create(&pagedims);
  if (packer == NULL)
  {
    err = error_OOM;
    goto failure;
  }

  margins.x0 = 5;
  margins.y0 = 3;
  margins.x1 = 5;
  margins.y1 = 3;
  packer__set_margins(packer, &margins);

  err = subtest1(packer, scr);
  if (err)
    goto failure;

  margins.x0 = 0;
  margins.y0 = 0;
  margins.x1 = 0;
  margins.y1 = 0;
  packer__set_margins(packer, &margins);

  dumppacker(packer, scr);

  packer__destroy(packer);

  txtscr_destroy(scr);

  return error_OK;


failure:

  return err;
}

#define MAXWIDTH  100
#define MAXHEIGHT 36
#define PADDING   2

static error subtest2(packer_loc loc, packer_clear clear)
{
  static const os_box pagedims = {       0,       0, MAXWIDTH, MAXHEIGHT };
  static const os_box margins  = { PADDING, PADDING,  PADDING,   PADDING };

  struct
  {
    int minw;
    int maxw;
    int h;
    int chosenw;
  }
  elements[] =
  {
    { 26,26,     26, 0 },
    { 26,26,     28, 0 },
//  { 96,INT_MAX, 1, 0 },
    { 26,26,     26, 0 },
    // want to force a newline here
    { 26,INT_MAX, 2, 0 }
  };

  error         err;
  txtscr_t     *scr;
  packer_t     *packer;
  int           i;
  int           j;
  const os_box *placed;
  os_box        list[32];

  printf("test2: loc=%d, clear=%d\n", loc, clear);

  scr = txtscr_create(MAXWIDTH, MAXHEIGHT);
  if (scr == NULL)
  {
    err = error_OOM;
    goto failure;
  }

  packer = packer__create(&pagedims);
  if (packer == NULL)
  {
    err = error_OOM;
    goto failure;
  }

  packer__set_margins(packer, &margins);

  dumppacker(packer, scr);

  i = 0;
  j = 0;

  do
  {
    int remaining;
    int first, last;
    int need;
    int k;

    printf("loop: i=%d\n", i);

    remaining = packer__next_width(packer, loc);
    printf("remaining = %d\n", remaining);

    first = last = i; /* upper bound 'last' is exclusive */

    need = elements[last].minw; /* skip padding on initial element */
    while (remaining >= need)
    {
      int chosenw;

      chosenw = MIN(remaining, elements[last].maxw);
      elements[last].chosenw = chosenw;
      remaining -= chosenw;
      last++;

      printf("element %d: chosenw = %d\n", last - 1, chosenw);

      if (last >= NELEMS(elements))
        break;

      need = PADDING + elements[last].minw;
    }

    i = last; /* 'last' and 'i' are essentially the same variable */

    printf("can place from %d to %d on this line\n", first, last - 1);

    /* place vertical padding */

    if (first > 0) /* don't pad at the top */
    {
      printf("place vertical padding\n");

      err = packer__place_by(packer,
                             loc,
                             MAXWIDTH - 2 * PADDING, PADDING,
                             NULL);
      if (err == error_PACKER_DIDNT_FIT)
      {
        printf("*** could not place vertical padding\n");
        break;
      }
    }

    for (k = first; k < last; k++)
    {
      if (k > first)
      {
        /* place horizontal padding */

        printf("place horizontal padding\n");

        err = packer__place_by(packer,
                               loc,
                               PADDING, elements[k].h,
                               NULL);
        if (err == error_PACKER_DIDNT_FIT)
        {
          printf("*** could not place horizontal padding\n");
          break;
        }
      }

      /* place element */

      printf("place element\n");

      err = packer__place_by(packer,
                             loc,
                             elements[k].chosenw, elements[k].h,
                            &placed);
      if (err == error_PACKER_DIDNT_FIT)
      {
        printf("*** could not place element\n");
        break;
      }
      else if (placed)
      {
        list[j++] = *placed;
      }
    }

    if (remaining < need)
    {
      /* there's space, but it's not enough for the next element - start a
       * new line */

      printf("*** won't fit on this line (only %d left, but need %d)\n",
             remaining, need);

      err = packer__clear(packer, clear);
      if (err)
        goto failure;
    }
  }
  while (i < NELEMS(elements));

  dumppacker(packer, scr);

  dumpboxlist(list, j, scr);

  packer__destroy(packer);

  txtscr_destroy(scr);

  return error_OK;


failure:

  return err;
}

static int test2(void)
{
  static const struct
  {
    packer_loc   loc;
    packer_clear clear;
  }
  tests[] =
  {
    { packer__LOC_TOP_LEFT,     packer__CLEAR_LEFT  },
    { packer__LOC_TOP_RIGHT,    packer__CLEAR_RIGHT },
  };

  error err;
  int   i;

  for (i = 0; i < NELEMS(tests); i++)
  {
    err = subtest2(tests[i].loc, tests[i].clear);
    if (err)
      goto failure;
  }

  return 0;


failure:

  return 1;
}

int packer_test(void)
{
  error err;

  err = test1();
  if (err)
    goto failure;

  err = test2();
  if (err)
    goto failure;

  return 0;


failure:

  return 1;
}
