/* --------------------------------------------------------------------------
 *    Name: Internet.h
 * Purpose: Declarations for the Internet library
 *  Author: David Thomas
 * Version: $Id: internet.h,v 1.2 2005-03-06 18:29:44 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_INTERNET_H
#define APPENGINE_INTERNET_H

#ifndef APPENGINE_NO_TCPIP
#include "netdb.h"
#endif /* APPENGINE_NO_TCPIP */

/* MIME.c ---------------------------------------------------------------- */

/*
 * DocumentMe
 */
extern const char *mime_fromfiletype(int filetype);

/*
 * DocumentMe
 */
extern int mime_tofiletype(const char *mimetype);

/* URI.c ----------------------------------------------------------------- */

/*
 * DocumentMe
 */
extern int uri_decode(const char *uri,
                      char *scheme,
                      char *creds,
                      char *host,
                      int *port,
                      char *path);

/*
 * DocumentMe
 */
/*
extern _kernel_oserror *xmimemap_translate(int input_format,
                                           char *input_buffer,
                                           int output_buffer,
                                           char *output_buffer);
*/

#ifndef APPENGINE_NO_TCPIP

/* Resolver2.s ----------------------------------------------------------- */

/*
 * DocumentMe
 */
extern _kernel_oserror *xresolver_get_host_by_name(char *name,
                                                   struct hostent **host_ent);

/*
 * DocumentMe
 */
extern _kernel_oserror *xresolver_get_host_by_addr(char *addr,
                                                   int length,
                                                   int type,
                                                   struct hostent **host_ent);

/*
 * DocumentMe
 */
extern _kernel_oserror *xresolver_get_host(char *name,
                                           struct hostent **host_ent);

#endif /* APPENGINE_NO_TCPIP */

#endif /* APPENGINE_INTERNET_H */

