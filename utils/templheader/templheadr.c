/**
 * Nettle utility: Templates icon name extractor (generates .h file)
 * (C) Nettle developers 2000-2004
 *
 * $Id: templheadr,v 1.8 2004/02/25 20:32:38 gerph Exp $
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define VARIABLEPREFIX  ""
#define WINDOWICONINFIX "_"
#define ENUMPREFIX      "ICONS_"
#define TRANSFORM       toupper

/* Template file format is taken from RO3 PRM p4-455 */

typedef struct tagFILEHEADER
{
  int fontoffset;
  int reserved[3];
} FILEHEADER;

typedef struct tagINDEXENTRY
{
  int offset;
  int datasize;
  int entrytype;
  char identifier[12];
} INDEXENTRY;

typedef struct tagSIMPLEWINDOW
{
  int windowdata[22];
} SIMPLEWINDOW;

typedef struct tagICON
{
  int minx;
  int miny;
  int maxx;
  int maxy;
  unsigned int flags;
  /* The next three are a specific case, but we're only concerned with that case! */
  int text;        /* offset, for a template file */
  int validation;  /* offset, for a template file */
  int text_len;
} ICON;

/* Icon flags things */
#define INDIRTEXT (1 | (1<<8))
#define DELETED   (1<<23)


/****************************************************************************
 *
 * Locate a "N" tag in an icon validation string, and return its value
 *
\***************************************************************************/

static char *find_validation_name(char *validation)
{
  static char buf[100];

  int want=1;

  while (*validation >= ' ')  /* Got to end of string? */
  {
    if (want==1) /* Are we at the start of a term? */
    {
      if ((*validation == 'N') || (*validation == 'n'))  /* Is it a "N" tag? */
      {
        int i=0;
        validation++;
        for (;*validation >= ' ' && *validation!=';';)  /* copy the name */
        {
          if (*validation=='\\')
            validation++;
          buf[i++]=TRANSFORM(*validation++);
        }

        buf[i]='\0';
        return &buf[0];
      }
      else
        want=0;  /* wasn't an 'N' tag, so skip to next term */
    }
    else
    {
      if (*validation == '\\')  /* Ensure we obey any escaped ";" we may encounter */
        validation++;
      else
        if (*validation == ';')  /* End of a term, so flag a check for next time round */
          want=1;
    }

    validation++;
  }

  return NULL;
}


/****************************************************************************
 *
 * Check a window for icons, if any, output any indirected+text validation
 *  string info.
 *
\***************************************************************************/

static int dowindow(FILE *header, char *identifier, char *windowptr)
{
  char windowname[13];
  SIMPLEWINDOW *win;
  ICON *ic;
  int i = 0, j = 0;
  int outputflag = 1;

  for (; i<13; i++)
  {
    int id = identifier[i];

    if (id < ' ')
    {
      windowname[i] = '\0';
      break;
    }
    else
    {
      windowname[i] = TRANSFORM(id);
    }
  }
  windowname[12] = '\0';

  win = (SIMPLEWINDOW *)(void *) windowptr;
  ic = (ICON *)(void *) (windowptr+sizeof(SIMPLEWINDOW));

  i=win->windowdata[21]; /* number of icons on this window */

  if (i==0)
    return 0;  /* No icons! */

  /* printf("Window '%s' has %d icons\n", windowname, i); */

  /* Loop through those icons! */

  for (; j<i; j++, ic++)
  {
    if ((ic->flags & INDIRTEXT) == INDIRTEXT)
    {
      if ((ic->flags & DELETED)==0)
      {
        if (ic->validation)
        {
          char *name = find_validation_name(windowptr + ic->validation);
          if (name)
          {
            if (outputflag)
            {
              outputflag=0;
              fprintf(header, "enum " ENUMPREFIX  "%s {\n", &windowname[0]);
            }
            else
              fprintf(header, ",\n");
            fprintf(header, "  " VARIABLEPREFIX "%s" \
              WINDOWICONINFIX "%s = %d", &windowname[0], name, j);
          }
        }
      }
    }
  }

  if (outputflag==0)
  {
    /* that means we've output something! */
    fprintf(header, "\n};\n\n\n");
  }

  return 0;
}



/****************************************************************************
 *
 * Read templatefile and call handler function for each window in it
 *
\***************************************************************************/

static int template2header(char *templatefile, char *headerfile)
{
  FILE *file_template = NULL;
  FILE *file_header = NULL;
  char *inbuf;
  int insize, rinsize;
  FILEHEADER *fh;
  INDEXENTRY *ie;

  file_template = fopen(templatefile, "rb");
  if (!file_template)
  {
    fprintf(stderr, "Could not open template file '%s'\r\n", templatefile);
    return 1;
  }

  fseek(file_template, 0, SEEK_END);
  insize = (int) ftell(file_template);
  fseek(file_template, 0, SEEK_SET);

  rinsize = (insize + 3) & ~3;

  inbuf = (char *) malloc(rinsize*2);
  if (!inbuf)
  {
    fclose(file_template);
    fprintf(stderr, "Could not allocate %d bytes at line %d\r\n", rinsize*2, __LINE__);
    return 1;
  }

  if (fread(inbuf+rinsize, 1, insize, file_template) != insize)
  {
    fclose(file_template);
    fprintf(stderr, "Failed to read all %d bytes from file at line %d\r\n", insize, __LINE__);
  }
  fclose(file_template);

  file_header = fopen(headerfile, "w");
  if (!file_header)
  {
    fprintf(stderr, "Failed to open '%s' for writing\r\n", headerfile);
    return 1;
  }

  fh = (FILEHEADER *)(void *) (inbuf+rinsize);

  /* Check the header. First word is offset to font data, or -1 for no fonts */

  if (fh->fontoffset != -1)
    printf("WARNING! Template file contains font information!\r\n");

  ie = (INDEXENTRY *)(void *) (((char *)fh) + sizeof(FILEHEADER));

  fprintf(file_header, "/*\n");
  fprintf(file_header, " * AUTO GENERATED CODE - DO NOT EDIT BY HAND!\n");
  fprintf(file_header, " *\n");
  fprintf(file_header, " * (C) Copyright, Nettle developers 2004\n");
  fprintf(file_header, " */\n\n\n");

  /* Now the index entries. */

  for(; ie->offset; ie++)
  {
    if (ie->entrytype == 1 /* window */)
    {
      memcpy(inbuf, inbuf + rinsize + ie->offset, ie->datasize);
      if (dowindow(file_header, &ie->identifier[0], inbuf))
      {
        fclose(file_header);
        return 1;
      }
    }
    else
      printf("WARNING! Non-window object found in template file!\r\n");
  }

  fclose(file_header);
  return 0;
}



/****************************************************************************
 *
 *  Program parameter checking.
 *
\***************************************************************************/

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "Syntax: %s <templatefile> <headerfile>\r\n", argv[0]);
    return 1;
  }

  printf("Template to .h converter version 0.02. (C) Nettle developers, 2004\r\n");

  return template2header(argv[1], argv[2]);
}
