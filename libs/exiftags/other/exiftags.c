/*
 * Copyright (c) 2001-2005, Eric M. Johnston <emj@postal.net>
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
 * $Id: exiftags.c,v 1.2 2008-02-18 01:08:39 dpt Exp $
 */

/*
 * exiftags: dump Exif information embedded in JPEG images.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* For getopt(). */

#if defined(WIN32) || defined(__riscos)
extern char *optarg;
extern int optind, opterr, optopt;
int getopt(int, char * const [], const char *);
#else
#include <unistd.h>
#endif

#include "jpeg.h"
#include "exif.h"


static int quiet;
static const char *version = "1.00";
static int fnum;
static const char *delim = ": ";


static void
printprops(struct exifprop *list, unsigned short lvl, int pas)
{
	static int prevf = -1;
	const char *n;

	if (!quiet) {
		if (prevf == fnum)
			printf("\n");
		else
			prevf = fnum;

		switch (lvl) {
		case ED_UNK:
			printf("Unsupported Properties:\n\n");
			break;
		case ED_CAM:
			printf("Camera-Specific Properties:\n\n");
			break;
		case ED_IMG:
			printf("Image-Specific Properties:\n\n");
			break;
		case ED_VRB:
			printf("Other Properties:\n\n");
			break;
		}
	}

	while (list) {

		/* Take care of point-and-shoot values. */

		if (list->lvl == ED_PAS)
			list->lvl = pas ? ED_CAM : ED_IMG;

		/* For now, just treat overridden & bad values as verbose. */

		if (list->lvl == ED_OVR || list->lvl == ED_BAD)
			list->lvl = ED_VRB;

		if (list->lvl == lvl) {
			n = list->descr ? list->descr : list->name;
			if (list->str)
				printf("%s%s%s\n", n, delim, list->str);
			else
				printf("%s%s%d\n", n, delim, list->value);
		}

		list = list->next;
	}
}


static int
doit(FILE *fp, int dumplvl, int pas)
{
	int mark, gotexif, first;
	unsigned int len, rlen;
	unsigned char *exifbuf;
	struct exiftags *t;

	gotexif = FALSE;
	first = 0;
	exifbuf = NULL;

	while (jpegscan(fp, &mark, &len, !(first++))) {

		if (mark != JPEG_M_APP1) {
			if (fseek(fp, len, SEEK_CUR))
				exifdie((const char *)strerror(errno));
			continue;
		}

		exifbuf = (unsigned char *)malloc(len);
		if (!exifbuf)
			exifdie((const char *)strerror(errno));

		rlen = fread(exifbuf, 1, len, fp);
		if (rlen != len) {
			exifwarn("error reading JPEG (length mismatch)");
			free(exifbuf);
			return (1);
		}

		t = exifparse(exifbuf, len);

		if (t && t->props) {
			gotexif = TRUE;

			if (dumplvl & ED_CAM)
				printprops(t->props, ED_CAM, pas);
			if (dumplvl & ED_IMG)
				printprops(t->props, ED_IMG, pas);
			if (dumplvl & ED_VRB)
				printprops(t->props, ED_VRB, pas);
			if (dumplvl & ED_UNK)
				printprops(t->props, ED_UNK, pas);
		}
		exiffree(t);
		free(exifbuf);
	}

	if (!gotexif) {
		exifwarn("couldn't find Exif data");
		return (1);
	}

	return (0);
}


static
void usage()
{
	fprintf(stderr, "Usage: %s [options] [files]\nDisplays Exif data "
	    "from the specified files or standard input.\n", progname);
	fprintf(stderr, "Version: %s\n\n", version);
	fprintf(stderr, "Available options:\n");
	fprintf(stderr, "  -a\tDisplay camera-specific, image-specific, "
	    "and verbose properties.\n");
	fprintf(stderr, "  -c\tDisplay camera-specific properties.\n");
	fprintf(stderr, "  -i\tDisplay image-specific properties.\n");
	fprintf(stderr, "  -v\tDisplay verbose properties.\n");
	fprintf(stderr, "  -u\tDisplay unknown/unsupported properties.\n");
	fprintf(stderr, "  -l\tCamera has a removable lens.\n");
	fprintf(stderr, "  -d\tDisplay parse debug information.\n");
	fprintf(stderr, "  -q\tSuppress section headers.\n");
	fprintf(stderr, "  -s\tSet delimiter to provided string "
	    "(default: \": \").\n");

	exit(1);
}


int
main(int argc, char **argv)
{
	register int ch;
	int dumplvl, pas, eval;
	char *mode;
	FILE *fp;

	progname = argv[0];
	dumplvl = eval = 0;
	debug = quiet = FALSE;
	pas = TRUE;
#ifdef WIN32
	mode = "rb";
#else
	mode = "r";
#endif

	while ((ch = getopt(argc, argv, "acivuldqs:")) != -1)
		switch (ch) {
		case 'a':
			dumplvl |= (ED_CAM | ED_IMG | ED_VRB);
			break;
		case 'c':
			dumplvl |= ED_CAM;
			break;
		case 'i':
			dumplvl |= ED_IMG;
			break;
		case 'v':
			dumplvl |= ED_VRB;
			break;
		case 'u':
			dumplvl |= ED_UNK;
			break;
		case 'l':
			pas = FALSE;
			break;
		case 'd':
			debug = TRUE;
			break;
		case 'q':
			quiet = TRUE;
			break;
		case 's':
			delim = optarg;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (!dumplvl && !debug)
		dumplvl |= (ED_CAM | ED_IMG);

	if (*argv) {
		for (fnum = 0; *argv; ++argv) {
			if ((fp = fopen(*argv, mode)) == NULL) {
				exifwarn2(strerror(errno), *argv);
				eval = 1;
				continue;
			}

			fnum++;

			/* Print filenames if more than one. */

			if (argc > 1)
				printf("%s%s:\n", fnum == 1 ? "" : "\n", *argv);

			if (doit(fp, dumplvl, pas))
				eval = 1;
			fclose(fp);
		}
        } else {
		if (doit(stdin, dumplvl, pas))
			eval = 1;
	}

	exit(eval);
}
