/* --------------------------------------------------------------------------
 *    Name: errors.h
 * Purpose: AppEngine errors
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_ERRORS_H
#define APPENGINE_ERRORS_H

/* AppEngine result codes extend DPTLib's result_t. */

#include "base/result.h"

/* ----------------------------------------------------------------------- */

#define result_BASE_GENERAL                  	0x8000
#define result_BASE_FILE                  	0x8100
#define result_BASE_SPRITE                	0x8200
#define result_BASE_SPRITEFX              	0x8300
#define result_BASE_KEYMAP                	0x8400
#define result_BASE_TREEVIEW              	0x8500
#define result_BASE_IMAGE_JPEG            	0x8600
#define result_BASE_FONT                  	0x8700
#define result_BASE_THUMBNAIL             	0x8800
#define result_BASE_PRIVATEEYE            	0xFF00

#define result_OS                         	(result_BASE_GENERAL     + 3)
#define result_UNAVAILABLE                	(result_BASE_GENERAL     + 5)

#define result_FILE_OPEN_FAILED           	(result_BASE_FILE        + 0)

#define result_SPRITE_UNSUPP_FUNC         	(result_BASE_SPRITE      + 0)

#define result_SPRITEFX_UNSUPP_EFFECT     	(result_BASE_SPRITEFX    + 0)
#define result_SPRITEFX_UNSUPP_FUNC       	(result_BASE_SPRITEFX    + 1)

#define result_KEYMAP_BAD_KEY             	(result_BASE_KEYMAP      + 0)
#define result_KEYMAP_SYNTAX_ERROR        	(result_BASE_KEYMAP      + 1)
#define result_KEYMAP_UNKNOWN_MODIFIER    	(result_BASE_KEYMAP      + 2)
#define result_KEYMAP_UNKNOWN_ACTION      	(result_BASE_KEYMAP      + 3)
#define result_KEYMAP_NOT_FOUND           	(result_BASE_KEYMAP      + 4)
#define result_KEYMAP_UNKNOWN_SECTION     	(result_BASE_KEYMAP      + 5)

#define result_TREEVIEW_STOP_WALK         	(result_BASE_TREEVIEW    + 0)
#define result_TREEVIEW_FOUND             	(result_BASE_TREEVIEW    + 1)

#define result_IMAGE_JPEG_NO_METADATA     	(result_BASE_IMAGE_JPEG  + 0)

#define result_FONT_NO_MATCH              	(result_BASE_FONT        + 0)

#define result_THUMBNAIL_UNSUPP_FUNC      	(result_BASE_THUMBNAIL   + 0)

#define result_PRIVATEEYE_VIEWER_NOT_FOUND	(result_BASE_PRIVATEEYE  + 0)
#define result_PRIVATEEYE_HIST_UNSUPP_FUNC	(result_BASE_PRIVATEEYE  + 1)
#define result_PRIVATEEYE_META_UNSUPP_FUNC	(result_BASE_PRIVATEEYE  + 2)

/* ----------------------------------------------------------------------- */

/* report an error using Wimp_ReportError */
void result_report(result_t err);

/* throw a fatal error */
void error_fatal(const char *token);
void error_fatal1(const char *token, const char *parameter1);

/* throw a fatal out of memory error */
void error_fatal_oom(void);

#endif /* APPENGINE_ERRORS_H */
