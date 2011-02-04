/* --------------------------------------------------------------------------
 *    Name: artworks-common.c
 * Purpose: Common ArtWorks code
 * ----------------------------------------------------------------------- */

#include "kernel.h"

#include "flex.h"

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/graphics/awrender.h"

#include "appengine/graphics/artworks.h"

_kernel_oserror *artworks_callback(int                     new_size,
                                   awrender_callback_regs *regs,
                                   artworks_handle        *handle)
{
  regs->resizable_size = flex_size(handle->resizable_block);

  if (new_size > regs->resizable_size)
  {
    /* small error block to save space - yuck */
    static char no_mem[] = "\0\0\0\1NoMem";

    if (flex_extend(handle->resizable_block, new_size) == 0)
      return (_kernel_oserror *) &no_mem;

    regs->resizable_size = new_size;
  }

  regs->resizable_block = *handle->resizable_block;

  if ((int) handle->fixed_block == -1)
  {
    regs->fixed_block = (void *) -1;
    regs->fixed_size  = regs->resizable_size;
  }
  else
  {
    regs->fixed_block = *handle->fixed_block;
    regs->fixed_size  = flex_size(handle->fixed_block);
  }

  return NULL;
}
