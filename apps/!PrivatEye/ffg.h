/* --------------------------------------------------------------------------
 *    Name: ffg.h
 * Purpose: Computer Concepts Foreign Format Graphics converters interface
 * ----------------------------------------------------------------------- */

#ifndef FFG_H
#define FFG_H

#include "oslib/types.h"
#include "oslib/wimp.h"

#define message_TRANSLATE_FFG     0x80E1F
#define message_TRANSLATE_FFG_ACK 0x80E20

void ffg_initialise(osbool (*loadable_fn)(bits file_type));
void ffg_finalise(void);
osbool ffg_is_loadable(bits src_file_type);
osbool ffg_convert(const wimp_message *message);
int ffg_apposite(const wimp_message *message);
void ffg_complete(const wimp_message *message);

#endif /* FFG_H */
