/* --------------------------------------------------------------------------
 *    Name: choicesdat.c
 * Purpose: Choices data
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/types.h"
#include "appengine/app/choices.h"

#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "choices-colour.h"
#include "globals.h"
#include "iconnames.h"
#include "privateeye.h"
#include "tags.h"
#include "viewer.h"

#include "choicesdat.h"

/* ----------------------------------------------------------------------- */

/* Writable variables which the choices data references */

static struct
{
  choices_vars vars;

  wimp_w       choices_w;
  wimp_w       choices_vwr_w;
#ifdef EYE_THUMBVIEW
  wimp_w       choices_thm_w;
#endif
  wimp_w       choices_col_w;
#ifdef EYE_TAGS
  wimp_w       choices_tag_w;
#endif
  wimp_w       choices_spr_w;
  wimp_w       choices_jpg_w;
  wimp_w       choices_gif_w;
  wimp_w       choices_png_w;
  wimp_w       choices_drw_w;
  wimp_w       choices_art_w;

  wimp_w       current_choices_xxx_w;
}
W;

/* ----------------------------------------------------------------------- */

static const choices_pane_handlers colours_handlers =
{
  colour_choicepane_initialise,
  colour_choicepane_finalise,
  colour_choicepane_changed,
  colour_choicepane_redraw,
};

enum
{
  choicepane_vwr,
#ifdef EYE_THUMBVIEW
  choicepane_thm,
#endif
  choicepane_col,
#ifdef EYE_TAGS
  choicepane_tag,
#endif
  choicepane_spr,
  choicepane_jpg,
  choicepane_gif,
  choicepane_png,
  choicepane_drw,
  choicepane_art,
  choicepane__LIMIT
};

/* If you alter these then the various pane indices below will also need
 * updating. */
static const choices_pane panes[choicepane__LIMIT] =
{
  { &W.choices_vwr_w, "vwr", CHOICES_R_VWR, NULL,             },
#ifdef EYE_THUMBVIEW
  { &W.choices_thm_w, "thm", CHOICES_R_THM, NULL,             },
#endif
  { &W.choices_col_w, "col", CHOICES_R_COL, &colours_handlers },
#ifdef EYE_TAGS
  { &W.choices_tag_w, "tag", CHOICES_R_TAG, NULL,             },
#endif
  { &W.choices_spr_w, "spr", CHOICES_R_SPR, NULL,             },
  { &W.choices_jpg_w, "jpg", CHOICES_R_JPG, NULL,             },
  { &W.choices_gif_w, "gif", CHOICES_R_GIF, NULL,             },
  { &W.choices_png_w, "png", CHOICES_R_PNG, NULL,             },
  { &W.choices_drw_w, "drw", CHOICES_R_DRW, NULL,             },
  { &W.choices_art_w, "art", CHOICES_R_ART, NULL,             },
};

/* ----------------------------------------------------------------------- */

/* These are prefixed "viewerchoices" rather than just "viewer" to avoid name
 * conflicts. */

static const choices_stringset_vals viewerchoices_size_vals[] =
{
  /* ideally, these values should be an enum */
  { viewersize_FIT_TO_IMAGE },
  { viewersize_FIT_TO_SCREEN },
};

static const choices_stringset viewerchoices_size =
{
  "size",
  CHOICES_VWR_D_SIZE,
  CHOICES_VWR_P_SIZE,
  NELEMS(viewerchoices_size_vals),
  viewerchoices_size_vals
};

static const choices_stringset_vals viewerchoices_scale_vals[] =
{
  { SCALE_100PC * 1 / 4 }, /*  25% */
  { SCALE_100PC * 2 / 4 }, /*  50% */
  { SCALE_100PC * 3 / 4 }, /*  75% */
  { SCALE_100PC * 4 / 4 }, /* 100% */
  { SCALE_100PC * 5 / 4 }, /* 125% */
  { SCALE_100PC * 6 / 4 }, /* 150% */
  { SCALE_100PC * 7 / 4 }, /* 175% */
  { SCALE_100PC * 8 / 4 }, /* 200% */
  { viewerscale_FIT_TO_SCREEN },
  /* { viewerscale_FIT_TO_WINDOW }, */
  { viewerscale_PRESERVE }
};

