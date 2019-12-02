/* --------------------------------------------------------------------------
 *    Name: jpeg-meta.c
 * Purpose: JPEG meta data handling, using exiftags library
 * ----------------------------------------------------------------------- */

// TODO
//
// don't yet know how to deal with extracting metadata from a jpeg which has
// been converted to a sprite
// go look in source file, or cache it when converting?
//
// all this mallocing looks painful, might want to use a separate buffer and
// use offsets instead of pointers in the tree
//
// instead of iterating over the file (NELEMS(map)) times we could walk over
// it once and get the segments displayed in file order

#include <stdlib.h>

#include "flex.h"

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "exiftags/exif.h"
#include "exiftags/jpeg.h"

#include "appengine/types.h"
#include "appengine/base/bsearch.h"
#include "appengine/base/bytesex.h"
#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"
#include "appengine/base/pack.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/list.h"
#include "appengine/datastruct/ntree.h"

#include "jpeg.h"

/* ----------------------------------------------------------------------- */

typedef error (*segment_interpreter)(const unsigned char *buf,
                                     int                  len,
                                     ntree_t             *root);

/* ----------------------------------------------------------------------- */

static error jpeg_meta_jfif(const unsigned char *buf,
                            int                  len,
                            ntree_t             *root)
{
  error          err;
  ntree_t       *tree;
  unsigned char *com;

  /* create a new node to hold the comment data */

  err = ntree_new(&tree);
  if (err)
    goto Failure;

  com = malloc(len + 1);
  if (com == NULL)
    goto Failure;

  memcpy(com, buf, len);
  com[len] = '\0'; /* ensure termination */

  ntree_set_data(tree, com);

  /* insert the node into the tree */

  err = ntree_insert(root, ntree_INSERT_AT_END, tree);
  if (err)
    goto Failure;

  return error_OK;


Failure:

  return err;
}

/* ----------------------------------------------------------------------- */

/* Builds a node containing the specified string, but does not link it in
 * anywhere. */
