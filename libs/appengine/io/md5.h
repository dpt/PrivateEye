/* --------------------------------------------------------------------------
 *    Name: md5.h
 * Purpose: MD5 digest
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_MD5_H
#define APPENGINE_MD5_H

#include "appengine/base/errors.h"

#define md5_DIGESTSZ 16

result_t md5_from_file(const char    *file_name,
                       unsigned char  digest[md5_DIGESTSZ]);

#endif /* APPENGINE_MD5_H */