static const choices_stringset viewerchoices_scale =
{
  "scale",
  CHOICES_VWR_D_SCALE,
  CHOICES_VWR_P_SCALE,
  NELEMS(viewerchoices_scale_vals),
  viewerchoices_scale_vals
};

static const choices_stringset_vals viewerchoices_steps_vals[] =
{
  { 1 },
  { 5 },
  { 10 },
  { 15 },
  { 20 },
};

static const choices_stringset viewerchoices_steps =
{
  "steps",
  CHOICES_VWR_D_STEPS,
  CHOICES_VWR_P_STEPS,
  NELEMS(viewerchoices_steps_vals),
  viewerchoices_steps_vals
};

static const choices_option viewerchoices_cover_icon_bar =
{
  CHOICES_VWR_O_COVERICONBAR
};

static const choices_choice viewerchoices[] =
{
  { "size",
    offsetof(eye_choices, viewer.size),
    choices_TYPE_STRING_SET,
    viewersize_FIT_TO_IMAGE,
    { .string_set = &viewerchoices_size } },

  { "scale",
    offsetof(eye_choices, viewer.scale),
    choices_TYPE_STRING_SET,
    SCALE_100PC,
    { .string_set = &viewerchoices_scale } },

  { "steps",
    offsetof(eye_choices, viewer.steps),
    choices_TYPE_STRING_SET,
    1, /* "Instant" */
    { .string_set = &viewerchoices_steps } },

  { "covericonbar",
    offsetof(eye_choices, viewer.cover_icon_bar),
    choices_TYPE_OPTION,
    1,
    { .option = &viewerchoices_cover_icon_bar } },

  { "scroll.x",
    offsetof(eye_choices, viewer.scroll_x),
    choices_TYPE_NUMBER_RANGE,
    32, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "scroll.y",
    offsetof(eye_choices, viewer.scroll_y),
    choices_TYPE_NUMBER_RANGE,
    32, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "stage.pasteboard.minsize",
    offsetof(eye_choices, viewer.stage.pasteboard.size),
    choices_TYPE_NUMBER_RANGE,
    32, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "stage.pasteboard.colour",
    offsetof(eye_choices, viewer.stage.pasteboard.colour),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_MID_DARK_GREY,
    { .colour = NULL /* no GUI */ } },

  { "stage.stroke.size",
    offsetof(eye_choices, viewer.stage.stroke.size),
    choices_TYPE_NUMBER_RANGE,
    2, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "stage.stroke.colour",
    offsetof(eye_choices, viewer.stage.stroke.colour),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_BLACK,
    { .colour = NULL /* no GUI */ } },

  { "stage.margin.size",
    offsetof(eye_choices, viewer.stage.margin.size),
    choices_TYPE_NUMBER_RANGE,
    16, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "stage.margin.colour",
    offsetof(eye_choices, viewer.stage.margin.colour),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_TRANSPARENT, /* match content */
    { .colour = NULL /* no GUI */ } },

  { "stage.shadow.size",
    offsetof(eye_choices, viewer.stage.shadow.size),
    choices_TYPE_NUMBER_RANGE,
    8, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "stage.shadow.colour",
    offsetof(eye_choices, viewer.stage.shadow.colour),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_VERY_DARK_GREY,
    { .colour = NULL /* no GUI */ } },
};

static const choices_group_handlers viewerchoices_handlers =
{
  viewer_choices_updated
};

static const choices_group viewerchoices_group =
{
  "viewer",
  NELEMS(viewerchoices),
  viewerchoices,
  choicepane_vwr,
  &viewerchoices_handlers,
};

/* ----------------------------------------------------------------------- */

