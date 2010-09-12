/*
 * Copyright (c) 2001-2003, Eric M. Johnston <emj@postal.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Eric M. Johnston.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: exifutil.c,v 1.3 2008-02-18 01:08:39 dpt Exp $
 */

/*
 * Utilities for dealing with Exif data.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "exif.h"
#include "exifint.h"


/*
 * Some global variables we all need.
 */

int debug;
const char *progname = "exiftags";


/*
 * Logging and error functions.
 */
void
exifdie(const char *msg)
{

	fprintf(stderr, "%s: %s\n", progname, msg);
	exit(1);
}

void
exifwarn(const char *msg)
{

	fprintf(stderr, "%s: %s\n", progname, msg);
}

void
exifwarn2(const char *msg1, const char *msg2)
{

	fprintf(stderr, "%s: %s (%s)\n", progname, msg1, msg2);
}


/*
 * Read an unsigned 2-byte int from the buffer.
 */
u_int16_t
exif2byte(unsigned char *b, enum byteorder o)
{

	if (o == BIG)
		return ((b[0] << 8) | b[1]);
	else
		return ((b[1] << 8) | b[0]);
}


/*
 * Read a signed 2-byte int from the buffer.
 */
int16_t
exif2sbyte(unsigned char *b, enum byteorder o)
{

	if (o == BIG)
		return ((b[0] << 8) | b[1]);
	else
		return ((b[1] << 8) | b[0]);
}


/*
 * Read an unsigned 4-byte int from the buffer.
 */
u_int32_t
exif4byte(unsigned char *b, enum byteorder o)
{

	if (o == BIG)
		return ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
	else
		return ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]);
}


/*
 * Write an unsigned 4-byte int to a buffer.
 */
void
byte4exif(u_int32_t n, unsigned char *b, enum byteorder o)
{
	int i;

	if (o == BIG)
		for (i = 0; i < 4; i++)
			b[3 - i] = (unsigned char)((n >> (i * 8)) & 0xff);
	else
		for (i = 0; i < 4; i++)
			b[i] = (unsigned char)((n >> (i * 8)) & 0xff);
}


/*
 * Read a signed 4-byte int from the buffer.
 */
int32_t
exif4sbyte(unsigned char *b, enum byteorder o)
{

	if (o == BIG)
		return ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
	else
		return ((b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]);
}


/*
 * Lookup and allocate description for a value.
 */
char *
finddescr(struct descrip *table, u_int16_t val)
{
	int i;
	char *c;

	for (i = 0; table[i].val != -1 && table[i].val != val; i++);
	if ((c = (char *)malloc(strlen(table[i].descr) + 1)) == NULL)
		exifdie((const char *)strerror(errno));
	strcpy(c, table[i].descr);
	return (c);
}


/*
 * Lookup and append description for a value.
 * Doesn't do anything if the value is unknown; first adds ", " if dest
 * contains a value; returns number of bytes added.  len is total size
 * of destination buffer.
 */
int
catdescr(char *c, struct descrip *table, u_int16_t val, int len)
{
	int i, l;

	l = 0;
	len -= 1;
	c[len] = '\0';

	for (i = 0; table[i].val != -1 && table[i].val != val; i++);
	if (table[i].val == -1)
		return (0);

	if (strlen(c)) {
		strncat(c, ", ", len - strlen(c));
		l += 2;
	}
	strncat(c, table[i].descr, len - strlen(c));
	l += strlen(table[i].descr);
	return (l);
}


/*
 * Lookup a property entry belonging to a particular set of tags.
 */
struct exifprop *
findprop(struct exifprop *prop, struct exiftag *tagset, u_int16_t tag)
{

	for (; prop && (prop->tagset != tagset || prop->tag != tag);
	    prop = prop->next);
	return (prop);
}


/*
 * Allocate memory for an Exif property.
 */
struct exifprop *
newprop(void)
{
	struct exifprop *prop;

	prop = (struct exifprop *)malloc(sizeof(struct exifprop));
	if (!prop)
		exifdie((const char *)strerror(errno));
	memset(prop, 0, sizeof(struct exifprop));
	return (prop);
}


/*
 * Given a parent, create a new child Exif property.  These are
 * typically used by maker note modules when a single tag may contain
 * multiple items of interest.
 */
