/* --------------------------------------------------------------------------
 *    Name: layout.h
 * Purpose: Laying out elements using the Packer
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_LAYOUT_H
#define APPENGINE_LAYOUT_H

#include "oslib/os.h"

#include "appengine/base/errors.h"
#include "appengine/geom/packer.h"

typedef struct layout_element
{
  enum
  {
    layout_BOX,
    layout_NEWLINE,
  }
  type;

  union
  {
    struct
    {
      int min_width, max_width;
      int height;
    }
    box;
  }
  data;
}
layout_element;

typedef struct layout_spec
{
  packer_t       *packer;
  packer_loc      loc;
  packer_cleardir clear;
  int             spacing;
  int             leading;
}
layout_spec;

error layout_place(const layout_spec     *spec,
                   const layout_element  *elements,
                         int              nelements,
                         os_box          *boxes,
                         int              nboxes);

#endif /* APPENGINE_LAYOUT_H */
