/* --------------------------------------------------------------------------
 *    Name: mode.c
 * Purpose: Returns the best mode for the specified args
 * ----------------------------------------------------------------------- */

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"

#include "appengine/base/os.h"

#include "appengine/vdu/sprite.h"

os_mode sprite_mode(int xeig, int yeig, int log2bpp, osbool inline_alpha)
{
  static int ro5extsprs = -1; /* not checked yet */

#define NO_MODE (UCHAR_MAX)

  static const unsigned char modetab[4][2][2] =
  {
    {
      { 25, NO_MODE, },
      {  0, NO_MODE, },
    },
    {
      { 26, NO_MODE, },
      {  8,       1, },
    },
    {
      { 27, NO_MODE, },
      { 12,       9, },
    },
    {
      { 28, NO_MODE, },
      { 15,      13, },
    },
  };

  unsigned int mode;

  if (xeig > 3 || yeig > 3)
    return (os_mode) -1;

  if (xeig >= 1 && yeig >= 1 && log2bpp <= 3)
  {
    /* use table */
    mode = modetab[log2bpp][yeig - 1][xeig - 1]; 
    if (mode != NO_MODE)
      return (os_mode) mode;
  }

  if (ro5extsprs < 0)
  {
    int os     = os_version();
    int sprext = get_module_version("SpriteExtend");

    /* Detect RISC OS 5 and SpriteExtend 1.52 which is the first version that
     * knows about extended sprite words. */
    ro5extsprs = (os >= osversion_5) && (sprext >= 0x00015200);
  }

  if (inline_alpha && ro5extsprs)
  {
    if (log2bpp != 5)
      return (os_mode) -1;

    mode = osspriteop_EXT_STYLE;
    mode |= xeig << osspriteop_EXT_XRES_SHIFT;
    mode |= yeig << osspriteop_EXT_YRES_SHIFT;
    mode |= ((os_MODE_FLAG_DATA_SUBFORMAT_ABGR << os_MODE_FLAG_DATA_SUBFORMAT_SHIFT) >> 8) << osspriteop_EXT_MODE_FLAGS_SHIFT;
    mode |= (1 + log2bpp) << osspriteop_EXT_TYPE_SHIFT;
  }
  else
  {
    /* 0 -> 180, 1 -> 90, 2 -> 45 */

    mode = osspriteop_NEW_STYLE;
    mode |= (180 >> xeig) << osspriteop_XRES_SHIFT;
    mode |= (180 >> yeig) << osspriteop_YRES_SHIFT;
    mode |= (1 + log2bpp) << osspriteop_TYPE_SHIFT;
  }

  return (os_mode) mode;
}

/* ---------------------------------------------------------------------- */

static osbool has_alpha(os_mode_flags mode_flags)
{
  os_mode_flags format;

  format = (mode_flags & os_MODE_FLAG_DATA_FORMAT) >> os_MODE_FLAG_DATA_FORMAT_SHIFT;
  return format == os_MODE_FLAG_DATA_SUBFORMAT_ABGR ||
         format == os_MODE_FLAG_DATA_SUBFORMAT_ARGB;
}

result_t sprite_describe_mode(os_mode osmode, char *desc, size_t sz)
{
  unsigned int mode = (unsigned int) osmode;
  const char  *compat;
  char         buf[64];

  if (mode < 256)
  {
    snprintf(buf, sizeof(buf), "Mode %d", mode);
  }
  else if (mode & osspriteop_NEW_STYLE)
  {
    osbool warning;

    if ((mode & osspriteop_EXT_STYLE) == osspriteop_EXT_STYLE)
    {
      osbool       wide_mask;
      unsigned int sprite_type;
      unsigned int sbz;
      unsigned int mode_flags;
      // unsigned int y_eigen;
      // unsigned int x_eigen;

      wide_mask   = (mode & osspriteop_ALPHA_MASK) == osspriteop_ALPHA_MASK;
      sprite_type = (mode & osspriteop_EXT_TYPE)       >> osspriteop_EXT_TYPE_SHIFT;
      sbz         = (mode & 0x000F000E); /* should be zero */
      mode_flags  = (mode & osspriteop_EXT_MODE_FLAGS) >> osspriteop_EXT_MODE_FLAGS_SHIFT;
      // y_eigen     = (mode & osspriteop_EXT_YRES)       >> osspriteop_EXT_YRES_SHIFT;
      // x_eigen     = (mode & osspriteop_EXT_XRES)       >> osspriteop_EXT_XRES_SHIFT;

      warning = (sbz != 0) ||
	        (sprite_type == 0) ||
		(wide_mask && has_alpha(mode_flags << 8));

      compat = "RISC OS 5.21";
    }
    else
    {
      osbool       wide_mask;
      unsigned int sprite_type;
      // unsigned int vertical_dpi;
      // unsigned int horizontal_dpi;
      
      wide_mask      = (mode & osspriteop_ALPHA_MASK) == osspriteop_ALPHA_MASK;
      sprite_type    = (mode & osspriteop_TYPE) >> osspriteop_TYPE_SHIFT;
      // vertical_dpi   = (mode & osspriteop_YRES) >> osspriteop_YRES_SHIFT;
      // horizontal_dpi = (mode & osspriteop_XRES) >> osspriteop_XRES_SHIFT;

      warning = (sprite_type == 0);
      if (wide_mask || sprite_type >= osspriteop_TYPE_CMYK)
        compat = "RISC OS Select, 5.21";
      else
        compat = "RISC OS 3.5";
    }

    snprintf(buf, sizeof(buf), "&%X (%s)%s", mode, compat, warning ? " [?]" : "");
  }
  else
  {
    snprintf(buf, sizeof(buf), "Unknown mode &%X", mode);
  }

  strncpy(desc, buf, sz);
  desc[sz - 1] = '\0';

  return result_OK;
}
