/* --------------------------------------------------------------------------
 *    Name: filerwin.h
 * Purpose: Filer-style grid window
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_FILERWIN_H
#define APPENGINE_FILERWIN_H

#include "oslib/wimp.h"

#define T filerwin

/* ----------------------------------------------------------------------- */

typedef struct T T;

/* ----------------------------------------------------------------------- */

enum
{
  filerwin_mode_LARGE_ICONS,
  filerwin_mode_SMALL_ICONS,
  filerwin_mode_FULL_INFO,
};

typedef unsigned int filerwin_mode;

enum
{
  filerwin_sort_NAME,
  filerwin_sort_TYPE,
  filerwin_sort_SIZE,
  filerwin_sort_DATE,
};

typedef unsigned int filerwin_sort;

/* ----------------------------------------------------------------------- */

T *filerwin_create(void);

void filerwin_destroy(T *doomed);

typedef void (filerwin_redrawfn)(wimp_draw *redraw,
                                  int        x,
                                  int        y,
                                  int        index,
                                  int        sel,
                                  void      *arg);

typedef void (filerwin_closefn)(wimp_close *close,
                                 void       *arg);

typedef void (filerwin_pointerfn)(wimp_pointer *pointer,
                                   void         *arg);

void filerwin_set_handlers(T *fw,
                            filerwin_redrawfn  *redraw,
                            filerwin_closefn   *close,
                            filerwin_pointerfn *pointer);

void filerwin_set_arg(T *fw, void *arg);

wimp_w filerwin_get_window_handle(T *fw);

void filerwin_set_nobjects(T *fw, int nobjects);
void filerwin_set_padding(T *fw, int hpad, int vpad);
void filerwin_set_dimensions(T *fw, int width, int height);
void filerwin_set_mode(T *fw, filerwin_mode mode);
void filerwin_set_sort(T *fw, filerwin_sort sort);
void filerwin_set_window_title(T *fw, const char *title);

/* Select the specified index, or -1 for all. */
void filerwin_select(T *fw, int i);

/* De-select the specified index, or -1 for all. */
void filerwin_deselect(T *fw, int i);

void filerwin_open(T *fw);

#undef T

#endif /* APPENGINE_FILERWIN_H */
