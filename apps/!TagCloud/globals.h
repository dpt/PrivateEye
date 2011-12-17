/* --------------------------------------------------------------------------
 *    Name: globals.h
 * Purpose: Global variables
 * ----------------------------------------------------------------------- */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "oslib/wimp.h"

enum
{
  Flag_Quit = 1 << 0
};

typedef unsigned int Flags;

/* The global variables are grouped together to allow the compiler to use
 * base pointer optimisation. */

extern struct AppGlobals
{
  Flags            flags;

  wimp_menu       *iconbar_m;

  wimp_t           task_handle;

  wimp_version_no  wimp_version;
}
GLOBALS;

#endif /* GLOBALS_H */