static error str_to_node(const char *str, size_t length, ntree_t **newnode)
{
  error    err;
  ntree_t *node;
  char    *s;

  err = ntree_new(&node);
  if (err)
    goto Failure;

  s = malloc(length + 1); /* add space for terminator */
  if (s == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  memcpy(s, str, length);
  s[length] = '\0';

  ntree_set_data(node, s);

  *newnode = node;

  return error_OK;


Failure:

  return err;
}

static error jpeg_meta_exif_prop(struct exifprop *list,
                                 int              lvl,
                                 ntree_t         *root)
{
  error err;

  if (list == NULL)
    return error_IMAGE_JPEG_NO_METADATA;

  for (; list; list = list->next)
  {
    ntree_t    *parent;
    ntree_t    *child;
    const char *desc;
    size_t      desclen;
    char        buf[12];
    const char *val;
    size_t      vallen;

    /* Take care of point-and-shoot values. */

    if (list->lvl == ED_PAS)
      list->lvl = 1 /* was 'pas' ### */ ? ED_CAM : ED_IMG;

    /* For now, just treat overridden & bad values as verbose. */

    if (list->lvl == ED_OVR || list->lvl == ED_BAD)
      list->lvl = ED_VRB;

    if (list->lvl != lvl)
      continue; /* don't output */

    /* create a heading */

    desc = list->descr ? list->descr : list->name;
    desclen = strlen(desc);

    err = str_to_node(desc, desclen, &parent);
    if (err)
      goto Failure;

    err = ntree_insert(root, ntree_INSERT_AT_END, parent);
    if (err)
      goto Failure;

    /* create a value */

    if (list->str)
    {
      val = list->str;
    }
    else
    {
      sprintf(buf, "%d", list->value);
      val = buf;
    }
    vallen = strlen(val);

    err = str_to_node(val, vallen, &child);
    if (err)
      goto Failure;

    err = ntree_insert(parent, ntree_INSERT_AT_END, child);
    if (err)
      goto Failure;
  }

  return error_OK;


Failure:

  return err;
}

static error jpeg_meta_exif(const unsigned char *buf,
                            int                  len,
                            ntree_t             *root)
{
  static const struct
  {
    const char *name;
    int         lvl;
  }
  map[] =
  {
    { "exif.cam", ED_CAM },
    { "exif.img", ED_IMG },
    { "exif.oth", ED_VRB },
    { "exif.uns", ED_UNK },
  };

  error            err;
  struct exiftags *tags;
  int              i;
  int              emitted = 0;

  /* parse the APP1 segment */

  tags = exifparse((unsigned char *) buf, len);
  if (tags == NULL)
    return error_IMAGE_JPEG_NO_METADATA;

  /* insert the tag data into the tree */

  for (i = 0; i < NELEMS(map); i++)
  {
    ntree_t *subtree;

    /* construct a subtree */

    err = ntree_new(&subtree);
    if (err)
      goto Failure;

    err = jpeg_meta_exif_prop(tags->props, map[i].lvl, subtree);
    if (err)
      goto Failure;

    if (err == error_OK)
    {
      char *s;

      /* insert on success */

      err = ntree_insert(root, ntree_INSERT_AT_END, subtree);
      if (err)
        goto Failure;

      s = str_dup(message0(map[i].name));
      if (s == NULL)
      {
        err = error_OOM;
        goto Failure;
      }

      ntree_set_data(subtree, s);

      emitted = 1;
    }
    else if (err == error_IMAGE_JPEG_NO_METADATA)
    {
      /* discard on failure */

      ntree_delete(subtree);
    }
    else
    {
      break; /* actual error */
    }
  }

  exiffree(tags);

  if (!emitted)
    return error_IMAGE_JPEG_NO_METADATA;

  return error_OK;


Failure:

  return err;
}

/* ----------------------------------------------------------------------- */

#define EIGHTBIM 0x4d494238 /* "8BIM" */

#define ADOBE_ID_MACINTOSH_PRINT_MANAGER_INFO      0x03E9
#define ADOBE_ID_RESOLUTION_INFO                   0x03ED
#define ADOBE_ID_PRINT_FLAGS                       0x03F3
#define ADOBE_ID_COLOUR_HALFTONING_INFORMATION     0x03F5
#define ADOBE_ID_IPTC_NAA_RECORD                   0x0404
#define ADOBE_ID_JPEG_QUALITY                      0x0406
#define ADOBE_ID_GRID_AND_GUIDES_INFORMATION       0x0408
#define ADOBE_ID_COPYRIGHT_FLAG                    0x040A
#define ADOBE_ID_THUMBNAIL_RESOURCE                0x040C
#define ADOBE_ID_GLOBAL_ANGLE                      0x040D
#define ADOBE_ID_DOCUMENT_SPECIFIC_IDS_SEED_NUMBER 0x0414
#define ADOBE_ID_GLOBAL_ALTITUDE                   0x0419
#define ADOBE_ID_CAPTION_DIGEST                    0x0425
#define ADOBE_ID_PRINT_SCALE                       0x0426
#define ADOBE_ID_PRINT_FLAGS_INFORMATION           0x2710

#define IPTC_SEGMENT_MARKER 0x021C // byteswap?

/* IPTC Application Records */

typedef struct
{
  int             number;
  enum { String, Int16U, Digits8, Digits2, Digits1, List } type;
  const char     *name;
}
iptc_record;

static const iptc_record application_record[] =
{
  {   0, Int16U,  "ApplicationRecordVersion"    },
  {   3, String,  "ObjectTypeReference"         },
  {   4, String,  "ObjectAttributeReference"    },
  {   5, String,  "ObjectName"                  },
  {   7, String,  "EditStatus"                  },
  {   8, Digits2, "EditorialUpdate"             },
  {  10, Digits1, "Urgency"                     },
  {  12, List,    "SubjectReference"            },
  {  15, String,  "Category"                    },
  {  20, List,    "SupplementalCategories"      },
  {  22, String,  "FixtureIdentifier"           },
  {  25, String,  "Keywords"                    },
  {  40, String,  "SpecialInstructions"         },
  {  55, Digits8, "DateCreated"                 },
  {  60, String,  "TimeCreated"                 },
  {  80, String,  "By-line"                     },
  {  85, String,  "By-lineTitle"                },
  {  90, String,  "City"                        },
  {  95, String,  "Province-State"              },
  { 100, String,  "Country-PrimaryLocationCode" },
  { 101, String,  "Country-PrimaryLocationName" },
  { 103, String,  "OriginalTransmissionReference" },
  { 105, String,  "Headline"                    },
  { 110, String,  "Credit"                      },
  { 115, String,  "Source"                      },
  { 116, String,  "CopyrightNotice"             },
  { 118, String,  "Contact"                     },
  { 120, String,  "Caption-Abstract"            },
  { 122, String,  "Writer-Editor"               },
  { 232, String,  "ExifCameraInfo"              },
  {  -1, String,  "Unknown"                     },
};

// 183, 240 seen but not understood

typedef error (*adobe_handler)(const unsigned char *buf,
                               size_t               length,
                               ntree_t             *root);

static error jpeg_meta_adobe_iptc_naa_record(const unsigned char *buf,
                                             size_t               length,
                                             ntree_t             *root)
{
  error                err;
  const unsigned char *p;
  unsigned short       size;

  for (p = buf; p < buf + length; p += 2 + 1 + 2 + size)
  {
    unsigned short marker;
    unsigned char  type;
    int            i;
    char           heading[100];
    ntree_t       *parent;
    ntree_t       *child;
    int            c;
    const char    *val;
    size_t         vallen;

    unpack(p, "scs", &marker, &type, &size);

    size = rev_s(size);

    if (marker != IPTC_SEGMENT_MARKER)
      continue;

    i = bsearch_int(&application_record[0].number,
                     NELEMS(application_record) - 1, /* skip last entry */
                     sizeof(application_record[0]),
                     type);
    if (i < 0)
      i = NELEMS(application_record) - 1; /* default */

    /* create a heading */

    c = sprintf(heading, "%s (%d)", application_record[i].name, type);

    err = str_to_node(heading, c, &parent);
    if (err)
      goto Failure;

    err = ntree_insert(root, ntree_INSERT_AT_END, parent);
    if (err)
      goto Failure;

    /* create a value */

    val    = (const char *) p + 2 + 1 + 2;
    vallen = size;

    err = str_to_node(val, vallen, &child);
    if (err)
      goto Failure;

    err = ntree_insert(parent, ntree_INSERT_AT_END, child);
    if (err)
      goto Failure;
  }

  return error_OK;


Failure:

  return err;
}

static error jpeg_meta_adobe_default_handler(const unsigned char *buf,
                                             size_t               length,
                                             ntree_t             *root)
{
  NOT_USED(buf);
  NOT_USED(length);
  NOT_USED(root);

  /* This returns OK so that the parent entry will remain. */

  return error_OK;
}

static ntree_t *jpeg_meta_adobe_parse(const unsigned char *buf,
                                      size_t               length)
{
  static const struct
  {
    unsigned int  id;
    const char   *desc; /* message token */
    adobe_handler fn;
  }
  map[] =
  {
    { ADOBE_ID_MACINTOSH_PRINT_MANAGER_INFO,      "adobe.03e9",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_RESOLUTION_INFO,                   "adobe.03ed",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_PRINT_FLAGS,                       "adobe.03f3",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_COLOUR_HALFTONING_INFORMATION,     "adobe.03f5",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_IPTC_NAA_RECORD,                   "adobe.0404",    jpeg_meta_adobe_iptc_naa_record },
    { ADOBE_ID_JPEG_QUALITY,                      "adobe.0406",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_GRID_AND_GUIDES_INFORMATION,       "adobe.0408",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_COPYRIGHT_FLAG,                    "adobe.040a",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_THUMBNAIL_RESOURCE,                "adobe.040c",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_GLOBAL_ANGLE,                      "adobe.040d",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_DOCUMENT_SPECIFIC_IDS_SEED_NUMBER, "adobe.0414",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_GLOBAL_ALTITUDE,                   "adobe.0419",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_CAPTION_DIGEST,                    "adobe.0425",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_PRINT_SCALE,                       "adobe.0426",    jpeg_meta_adobe_default_handler },
    { ADOBE_ID_PRINT_FLAGS_INFORMATION,           "adobe.2710",    jpeg_meta_adobe_default_handler },
    { 0xFFFFFFFF,                                 "adobe.unknown", jpeg_meta_adobe_default_handler },
  };

  error                err;
  int                  idlen;
  ntree_t             *root;
  char                *s;
  const unsigned char *p;
  int                  blklen;

  /* buf holds the APP13 chunk */

  /* allocate a new root node for our tree and label it with the Photoshop
   * ID string */

  idlen = strlen((const char *) buf) + 1; /* include terminator */

  err = ntree_new(&root);
  if (err)
    goto Failure;

  s = malloc(idlen);
  if (s == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  memcpy(s, buf, idlen);

  ntree_set_data(root, s);

  /* "8BIM" blocks */

  for (p = buf + idlen; p < buf + length; p += blklen)
  {
    unsigned int   type;
    unsigned short id;
    unsigned char  namelen;
    int            namefieldlen;
    unsigned int   segsize;
    ntree_t       *subtree;
    int            i;

//    fprintf(stderr, "8BIM: %d bytes left\n", (buf + length) - p);

    unpack(p, "isc", &type, &id, &namelen);

    /* don't endian swap 'type' */

    id = rev_s(id);

#define PAD2(x) (((x) + 1) & ~1)

    namefieldlen = PAD2(1 + namelen);
    unpack(p + 4 + 2 + namefieldlen, "i", &segsize);

    segsize = rev_l(segsize);

    blklen = 4 + 2 + namefieldlen + 4 + PAD2(segsize);

    if (type != EIGHTBIM)
      continue;

    /* create a sub-tree for each metadata group */

    err = ntree_new(&subtree);
    if (err)
      goto Failure;

    i = bsearch_uint(&map[0].id, NELEMS(map), sizeof(map[0]), id);
    if (i < 0)
      i = NELEMS(map) - 1; /* not found: use the default handler */

    err = map[i].fn(p + 4 + 2 + namefieldlen + 4, blklen, subtree);

    if (err == error_OK)
    {
      char fmt[32];
      char desc[256];
      char desc2[256];
      char heading[256];
      int  used;

      /* insert on success */

      err = ntree_insert(root, ntree_INSERT_AT_END, subtree);
      if (err)
        goto Failure;

      strcpy(desc, message0(map[i].desc));

      if (namelen > 0)
      {
        strcpy(fmt, message0("adobe.format2"));

        memcpy(desc2, (const char *) p + 4 + 2 + 1, namelen);
        desc2[namelen] = '\0';

        used = sprintf(heading, fmt, desc, desc2, id);
      }
      else
      {
        strcpy(fmt, message0("adobe.format"));

        used = sprintf(heading, fmt, desc, id);
      }

      s = malloc(used + 1);
      if (s == NULL)
      {
        err = error_OOM;
        goto Failure;
      }

      memcpy(s, heading, used + 1);

      ntree_set_data(subtree, s);
    }
    else if (err == error_IMAGE_JPEG_NO_METADATA)
    {
      /* discard on failure */

      ntree_delete(subtree);
    }
    else
    {
      break; /* actual error */
    }
  }

  return root;


Failure:

  return NULL;
}

static error jpeg_meta_adobe(const unsigned char *buf,
                             int                  len,
                             ntree_t             *root)
{
  error    err;
  ntree_t *subtree;

  /* parse the APP13 segment */

  subtree = jpeg_meta_adobe_parse(buf, len);
  if (subtree == NULL)
    return error_IMAGE_JPEG_NO_METADATA;

  /* insert the tag data into the tree */

  /* insert on success */

  err = ntree_insert(root, ntree_INSERT_AT_END, subtree);
  if (err)
    goto Failure;

  return error_OK;


Failure:

  return err;
}

/* ----------------------------------------------------------------------- */

static const struct
{
  JPEG_MARKER          marker;
  const char          *name;
  segment_interpreter  fn;
}
jpeg_marker_map[] =
{
  { M_COM,   "jfif",  jpeg_meta_jfif  },
  { M_APP1,  "exif",  jpeg_meta_exif  },
  { M_APP13, "adobe", jpeg_meta_adobe },
};

int jpeg_get_meta(image_t *image, ntree_t **newtree)
{
  error    err;
  ntree_t *tree;
  char    *s;
  int      i;

  *newtree = NULL;

  /* create our tree's root */

  err = ntree_new(&tree);
  if (err)
    goto Failure;

  s = str_dup(message0("metadata"));
  if (s == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  ntree_set_data(tree, s);

  for (i = 0; i < NELEMS(jpeg_marker_map); i++)
  {
    int offset = 0; /* continuation value */

    for (;;)
    {
      const unsigned char *buf;
      int                  len;
      ntree_t             *subtree;

      /* find the required marker */

      jpeg_find(image->image,
                flex_size(&image->image),
                jpeg_marker_map[i].marker,
               &buf,
               &len,
               &offset);

      /* loop until we run out of segments */

      if (buf == NULL || len == 0)
        break; /* try next segment type */

      /* create a sub-tree for each metadata group */

      err = ntree_new(&subtree);
      if (err)
        goto Failure;

      err = jpeg_marker_map[i].fn(buf, len, subtree);

      if (err == error_OK)
      {
        /* insert on success */

        err = ntree_insert(tree, ntree_INSERT_AT_END, subtree);
        if (err)
          goto Failure;

        s = str_dup(message0(jpeg_marker_map[i].name));
        if (s == NULL)
        {
          err = error_OOM;
          goto Failure;
        }

        ntree_set_data(subtree, s);
      }
      else if (err == error_IMAGE_JPEG_NO_METADATA)
      {
        /* discard on failure */

        ntree_delete(subtree);
      }
      else
      {
        break; /* actual error */
      }
    }
  }

  *newtree = tree;

  return 0;


Failure:

  return 1;
}

int jpeg_meta_available(image_t *image)
{
  int i;

  for (i = 0; i < NELEMS(jpeg_marker_map); i++)
  {
    const unsigned char *buf;
    int                  len;
    int                  offset = 0; /* continuation value */

    jpeg_find(image->image,
              flex_size(&image->image),
              jpeg_marker_map[i].marker,
             &buf,
             &len,
             &offset);

    if (buf && len > 0)
      return 1; /* got one - metadata is available */
  }

  return 0; /* no interesting chunks found */
}
