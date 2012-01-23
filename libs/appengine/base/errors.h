/* --------------------------------------------------------------------------
 *    Name: errors.h
 * Purpose: AppEngine errors
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_ERRORS_H
#define APPENGINE_ERRORS_H

typedef unsigned long int error;

/* ----------------------------------------------------------------------- */

#define error_BASE_GENERAL                      0x0000
#define error_BASE_FILE                         0x0100
#define error_BASE_TAGDB                        0x0200
#define error_BASE_SPRITE                       0x0300
#define error_BASE_SPRITEFX                     0x0400
#define error_BASE_KEYMAP                       0x0500
#define error_BASE_TREEVIEW                     0x0600
#define error_BASE_IMAGE_JPEG                   0x0700
#define error_BASE_FONT                         0x0800
#define error_BASE_ATOM                         0x0900
#define error_BASE_FILENAMEDB                   0x0A00
#define error_BASE_PACKER                       0x0B00
#define error_BASE_LAYOUT                       0x0C00
#define error_BASE_THUMBNAIL                    0x0D00
#define error_BASE_STREAM                       0x0E00
#define error_BASE_PICKLE                       0x0F00
#define error_BASE_HASH                         0x1000
#define error_BASE_PRIVATEEYE                   0xFF00

#define error_OK                                (error_BASE_GENERAL     + 0)
#define error_OOM                               (error_BASE_GENERAL     + 1)
#define error_STOP_WALK                         (error_BASE_GENERAL     + 2)
#define error_OS                                (error_BASE_GENERAL     + 3)
#define error_BAD_ARG                           (error_BASE_GENERAL     + 4)

#define error_FILE_OPEN_FAILED                  (error_BASE_FILE        + 0)

#define error_TAGDB_INCOMPATIBLE                (error_BASE_TAGDB       + 0)
#define error_TAGDB_COULDNT_OPEN_FILE           (error_BASE_TAGDB       + 1)
#define error_TAGDB_SYNTAX_ERROR                (error_BASE_TAGDB       + 2)
#define error_TAGDB_UNKNOWN_ID                  (error_BASE_TAGDB       + 3)
#define error_TAGDB_BUFF_OVERFLOW               (error_BASE_TAGDB       + 4)
#define error_TAGDB_UNKNOWN_TAG                 (error_BASE_TAGDB       + 5)

#define error_SPRITE_UNSUPP_FUNC                (error_BASE_SPRITE      + 0)

#define error_SPRITEFX_UNSUPP_EFFECT            (error_BASE_SPRITEFX    + 0)
#define error_SPRITEFX_UNSUPP_FUNC              (error_BASE_SPRITEFX    + 1)

#define error_KEYMAP_BAD_KEY                    (error_BASE_KEYMAP      + 0)
#define error_KEYMAP_SYNTAX_ERROR               (error_BASE_KEYMAP      + 1)
#define error_KEYMAP_UNKNOWN_MODIFIER           (error_BASE_KEYMAP      + 2)
#define error_KEYMAP_UNKNOWN_ACTION             (error_BASE_KEYMAP      + 3)
#define error_KEYMAP_NOT_FOUND                  (error_BASE_KEYMAP      + 4)
#define error_KEYMAP_UNKNOWN_SECTION            (error_BASE_KEYMAP      + 5)

#define error_TREEVIEW_STOP_WALK                (error_BASE_TREEVIEW    + 0)
#define error_TREEVIEW_FOUND                    (error_BASE_TREEVIEW    + 1)

#define error_IMAGE_JPEG_NO_METADATA            (error_BASE_IMAGE_JPEG  + 0)

#define error_FONT_NO_MATCH                     (error_BASE_FONT        + 0)

#define error_ATOM_SET_EMPTY                    (error_BASE_ATOM        + 0)
#define error_ATOM_NAME_EXISTS                  (error_BASE_ATOM        + 1)
#define error_ATOM_OUT_OF_RANGE                 (error_BASE_ATOM        + 2)

#define error_FILENAMEDB_INCOMPATIBLE           (error_BASE_FILENAMEDB  + 0)
#define error_FILENAMEDB_COULDNT_OPEN_FILE      (error_BASE_FILENAMEDB  + 1)
#define error_FILENAMEDB_SYNTAX_ERROR           (error_BASE_FILENAMEDB  + 2)
#define error_FILENAMEDB_BUFF_OVERFLOW          (error_BASE_FILENAMEDB  + 3)

#define error_PACKER_DIDNT_FIT                  (error_BASE_PACKER      + 0)
#define error_PACKER_EMPTY                      (error_BASE_PACKER      + 1)

#define error_LAYOUT_BUFFER_FULL                (error_BASE_LAYOUT      + 0)

#define error_THUMBNAIL_UNSUPP_FUNC             (error_BASE_THUMBNAIL   + 0)

#define error_STREAM_UNKNOWN_OP                 (error_BASE_STREAM      + 0)
#define error_STREAM_CANT_SEEK                  (error_BASE_STREAM      + 1)
#define error_STREAM_BAD_SEEK                   (error_BASE_STREAM      + 2)

#define error_PICKLE_END                        (error_BASE_PICKLE      + 0)
#define error_PICKLE_SKIP                       (error_BASE_PICKLE      + 1)
#define error_PICKLE_INCOMPATIBLE               (error_BASE_PICKLE      + 2)
#define error_PICKLE_COULDNT_OPEN_FILE          (error_BASE_PICKLE      + 3)
#define error_PICKLE_SYNTAX_ERROR               (error_BASE_PICKLE      + 4)

#define error_HASH_END                          (error_BASE_HASH        + 0)
#define error_HASH_BAD_CONT                     (error_BASE_HASH        + 1)

#define error_PRIVATEEYE_VIEWER_NOT_FOUND       (error_BASE_PRIVATEEYE  + 0)
#define error_PRIVATEEYE_HIST_UNSUPP_FUNC       (error_BASE_PRIVATEEYE  + 1)
#define error_PRIVATEEYE_META_UNSUPP_FUNC       (error_BASE_PRIVATEEYE  + 2)

/* ----------------------------------------------------------------------- */

/* report an error using Wimp_ReportError */
void error_report(error err);

/* throw a fatal error */
void error_fatal(const char *token);
void error_fatal1(const char *token, const char *parameter1);

/* throw a fatal out of memory error */
void error_fatal_oom(void);

#endif /* APPENGINE_ERRORS_H */
