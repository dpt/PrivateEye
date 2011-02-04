/* --------------------------------------------------------------------------
 *    Name: pointer.h
 * Purpose: Pointer
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_POINTER_H
#define APPENGINE_POINTER_H

#include "kernel.h"

#include "oslib/os.h"
#include "oslib/wimp.h"

/* Sets the pointer to use the shape taken from sprite identified by 'name'.
 * First the local window sprite pool is searched. If not found, the Wimp
 * pool is searched. x,y are the active point. */
void set_pointer_shape(const char *name, int x, int y);

/* Restores the pointer shape */
void restore_pointer_shape(void);

#endif /* APPENGINE_POINTER_H */
