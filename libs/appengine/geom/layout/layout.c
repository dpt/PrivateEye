/* --------------------------------------------------------------------------
 *    Name: layout.c
 * Purpose: Laying out elements using the Packer
 * Version: $Id: layout.c,v 1.1 2009-05-18 22:25:31 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/types.h"
#include "oslib/os.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/geom/packer.h"

#include "appengine/geom/layout.h"

/* ----------------------------------------------------------------------- */

error layout__place(const layout_spec     *spec,
                    const layout_element  *elements,
                          int              nelements,
                          os_box          *boxes,
                          int              nboxes)
{
  error err = error_OK;
  int   j;
  int   i;
  int   chosen_width[32];

  j = 0;

  for (i = 0; i < nelements; )
  {
    int nextw;
    int first;
    int clear;
    int k;

    nextw = packer__next_width(spec->packer, spec->loc);

    first = i; /* upper bound 'i' is exclusive */

    clear = 0;

    if (elements[first].type == layout_BOX)
    {
      int remaining;
      int need;

      remaining = nextw;

      /* fit as many items on the line as possible */

      /* skip the padding on the initial element */
      need = elements[i].data.box.min_width;
      while (remaining >= need)
      {
        /* MAYBE if (need > elements[i].data.box.max_width) bad spec; */

        chosen_width[i] = MIN(remaining, elements[i].data.box.max_width);
        remaining -= chosen_width[i];

        if (++i >= nelements)
          break;

        if (elements[i].type != layout_BOX)
          break;

        need = spec->spacing + elements[i].data.box.min_width;
      }

      if (remaining < need)
        clear = 1;



      /* place vertical padding */

      if (first > 0 || /* don't pad at the top */
          elements[0].type == layout_NEWLINE /* unless explicit */)
      {
        err = packer__place_by(spec->packer,
                               spec->loc,
                               nextw, spec->leading,
                               NULL);
        if (err == error_PACKER_DIDNT_FIT)
          break;
        else if (err)
          goto failure;

        if (elements[first].type == layout_NEWLINE)
          continue;
      }

      for (k = first; k < i; k++)
      {
        const os_box *placed;

        if (k > first)
        {
          /* place horizontal padding */

          err = packer__place_by(spec->packer,
                                 spec->loc,
                                 spec->spacing, elements[k].data.box.height,
                                 NULL);
          if (err == error_PACKER_DIDNT_FIT)
            break;
          else if (err)
            goto failure;
        }

        /* place element */

        err = packer__place_by(spec->packer,
                               spec->loc,
                               chosen_width[k], elements[k].data.box.height,
                              &placed);
        if (err == error_PACKER_DIDNT_FIT)
          break;
        else if (err)
          goto failure;

        if (placed)
        {
          if (j == nboxes)
          {
            err = error_LAYOUT_BUFFER_FULL;
            goto failure;
          }

          boxes[j++] = *placed;
        }
      }

      if (err == error_PACKER_DIDNT_FIT)
        break;
      else if (err)
        goto failure;
    }
    else if (elements[first].type == layout_NEWLINE)
    {
      i++;
      clear = 1;
    }

    if (clear)
    {
      /* there's space, but it's not enough for the next element - start a
       * new line */

      err = packer__clear(spec->packer, spec->clear);
      if (err)
        goto failure;
    }
  }

  return error_OK;


failure:

  return err;
}