static const choices_numberrange cache_size =
{
  CHOICES_VWR_D_CACHE,
  CHOICES_VWR_B_CACHEDOWN,
  CHOICES_VWR_B_CACHEUP,
  0, 256 * 1024,
  1024,
  0 /* precision */
};

static const choices_choice cache[] =
{
  { "size",
    offsetof(eye_choices, cache.size),
    choices_TYPE_NUMBER_RANGE,
    0,
    { .number_range = &cache_size } },
};

static const choices_group cache_group =
{
  "cache",
  NELEMS(cache),
  cache,
  choicepane_vwr,
  NULL, /* callbacks */
};

/* ----------------------------------------------------------------------- */

#ifdef EYE_THUMBVIEW

static const choices_stringset_vals thumbview_size_vals[] =
{
  { 0 },
  { 1 },
  { 2 },
};

static const choices_stringset thumbview_size_set =
{
  "thumbview.size", /* "menu.thumbview.size" */
  CHOICES_THM_D_SIZE,
  CHOICES_THM_P_SIZE,
  NELEMS(thumbview_size_vals),
  thumbview_size_vals
};

static const choices_stringset_vals thumbview_item_size_vals[] =
{
  { 0 },
  { 1 },
  { 2 },
};

static const choices_stringset thumbview_item_size_set =
{
  "thumbview.item.size", /* "menu.thumbview.item.size" */
  CHOICES_THM_D_ITEMS,
  CHOICES_THM_P_ITEMS,
  NELEMS(thumbview_item_size_vals),
  thumbview_item_size_vals
};

static const choices_stringset_vals thumbview_padding_size_vals[] =
{
  { 0 },
  { 1 },
  { 2 },
};

static const choices_stringset thumbview_padding_size_set =
{
  "thumbview.padding.size", /* "menu.thumbview.padding.size" */
  CHOICES_THM_D_PADDING,
  CHOICES_THM_P_PADDING,
  NELEMS(thumbview_padding_size_vals),
  thumbview_padding_size_vals
};

static const choices_choice thumbview[] =
{
  /* --- visible --- */

  { "size",
    offsetof(eye_choices, thumbview.size),
    choices_TYPE_STRING_SET,
    0,
    { .string_set = &thumbview_size_set } },

  { "item.size",
    offsetof(eye_choices, thumbview.item_size),
    choices_TYPE_STRING_SET,
    0,
    { .string_set = &thumbview_item_size_set } },

  { "padding.size",
    offsetof(eye_choices, thumbview.padding_size),
    choices_TYPE_STRING_SET,
    0,
    { .string_set = &thumbview_padding_size_set } },

  /* --- hidden --- */

  { "thumbnail.width",
    offsetof(eye_choices, thumbview.thumbnail_w),
    choices_TYPE_NUMBER_RANGE,
    128, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "thumbnail.height",
    offsetof(eye_choices, thumbview.thumbnail_h),
    choices_TYPE_NUMBER_RANGE,
    96, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "item.width",
    offsetof(eye_choices, thumbview.item_w),
    choices_TYPE_NUMBER_RANGE,
    512, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "item.height",
    offsetof(eye_choices, thumbview.item_h),
    choices_TYPE_NUMBER_RANGE,
    256, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "padding.horz",
    offsetof(eye_choices, thumbview.padding_h),
    choices_TYPE_NUMBER_RANGE,
    16, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "padding.vert",
    offsetof(eye_choices, thumbview.padding_v),
    choices_TYPE_NUMBER_RANGE,
    16, /* OS units */
    { .number_range = NULL /* no GUI */ } },
};

static const choices_group thumbview_group =
{
  "thumbview",
  NELEMS(thumbview),
  thumbview,
  choicepane_thm,
  NULL, /* callbacks */
};

#endif /* EYE_THUMBVIEW */

/* ----------------------------------------------------------------------- */

static const choices_option colour_on =
{
  CHOICES_COL_O_ON
};

