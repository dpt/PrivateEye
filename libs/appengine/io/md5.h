/* --------------------------------------------------------------------------
 *    Name: md5.h
 * Purpose: MD5 digest
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_MD5_H
#define APPENGINE_MD5_H

#include "appengine/base/errors.h"

#define MD5DIGESTSZ 16

error md5__from_file(const char    *file_name,
                     unsigned char  digest[MD5DIGESTSZ]);

#endif /* APPENGINE_MD5_H */
