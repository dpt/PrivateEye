/* --------------------------------------------------------------------------
 *    Name: sharpen.c
 * Purpose: Blur effect
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/os.h"
#include "oslib/osspriteop.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"

#include "convolve.h"

#include "appengine/graphics/sprite-effects.h"

error effects_sharpen_apply(osspriteop_area   *area,
                            osspriteop_header *src,
                            osspriteop_header *dst,
                            int                amount)
{
  static const float kernel[] = { 1.5f, -0.25f };

  error         err;
  convolve_lut *lut;

  NOT_USED(amount);

  err = error_OK;
  lut = NULL;

  err = convolve_init(kernel, NELEMS(kernel), &lut);
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