static const choices_numberrange colour_gamma =
{
  CHOICES_COL_D_GAMMA,
  CHOICES_COL_B_GAMMADOWN,
  CHOICES_COL_B_GAMMAUP,
  10, 290,
  1,
  2
};

static const choices_numberrange colour_contrast =
{
  CHOICES_COL_D_CONTRAST,
  CHOICES_COL_B_CONTRASTDOWN,
  CHOICES_COL_B_CONTRASTUP,
  10, 190,
  1,
  0
};

static const choices_numberrange colour_brightness =
{
  CHOICES_COL_D_BRIGHTNESS,
  CHOICES_COL_B_BRIGHTNESSDOWN,
  CHOICES_COL_B_BRIGHTNESSUP,
  10, 190,
  1,
  0
};

static const choices_choice colour[] =
{
  { "on",
    offsetof(eye_choices, colour.on),
    choices_TYPE_OPTION,
    0, /* off by default */
    { .option = &colour_on } },

  { "gamma",
    offsetof(eye_choices, colour.gamma),
    choices_TYPE_NUMBER_RANGE,
    100,
    { .number_range = &colour_gamma } },

  { "contrast",
    offsetof(eye_choices, colour.contrast),
    choices_TYPE_NUMBER_RANGE,
    100,
    { .number_range = &colour_contrast } },

  { "brightness",
    offsetof(eye_choices, colour.brightness),
    choices_TYPE_NUMBER_RANGE,
    100,
    { .number_range = &colour_brightness } },
};

static const choices_group colour_group =
{
  "colour",
  NELEMS(colour),
  colour,
  choicepane_col,
  NULL, /* callbacks */
};

/* ----------------------------------------------------------------------- */

#ifdef EYE_TAGS

static const choices_stringset_vals tagcloud_display_vals[] =
{
  { 0 },
  { 1 },
  { 2 },
  { 3 },
};

static const choices_stringset tagcloud_display_set =
{
  "tagcloud.display", /* "menu.tagcloud.display" */
  CHOICES_TAG_D_DISPLAY,
  CHOICES_TAG_P_DISPLAY,
  NELEMS(tagcloud_display_vals),
  tagcloud_display_vals
};

static const choices_stringset_vals tagcloud_sort_vals[] =
{
  { 0 },
  { 1 },
};

static const choices_stringset tagcloud_sort_set =
{
  "tagcloud.sort", /* "menu.tagcloud.sort" */
  CHOICES_TAG_D_SORT,
  CHOICES_TAG_P_SORT,
  NELEMS(tagcloud_sort_vals),
  tagcloud_sort_vals
};

static const choices_stringset_vals tagcloud_size_vals[] =
{
  { -1 }, /* Use desktop font size */
  {  8 },
  { 10 },
  { 12 },
  { 14 },
  { 18 },
  { 24 },
  { 36 },
};

static const choices_stringset tagcloud_size_set =
{
  "tagcloud.size", /* "menu.tagcloud.size" */
  CHOICES_TAG_D_SIZE,
  CHOICES_TAG_P_SIZE,
  NELEMS(tagcloud_size_vals),
  tagcloud_size_vals
};

static const choices_stringset_vals tagcloud_scale_vals[] =
{
  {   0 },
  {  50 },
  { 100 },
  { 150 },
  { 200 },
};

static const choices_stringset tagcloud_scale_set =
{
  "tagcloud.scale", /* "menu.tagcloud.scale" */
  CHOICES_TAG_D_SCALE,
  CHOICES_TAG_P_SCALE,
  NELEMS(tagcloud_scale_vals),
  tagcloud_scale_vals
};

static const choices_option tagcloud_selfirst =
{
  CHOICES_TAG_O_SELFIRST
};

