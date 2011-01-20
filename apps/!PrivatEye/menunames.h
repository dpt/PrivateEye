/* --------------------------------------------------------------------------
 *    Name: menunames.h
 * Purpose: Identifiers for menu numbers
 * Version: $Id: menunames.h,v 1.22 2009-03-29 18:18:27 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef MENUNAMES_H
#define MENUNAMES_H

/* We prefer enum rather than #defines to allow const-like behaviour and real
 * compile-time syntax checking. */
enum
{
  ICONBAR_INFO          = 0,
  ICONBAR_CLOSE,
  ICONBAR_EMPTYCACHE,
  ICONBAR_SEARCHTAGS,
  ICONBAR_CHOICES,
  ICONBAR_QUIT,
  ICONBAR__LIMIT,

  IMAGE_FILE            = 0,
  IMAGE_SAVE,
  IMAGE_EDIT,
  IMAGE_SCALE,

  FILE_INFO             = 0,
  FILE_SOURCE,
  FILE_METADATA,
  FILE_HIST,
  FILE_TAGS,
  FILE_NEWVIEW,

  EDIT_EFFECTS          = 0,
  EDIT_ROTATE,
  EDIT_CONVERT_TO_SPRITE,
  EDIT_CLAIM,
  EDIT_RELEASE,

  EFFECTS_COLOUR        = 0,
  EFFECTS_CURVE,
  EFFECTS_GREY,
  EFFECTS_BLUR,
  EFFECTS_SHARPEN,
  EFFECTS_EXPAND,
  EFFECTS_EQUALISE,
  EFFECTS_EMBOSS,

  METADATA_EXPAND_ALL   = 0,
  METADATA_COLLAPSE_ALL,

  THUMBVIEW_DISPLAY     = 0,
  THUMBVIEW_IMAGE,
  THUMBVIEW_SELECT_ALL,
  THUMBVIEW_CLEAR_ALL,
  THUMBVIEW_OPTIONS,

  DISPLAY_LARGE_THUMB   = 0,
  DISPLAY_SMALL_THUMB,
  DISPLAY_FULL_INFO_HORZ,
  DISPLAY_FULL_INFO_VERT,

  TIMAGE_TAG            = 0,
  TIMAGE_ROTATE
};

#endif /* MENUNAMES_H */
