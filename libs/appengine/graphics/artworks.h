/* --------------------------------------------------------------------------
 *    Name: artworks-common.h
 * Purpose: Common ArtWorks code
 * Version: $Id: artworks.h,v 1.1 2009-04-28 23:32:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_ARTWORKS_COMMON_H
#define APPENGINE_ARTWORKS_COMMON_H

#include "kernel.h"

#include "flex.h"

#include "appengine/graphics/awrender.h"

typedef struct
{
  flex_ptr resizable_block;
  flex_ptr fixed_block;
}
artworks_handle;

_kernel_oserror *artworks_callback(int                     new_size,
                                   awrender_callback_regs *regs,
                                   artworks_handle        *handle);

#endif /* APPENGINE_ARTWORKS_COMMON_H */
