/* --------------------------------------------------------------------------
 *    Name: thumbview.h
 * Purpose: Filer-like thumbnail viewing
 * ----------------------------------------------------------------------- */

#ifndef THUMBVIEW_H
#define THUMBVIEW_H

typedef struct thumbview thumbview;

error thumbview__init(void);
void thumbview__fin(void);
void thumbview_open(thumbview *tv);
error thumbview_create(thumbview **new_tv);
void thumbview_destroy(thumbview *doomed);
void thumbview_load_dir(thumbview *tv, const char *dir_name);
void thumbview__close_all(void);

#endif /* THUMBVIEW_H */