static const choices_choice tagcloud[] =
{
  { "display",
    offsetof(eye_choices, tagcloud.display),
    choices_TYPE_STRING_SET,
    0,
    { .string_set = &tagcloud_display_set } },

  { "sort",
    offsetof(eye_choices, tagcloud.sort),
    choices_TYPE_STRING_SET,
    0,
    { .string_set = &tagcloud_sort_set } },

  { "size",
    offsetof(eye_choices, tagcloud.size),
    choices_TYPE_STRING_SET,
    12, /* pt */
    { .string_set = &tagcloud_size_set } },

  { "leading",
    offsetof(eye_choices, tagcloud.leading),
    choices_TYPE_NUMBER_RANGE,
    (int) (1.4 * 256), /* 8.8 fixed point */
    { .number_range = NULL /* no GUI */ } },

  { "padding",
    offsetof(eye_choices, tagcloud.padding),
    choices_TYPE_NUMBER_RANGE,
    16, /* OS units */
    { .number_range = NULL /* no GUI */ } },

  { "scale",
    offsetof(eye_choices, tagcloud.scale),
    choices_TYPE_STRING_SET,
    100, /* percent */
    { .string_set = &tagcloud_scale_set } },

  { "selfirst",
    offsetof(eye_choices, tagcloud.selfirst),
    choices_TYPE_OPTION,
    FALSE,
    { .option = &tagcloud_selfirst } },
};

static const choices_group_handlers tagcloud_handlers =
{
  tags_choices_updated
};

static const choices_group tagcloud_group =
{
  "tagcloud",
  NELEMS(tagcloud),
  tagcloud,
  choicepane_tag,
  &tagcloud_handlers,
};

#endif /* EYE_TAGS */

/* ----------------------------------------------------------------------- */

static const choices_colour sprite_background =
{
  CHOICES_SPR_P_BACKGROUND
};

static const choices_option sprite_load =
{
  CHOICES_SPR_O_LOAD
};

static const choices_stringset_vals sprite_plotflags_vals[] =
{
  { 0 },
  { osspriteop_DITHERED },
};

static const choices_stringset sprite_plotflags =
{
  "sprite",
  CHOICES_SPR_D_DITHERING,
  CHOICES_SPR_P_DITHERING,
  NELEMS(sprite_plotflags_vals),
  sprite_plotflags_vals
};

static const choices_option sprite_usetinct =
{
  CHOICES_SPR_O_TINCT
};

static const choices_choice sprite[] =
{
  { "background",
    offsetof(eye_choices, sprite.background),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_TRANSPARENT,
    { .colour = &sprite_background } },

  { "load",
    offsetof(eye_choices, sprite.load),
    choices_TYPE_OPTION,
    FALSE,
    { .option = &sprite_load } },

  { "plot.flags",
    offsetof(eye_choices, drawable.sprite.plot_flags),
    choices_TYPE_STRING_SET,
    osspriteop_DITHERED,
    { .string_set = &sprite_plotflags } },

  { "tinct",
    offsetof(eye_choices, drawable.sprite.use_tinct),
    choices_TYPE_OPTION,
    TRUE,
    { .option = &sprite_usetinct } },
};

static const choices_group sprite_group =
{
  "sprite",
  NELEMS(sprite),
  sprite,
  choicepane_spr,
  &viewerchoices_handlers, /* update viewers */
};

/* ----------------------------------------------------------------------- */

static const choices_colour jpeg_background =
{
  CHOICES_JPG_P_BACKGROUND
};

static const choices_option jpeg_load =
{
  CHOICES_JPG_O_LOAD
};

static const choices_stringset_vals jpeg_plotflags_vals[] =
{
  { 0 },
  { jpeg_SCALE_DITHERED },
  { jpeg_SCALE_DITHERED | jpeg_SCALE_ERROR_DIFFUSED },
};

static const choices_stringset jpeg_plotflags =
{
  "jpeg", /* "menu.jpeg" */
  CHOICES_JPG_D_DITHERING,
  CHOICES_JPG_P_DITHERING,
  NELEMS(jpeg_plotflags_vals),
  jpeg_plotflags_vals
};

