/* --------------------------------------------------------------------------
 *    Name: md5.h
 * Purpose: MD5 digest
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_MD5_H
#define APPENGINE_MD5_H

#include "appengine/base/errors.h"

/* 'digest' must have at least 33 characters of storage. */
error md5__from_file(const char *file_name, char *digest);

#endif /* APPENGINE_MD5_H */
