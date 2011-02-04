/* --------------------------------------------------------------------------
 *    Name: awrender.h
 * Purpose: Low level ArtWorks rendering
 *  Author: Tony Houghton
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_AWRENDER_H
#define APPENGINE_AWRENDER_H

#include "kernel.h"

#include "oslib/os.h"

#define awrender_DefaultWorkSpace 512

typedef void *awrender_doc;
typedef void *awrender_handle;

typedef struct
{
  void *resizable_block;
  int resizable_size;
  void *fixed_block;
  int fixed_size;
}
awrender_callback_regs;

typedef _kernel_oserror *(*awrender_callback_handler)(int new_size, /* Or -1 to read */
                                                      awrender_callback_regs *, /* To return R0-R3 */
                                                      awrender_handle);

typedef struct
{
  int    dither_x, dither_y;
  os_box clip_rect;
  int    print_lowx, print_lowy;
  int    print_handle;
}
awrender_info_block;

typedef struct
{
  int        xeig, yeig;
  int        log2bpp;
  os_palette palette[20];
}
awrender_vdu_block;

typedef enum
{
  awrender_OutputToVDU,
  awrender_OutputToPrinter    = 2,
  awrender_OutputToPostScript = 4
}
awrender_output;

#ifdef __cplusplus
extern "C"
{
#endif

extern _kernel_oserror *awrender_file_init(awrender_doc,
                                           awrender_callback_handler,
                                           int /* doc_size */,
                                           awrender_handle);

extern _kernel_oserror *awrender_render(awrender_doc, awrender_info_block *,
                                        os_trfm const *,
                                        awrender_vdu_block const *,
                                        void * /* resizable_block */,
                                        awrender_callback_handler,
                                        int /* wysiwyg */, awrender_output,
                                        awrender_handle);

extern _kernel_oserror *awrender_doc_bounds(awrender_doc, os_box *);

#ifdef __cplusplus
}
#endif

#endif /* APPENGINE_AWRENDER_H */