static const choices_option jpeg_sprite =
{
  CHOICES_JPG_O_SPRITE
};

static const choices_stringset_vals jpeg_cleaning_vals[] =
{
  { image_JPEG_CLEANING_IF_REQUIRED },
  { image_JPEG_CLEANING_ALWAYS },
};

static const choices_stringset jpeg_cleaning_set =
{
  "cleanjpeg", /* "menu.cleanjpeg" */
  CHOICES_JPG_D_CLEANING,
  CHOICES_JPG_P_CLEANING,
  NELEMS(jpeg_cleaning_vals),
  jpeg_cleaning_vals
};

static const choices_option jpeg_trim =
{
  CHOICES_JPG_O_TRIM
};

static const choices_choice jpeg[] =
{
  { "background",
    offsetof(eye_choices, jpeg.background),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_TRANSPARENT,
    { .colour = &jpeg_background } },

  { "cleaning",
    offsetof(eye_choices, image.jpeg.cleaning),
    choices_TYPE_STRING_SET,
    image_JPEG_CLEANING_IF_REQUIRED,
    { .string_set = &jpeg_cleaning_set } },

  { "load",
    offsetof(eye_choices, jpeg.load),
    choices_TYPE_OPTION,
    TRUE,
    { .option = &jpeg_load } },

  { "plot.flags",
    offsetof(eye_choices, drawable.jpeg.plot_flags),
    choices_TYPE_STRING_SET,
    jpeg_SCALE_DITHERED | jpeg_SCALE_ERROR_DIFFUSED,
    { .string_set = &jpeg_plotflags } },

  { "sprite",
    offsetof(eye_choices, image.jpeg.sprite),
    choices_TYPE_OPTION,
    FALSE,
    { .option = &jpeg_sprite } },

  { "trim",
    offsetof(eye_choices, image.jpeg.trim),
    choices_TYPE_OPTION,
    TRUE,
    { .option = &jpeg_trim } },
};

static const choices_group jpeg_group =
{
  "jpeg",
  NELEMS(jpeg),
  jpeg,
  choicepane_jpg,
  &viewerchoices_handlers, /* update viewers */
};

/* ----------------------------------------------------------------------- */

static const choices_colour gif_background =
{
  CHOICES_GIF_P_BACKGROUND
};

static const choices_option gif_load =
{
  CHOICES_GIF_O_LOAD
};

static const choices_choice gif[] =
{
  { "background",
    offsetof(eye_choices, gif.background),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_TRANSPARENT,
    { .colour = &gif_background } },

  { "load",
    offsetof(eye_choices, gif.load),
    choices_TYPE_OPTION,
    TRUE,
    { .option = &gif_load } },
};

static const choices_group gif_group =
{
  "gif",
  NELEMS(gif),
  gif,
  choicepane_gif,
  &viewerchoices_handlers, /* update viewers */
};

/* ----------------------------------------------------------------------- */

static const choices_colour png_background =
{
  CHOICES_PNG_P_BACKGROUND
};

static const choices_option png_load =
{
  CHOICES_PNG_O_LOAD
};

static const choices_choice png[] =
{
  { "background",
    offsetof(eye_choices, png.background),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_TRANSPARENT,
    { .colour = &png_background } },

  { "load",
    offsetof(eye_choices, png.load),
    choices_TYPE_OPTION,
    TRUE,
    { .option = &png_load } },
};

static const choices_group png_group =
{
  "png",
  NELEMS(png),
  png,
  choicepane_png,
  &viewerchoices_handlers, /* update viewers */
};

/* ----------------------------------------------------------------------- */

static const choices_colour drawfile_background =
{
  CHOICES_DRW_P_BACKGROUND
};

/* static const choices_numberrange drawfile_border =
{
}; */

static const choices_option drawfile_load =
{
  CHOICES_DRW_O_LOAD
};

