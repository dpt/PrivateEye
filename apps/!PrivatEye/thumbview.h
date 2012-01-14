/* --------------------------------------------------------------------------
 *    Name: thumbview.h
 * Purpose: Filer-like thumbnail viewing
 * ----------------------------------------------------------------------- */

#ifndef THUMBVIEW_H
#define THUMBVIEW_H

typedef struct thumbview thumbview;

error thumbview_init(void);
void thumbview_fin(void);
void thumbview_open(thumbview *tv);
error thumbview_create(thumbview **new_tv);
void thumbview_destroy(thumbview *doomed);
void thumbview_load_dir(thumbview *tv, const char *dir_name);
void thumbview_close_all(void);

#endif /* THUMBVIEW_H */
