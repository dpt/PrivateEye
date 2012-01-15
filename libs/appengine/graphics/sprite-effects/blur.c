/* --------------------------------------------------------------------------
 *    Name: blur.c
 * Purpose: Blur effect
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/os.h"
#include "oslib/osspriteop.h"

#include "appengine/types.h"

#include "convolve.h"

#include "appengine/graphics/sprite-effects.h"

#define MINAMT  2 /*  5-pt kernel */
#define MAXAMT 48 /* 95-pt kernel */

static void make_kernel(effects_blur_method method, int width, float *kernel)
{
  int    i;
  double k[width];
  double sum;

  switch (method)
  {
  case effects_blur_BOX:
    for (i = 0; i < width; i++)
      k[i] = 1.0;
    break;

  case effects_blur_GAUSSIAN:
    for (i = 0; i < width; i++)
    {
      double f;

      /* 2f^3 - 3f^2 + 1 */
      f = (double) i / width;
      k[i] = (float) (((2.0 * f * f * f) - (3.0 * f * f) + 1.0) / width);
    }
    break;
  }

  /* sum the weights */

  sum = k[0];
  for (i = 1; i < width; i++)
    sum += k[i] * 2;

  /* scale the kernel */

  for (i = 0; i < width; i++)
    kernel[i] = (float) (k[i] / sum);
}

error effects_blur_apply(osspriteop_area    *area,
                         osspriteop_header  *src,
                         osspriteop_header  *dst,
                         effects_blur_method method,
                         int                 amount)
{
  float         kernel[MAXAMT];
  error         err;
  convolve_lut *lut;

  err = error_OK;
  lut = NULL;

  amount = CLAMP(amount, MINAMT, MAXAMT);

  make_kernel(method, amount, kernel);

  err = convolve_init(kernel, amount, &lut);
  if (err)
    goto Failure;

  err = convolve_sprite(lut, area, src, dst);
  if (err)
    goto Failure;

  /* FALLTHROUGH */

Failure:

  convolve_destroy(lut);

  return err;
}