struct exifprop *
childprop(struct exifprop *parent)
{
	struct exifprop *prop;

	prop = newprop();

	/* By default, the child inherits most values from its parent. */

	prop->tag = parent->tag;
	prop->type = TIFF_UNKN;
	prop->name = parent->name;
	prop->descr = parent->descr;
	prop->lvl = parent->lvl;
	prop->ifdseq = parent->ifdseq;
	prop->par = parent;
	prop->next = parent->next;

	/* Now insert the new property into our list. */

	parent->next = prop;

	return (prop);
}


/*
 * Allocate a buffer for a property's display string.
 */
void
exifstralloc(char **str, int len)
{

	if (*str) {
		exifwarn("tried to alloc over non-null string");
		abort();
	}
	if ((*str = (char *)calloc(1, len)) == NULL)
		exifdie((const char *)strerror(errno));
}


/*
 * Print hex values of a buffer.
 */
void
hexprint(unsigned char *b, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf(" %02X", b[i]);
}


/*
 * Print debug info for a property.
 */
void
dumpprop(struct exifprop *prop, struct field *afield)
{
	int i;

	if (!debug) return;

	for (i = 0; ftypes[i].type && ftypes[i].type != prop->type; i++);
	if (afield) {
		printf("   %s (0x%04X): %s, %d; %d\n", prop->name,
		    prop->tag, ftypes[i].name, prop->count,
		    prop->value);
		printf("      ");
		hexprint(afield->tag, 2);
		printf(" |");
		hexprint(afield->type, 2);
		printf(" |");
		hexprint(afield->count, 4);
		printf(" |");
		hexprint(afield->value, 4);
		printf("\n");
	} else
		printf("   %s (0x%04X): %s, %d; %d, 0x%04X\n",
		    prop->name, prop->tag, ftypes[i].name,
		    prop->count, prop->value, prop->value);
}


/*
 * Allocate and read an individual IFD.  Takes the beginning and end of the
 * Exif buffer, returns the IFD and an offset to the next IFD.
 */
u_int32_t
readifd(u_int32_t offset, struct ifd **dir, struct exiftag *tagset,
    struct tiffmeta *md)
{
	u_int32_t ifdsize;
	unsigned char *b;

	b = md->btiff;

	/*
	 * Verify that we have a valid offset.  Some maker note IFDs prepend
	 * a string and will screw us up otherwise (e.g., Olympus).
	 * (Number of directory entries is in the first 2 bytes.)
	 */

	if (b + offset + 2 > md->etiff) {
		*dir = NULL;
		return (0);
	}

	*dir = (struct ifd *)malloc(sizeof(struct ifd));
	if (!*dir)
		exifdie((const char *)strerror(errno));

	(*dir)->num = exif2byte(b + offset, md->order);
	(*dir)->par = NULL;
	(*dir)->tagset = tagset;
	(*dir)->md = *md;
	(*dir)->next = NULL;

	ifdsize = (*dir)->num * sizeof(struct field);
	b += offset + 2;

	/* Sanity check our sizes. */

	if (b + ifdsize > md->etiff) {
		free(*dir);
		*dir = NULL;
		return (0);
	}

	/* Point to our array of fields. */

	(*dir)->fields = (struct field *)b;

	/*
	 * While we're here, find the offset to the next IFD.
	 *
	 * Note that this offset isn't always going to be valid.  It
	 * seems that some camera implementations of Exif ignore the spec
	 * and do not include the offset for all IFDs (e.g., maker note).
	 * Therefore, it may be necessary to call readifd() directly (in
	 * leiu of readifds()) to avoid problems when reading these non-
	 * standard IFDs.
	 */

	return ((b + ifdsize + 4 > md->etiff) ? 0 :
	    exif4byte(b + ifdsize, md->order));
}


/*
 * Read a chain of IFDs.  Takes the IFD offset and returns the first
 * node in a chain of IFDs.  Note that it can return NULL.
 */
struct ifd *
readifds(u_int32_t offset, struct exiftag *tagset, struct tiffmeta *md)
{
	struct ifd *firstifd, *curifd;

	/*
	 * XXX Note: we really should be checking to see if this IFD
	 * has already been visited...
	 */

	/* Fetch our first one. */

	offset = readifd(offset, &firstifd, tagset, md);
	curifd = firstifd;

	/* Fetch any remaining ones. */

	while (offset) {
		offset = readifd(offset, &(curifd->next), tagset, md);
		curifd = curifd->next;
	}
	return (firstifd);
}


/*
 * Euclid's algorithm to find the GCD.
 */
u_int32_t
exiftags_gcd(u_int32_t a, u_int32_t b)
{

	if (!b) return (a);
	return (exiftags_gcd(b, a % b));
}
