/* --------------------------------------------------------------------------
 *    Name: actions.h
 * Purpose: Actions
 * Version: $Id: actions.h,v 1.3 2009-05-20 21:38:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef ACTIONS_H
#define ACTIONS_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

enum
{
  Close,
  ConvToSpr,
  Copy,
  Effects,
  Help,
  Hist,
  HorzFlip,
  Info,
  Kill,
  MetaData,
  NewView,
  PanDown,
  PanLeft,
  PanRandom,
  PanRight,
  PanUp,
  Release,
  Rotate,
  RotateLeft,
  RotateRight,
  Save,
  Scale,
  SourceInfo,
  StepBackwards,
  StepForwards,
  Tags,
  VertFlip,
  ZoomIn,
  ZoomOut,
  ZoomReset,
  ZoomToggle,
  Unknown,

  TagCloud_List,
  TagCloud_Cloud,
  TagCloud_SmallCloud,
  TagCloud_UnscaledCloud,
  TagCloud_SortByCount,
  TagCloud_SortByName,
  TagCloud_SortSelFirst,
  TagCloud_Rename,
  TagCloud_Kill,
  TagCloud_New,
  TagCloud_Info,
  TagCloud_Commit,

  Thumbview_LargeThumbs,
  Thumbview_SmallThumbs,
  Thumbview_FullInfoHorz,
  Thumbview_FullInfoVert,
  Thumbview_SortByName,
  Thumbview_SortByCount,
  Thumbview_SelectAll,
  Thumbview_ClearSelection,
};

error action_help(void);
error action_close_window(wimp_w w);

#endif /* ACTIONS_H */
