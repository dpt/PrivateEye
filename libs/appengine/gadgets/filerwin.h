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
  filerwin__mode_LARGE_ICONS,
  filerwin__mode_SMALL_ICONS,
  filerwin__mode_FULL_INFO,
};

typedef unsigned int filerwin__mode;

enum
{
  filerwin__sort_NAME,
  filerwin__sort_TYPE,
  filerwin__sort_SIZE,
  filerwin__sort_DATE,
};

typedef unsigned int filerwin__sort;

/* ----------------------------------------------------------------------- */

T *filerwin__create(void);

void filerwin__destroy(T *doomed);

typedef void (filerwin__redrawfn)(wimp_draw *redraw,
                                  int        x,
                                  int        y,
                                  int        index,
                                  int        sel,
                                  void      *arg);

typedef void (filerwin__closefn)(wimp_close *close,
                                 void       *arg);

typedef void (filerwin__pointerfn)(wimp_pointer *pointer,
                                   void         *arg);

void filerwin__set_handlers(T *fw,
                            filerwin__redrawfn  *redraw,
                            filerwin__closefn   *close,
                            filerwin__pointerfn *pointer);

void filerwin__set_arg(T *fw, void *arg);

wimp_w filerwin__get_window_handle(T *fw);

void filerwin__set_nobjects(T *fw, int nobjects);
void filerwin__set_padding(T *fw, int hpad, int vpad);
void filerwin__set_dimensions(T *fw, int width, int height);
void filerwin__set_mode(T *fw, filerwin__mode mode);
void filerwin__set_sort(T *fw, filerwin__sort sort);
void filerwin__set_window_title(T *fw, const char *title);

/* Select the specified index, or -1 for all. */
void filerwin__select(T *fw, int i);

/* De-select the specified index, or -1 for all. */
void filerwin__deselect(T *fw, int i);

void filerwin__open(T *fw);

#undef T

#endif /* APPENGINE_FILERWIN_H */