static const choices_stringset_vals drawfile_flatness_vals[] =
{
  { drawfile_FLATNESS_COARSE },
  { drawfile_FLATNESS_NORMAL },
  { drawfile_FLATNESS_BEST },
  { drawfile_FLATNESS_AUTO },
};

static const choices_stringset drawfile_flatness_set =
{
  "drawfile", /* "menu.drawfile" */
  CHOICES_DRW_D_FLATNESS,
  CHOICES_DRW_P_FLATNESS,
  NELEMS(drawfile_flatness_vals),
  drawfile_flatness_vals
};

static const choices_choice drawfile[] =
{
  { "background",
    offsetof(eye_choices, drawfile.background),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_WHITE, /* poo */
    { .colour = &drawfile_background } },

  { "border",
    offsetof(eye_choices, image.drawfile.border),
    choices_TYPE_NUMBER_RANGE,
    16,
    { .number_range = NULL /* &drawfile_border */ } },

  { "flatness",
    offsetof(eye_choices, drawable.drawfile.flatness),
    choices_TYPE_STRING_SET,
    drawfile_FLATNESS_NORMAL,
    { .string_set = &drawfile_flatness_set } },

  { "load",
    offsetof(eye_choices, drawfile.load),
    choices_TYPE_OPTION,
    FALSE,
    { .option = &drawfile_load } },
};

static const choices_group drawfile_group =
{
  "drawfile",
  NELEMS(drawfile),
  drawfile,
  choicepane_drw,
  &viewerchoices_handlers, /* update viewers */
};

/* ----------------------------------------------------------------------- */

static const choices_colour artworks_background =
{
  CHOICES_ART_P_BACKGROUND
};

/* static const choices_numberrange artworks_border =
{
}; */

static const choices_option artworks_load =
{
  CHOICES_ART_O_LOAD
};

static const choices_stringset_vals artworks_quality_vals[] =
{
  { artworks_OUTLINE },
  { artworks_SIMPLE },
  { artworks_NORMAL },
  { artworks_ANTI_ALIASED },
};

static const choices_stringset artworks_quality_set =
{
  "artworks", /* "menu.artworks" */
  CHOICES_ART_D_QUALITY,
  CHOICES_ART_P_QUALITY,
  NELEMS(artworks_quality_vals),
  artworks_quality_vals
};

static const choices_choice artworks[] =
{
  { "background",
    offsetof(eye_choices, artworks.background),
    choices_TYPE_COLOUR,
    (int) os_COLOUR_WHITE,
    { .colour = &artworks_background } },

  { "border",
    offsetof(eye_choices, image.artworks.border),
    choices_TYPE_NUMBER_RANGE,
    16,
    { .number_range = NULL /* &artworks_border */ } },

  { "load",
    offsetof(eye_choices, artworks.load),
    choices_TYPE_OPTION,
    FALSE,
    { .option = &artworks_load } },

  { "quality",
    offsetof(eye_choices, drawable.artworks.quality),
    choices_TYPE_STRING_SET,
    artworks_ANTI_ALIASED,
    { .string_set = &artworks_quality_set } },
};

static const choices_group artworks_group =
{
  "artworks",
  NELEMS(artworks),
  artworks,
  choicepane_art,
  &viewerchoices_handlers, /* update viewers */
};

/* ----------------------------------------------------------------------- */

static const choices_choice hist[] =
{
  { "bars",
    offsetof(eye_choices, hist.bars),
    choices_TYPE_NUMBER_RANGE,
    32,
    { .number_range = NULL /* no GUI */ } },
};

static const choices_group hist_group =
{
  "hist",
  NELEMS(hist),
  hist,
  -1, /* no UI */
  NULL, /* callbacks */
};

/* ----------------------------------------------------------------------- */

static const choices_choice info[] =
{
  { "padding",
    offsetof(eye_choices, info.padding),
    choices_TYPE_NUMBER_RANGE,
    128, /* 64 on either side */
    { .number_range = NULL /* no GUI */ } },
};

