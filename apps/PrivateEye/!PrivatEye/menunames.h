/* --------------------------------------------------------------------------
 *    Name: menunames.h
 * Purpose: Identifiers for menu numbers
 * ----------------------------------------------------------------------- */

#ifndef MENUNAMES_H
#define MENUNAMES_H

/* We prefer enum rather than #defines to allow const-like behaviour and real
 * compile-time syntax checking. */
enum
{
  ICONBAR_INFO          = 0,
#if defined(EYE_CANVAS)
  ICONBAR_NEW,
#endif
  ICONBAR_CLOSE,
  ICONBAR_EMPTYCACHE,
#ifdef EYE_TAGS
  ICONBAR_SEARCHTAGS,
#endif
  ICONBAR_CHOICES,
  ICONBAR_QUIT,
  ICONBAR__LIMIT,

  NEW_CANVAS            = 0,

  CLOSE_VIEWERS         = 0,
  CLOSE_THUMBVIEWS,
  CLOSE_CANVASES,

  IMAGE_FILE            = 0,
  IMAGE_SAVE,
  IMAGE_EDIT,
  IMAGE_SCALE,

  FILE_INFO             = 0,
  FILE_SOURCE,
#ifdef EYE_META
  FILE_METADATA,
#endif
  FILE_HIST,
#ifdef EYE_TAGS
  FILE_TAGS,
#endif
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

#ifdef EYE_THUMBVIEW
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
#endif
};

#endif /* MENUNAMES_H */
