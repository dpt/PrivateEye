/* --------------------------------------------------------------------------
 *    Name: BASIC.h
 * Purpose: Declarations for the BASIC library
 *  Author: David Thomas
 * Version: $Id: basic.h,v 1.1 2010-01-09 22:08:10 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_BASIC_H
#define APPENGINE_BASIC_H

/*
 * DocumentMe
 */
extern int basic_data_open(char *filename);

/*
 * DocumentMe
 */
extern void basic_data_close(void);

/*
 * DocumentMe
 */
extern int basic_data_read(void *buffer_v,
                           int buffer_size);

#endif /* APPENGINE_BASIC_H */