static const choices_group info_group =
{
  "info",
  NELEMS(info),
  info,
  -1, /* no UI */
  NULL, /* callbacks */
};

/* ----------------------------------------------------------------------- */

#ifdef EYE_META

static const choices_choice metadata[] =
{
  { "bgcolour",
    offsetof(eye_choices, metadata.bgcolour),
    choices_TYPE_NUMBER_RANGE,
    wimp_COLOUR_WHITE, /* wimp_colour */
    { .number_range = NULL /* no GUI */ } },

  { "wrapwidth",
    offsetof(eye_choices, metadata.wrapwidth),
    choices_TYPE_NUMBER_RANGE,
    80, /* columns */
    { .number_range = NULL /* no GUI */ } },

  { "line.height",
    offsetof(eye_choices, metadata.line_height),
    choices_TYPE_NUMBER_RANGE,
    44, /* OS units */
    { .number_range = NULL /* no GUI */ } },
};

static const choices_group metadata_group =
{
  "metadata",
  NELEMS(metadata),
  metadata,
  -1, /* no UI */
  NULL, /* callbacks */
};

#endif /* EYE_META */

/* ----------------------------------------------------------------------- */

static const choices_choice rotate[] =
{
  { "snap",
    offsetof(eye_choices, rotate.snap),
    choices_TYPE_NUMBER_RANGE,
    9, /* degrees */
    { .number_range = NULL /* no GUI */ } },

  { "maxthumb",
    offsetof(eye_choices, rotate.max_thumb),
    choices_TYPE_NUMBER_RANGE,
    100, /* pixels */
    { .number_range = NULL /* no GUI */ } },
};

static const choices_group rotate_group =
{
  "rotate",
  NELEMS(rotate),
  rotate,
  -1, /* no UI */
  NULL, /* callbacks */
};

/* ----------------------------------------------------------------------- */

static const choices_choice scale[] =
{
  { "step",
    offsetof(eye_choices, scale.step),
    choices_TYPE_NUMBER_RANGE,
    1,
    { .number_range = NULL /* no GUI */ } },

  { "mult",
    offsetof(eye_choices, scale.mult),
    choices_TYPE_NUMBER_RANGE,
    10,
    { .number_range = NULL /* no GUI */ } },
};

static const choices_group scale_group =
{
  "scale",
  NELEMS(scale),
  scale,
  -1, /* no UI */
  NULL, /* callbacks */
};


/* ----------------------------------------------------------------------- */

static const choices_choice effects[] =
{
  { "curve.width",
    offsetof(eye_choices, effects.curve_width),
    choices_TYPE_NUMBER_RANGE,
    8, /* Draw units / 256 */
    { .number_range = NULL /* no GUI */ } },
};

static const choices_group effects_group =
{
  "effects",
  NELEMS(effects),
  effects,
  -1, /* no UI */
  NULL, /* callbacks */
};

/* ----------------------------------------------------------------------- */

static const choices_group *groups[] =
{
  /* visible groups */
  &viewerchoices_group,
  &cache_group,
  &colour_group,
#ifdef EYE_TAGS
  &tagcloud_group,
#endif
  &sprite_group,
  &jpeg_group,
  &gif_group,
  &png_group,
  &drawfile_group,
  &artworks_group,

  /* hidden groups */
  &hist_group,
  &info_group,
#ifdef EYE_META
  &metadata_group,
#endif
  &rotate_group,
  &scale_group,
#ifdef EYE_THUMBVIEW
  &thumbview_group,
#endif
  &effects_group,
};

const choices privateeye_choices =
{
  "PrivateEye",
  NELEMS(groups),
  groups,

  &W.choices_w,
  &W.current_choices_xxx_w,
  CHOICES_B_SET,
  CHOICES_B_CANCEL,
  CHOICES_B_SAVE,

  &GLOBALS.choices,
  &GLOBALS.proposed_choices,
  sizeof(eye_choices),

  &W.vars,

  panes,
  NELEMS(panes),
};
