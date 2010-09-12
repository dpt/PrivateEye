/*
 * Copyright (c) 2002-2005, Eric M. Johnston <emj@postal.net>
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
 * $Id: exifcom.c,v 1.2 2008-02-18 01:08:39 dpt Exp $
 */

/*
 * exifcom: display or set Exif user comment.
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


static const char *version = "1.00";
static int fnum, bflag, iflag, nflag, vflag;
static const char *com;
static const char *delim = ": ";

#define ASCCOM		"ASCII\0\0\0"


/*
 * Display the comment.  This function just uses what's returned by
 * exifscan() -- it doesn't touch the file.
 */
static int
printcom(const char *fname, struct exifprop *p, const unsigned char *btiff)
{
	int rc = 0;

	/* No comment tag. */

	if (!p) {
		fprintf(stderr, "%s: comment not available\n", fname);
		return (1);
	}

	if (vflag)
		printf("Supported Length%s%d\n", delim, p->count - 8);

	/* Comment tag unset. */

	if (!*(btiff + p->value))
		rc = 2;

	else {
		/* Comment tag not ASCII. */

		if (memcmp(ASCCOM, btiff + p->value, 8)) {
			if (!vflag)
				exifwarn2("comment type not supported",
				    (const char *)(btiff + p->value));
			rc = 3;
		}

		if (vflag)
			printf("Type%s%s\n", delim, btiff + p->value);
	}

	/* Comment tag OK, but blank. */

	if (!rc && (!p->str || !*(p->str)))
		rc = 2;

	/* Print comment in the normal, non-verbose case. */

	if (!vflag) {
		printf("%s", rc ? "" : p->str);
		return (rc);
	}

	/* Print length and comment if it's supported. */

	if (rc != 1 && rc != 3) {
		printf("Length%s%d\n", delim, rc ? 0 : (int)strlen(p->str));
		printf("Comment%s%s\n", delim, rc ? "" : p->str);
	}

	return (rc);
}


/*
 * Blank or write a comment.
 */
static int
writecom(FILE *fp, const char *fname, long pos, struct exifprop *p,
    const unsigned char *buf, const unsigned char *btiff)
{
	u_int32_t l;
	int ch, checkch;
	long psave;

	/* No comment tag or it's zero length. */

	if (!p) {
		fprintf(stderr, "%s: comment not available\n", fname);
		return (1);
	}

	if (p->count < 9) {
		fprintf(stderr, "%s: comment size zero\n", fname);
		return (1);
	}

	/* Be careful with existing or unsupported comments. */

	if (iflag && *(btiff + p->value)) {

		if (memcmp(ASCCOM, btiff + p->value, 8)) {
			if (nflag)
				return (1);
			fprintf(stderr, "overwrite %.8s comment in %s? "
			    "(y/n [n]) ", btiff + p->value, fname);

			checkch = ch = getchar();
			while (ch != '\n' && ch != EOF)
				ch = getchar();
			if (checkch != 'y' && checkch != 'Y') {
				fprintf(stderr, "not overwritten\n");
				return (1);
			}

		} else if (p->str && *(p->str)) {
			if (nflag)
				return (1);
			fprintf(stderr, "overwrite comment in %s? (y/n [n]) ",
			    fname);

			checkch = ch = getchar();
			while (ch != '\n' && ch != EOF)
				ch = getchar();
			if (checkch != 'y' && checkch != 'Y') {
				fprintf(stderr, "not overwritten\n");
				return (1);
			}
		}
	}

	/* Remember where we are and move to the comment in our file. */

	psave = ftell(fp);
	if (fseek(fp, pos + (btiff - buf) + p->value, SEEK_SET))
		exifdie((const char *)strerror(errno));

	/*
	 * Blank it with zeros and then reset the position.
	 * XXX Note that this is counter to the spec's recommendation:
	 * "When a UserComment area is set aside, it is recommended that
	 * the ID code be ASCII and that the following user comment part
	 * be filled with blank characters [20.H]."  However, cameras (Canon)
	 * seem to make it all zero; the rationale is that one can restore
	 * the image file to its original state.
	 */

	if (bflag) {
		for (l = 0; l < p->count; l++)
			if (fwrite("\0", 1, 1, fp) != 1)
				exifdie((const char *)strerror(errno));
		if (fseek(fp, pos + (btiff - buf) + p->value, SEEK_SET))
			exifdie((const char *)strerror(errno));
	}

	/* Write the character code and comment. */

	if (com) {
		l = strlen(com);
		if (l > p->count - 8) {
			fprintf(stderr, "%s: truncating comment to fit\n",
			    fname);
			l = p->count - 8;
		}

		/* Character code. */

		if (fwrite(ASCCOM, 8, 1, fp) != 1)
			exifdie((const char *)strerror(errno));

		/* Comment. */

		if (fwrite(com, l, 1, fp) != 1)
			exifdie((const char *)strerror(errno));

		/*
		 * Pad with spaces (this seems to be standard practice).
		 * XXX For now we're not NUL terminating the string.
		 * This doesn't appear to be required by the spec, but it's
		 * always possible that something out there will break.
		 * I've seen some utilities pad with spaces and set the
		 * last byte to NUL.
		 */

		for (l = p->count - 8 - l; l; l--)
			if (fwrite(" ", 1, 1, fp) != 1)
				exifdie((const char *)strerror(errno));
	}

	/* Restore the file pointer. */

	if (fseek(fp, psave, SEEK_SET))
		exifdie((const char *)strerror(errno));

	return (0);
}


/*
 * Scan the JPEG file for Exif data and parse it.
 */
static int
doit(FILE *fp, const char *fname)
{
	int mark, gotapp1, first, rc;
	unsigned int len, rlen;
	unsigned char *exifbuf;
	struct exiftags *t;
	long app1;

	gotapp1 = FALSE;
	first = 0;
	exifbuf = NULL;
	rc = 0;

	while (jpegscan(fp, &mark, &len, !(first++))) {

		if (mark != JPEG_M_APP1) {
			if (fseek(fp, len, SEEK_CUR))
				exifdie((const char *)strerror(errno));
			continue;
		}

		exifbuf = (unsigned char *)malloc(len);
		if (!exifbuf)
			exifdie((const char *)strerror(errno));

		app1 = ftell(fp);
		rlen = fread(exifbuf, 1, len, fp);
		if (rlen != len) {
			fprintf(stderr, "%s: error reading JPEG (length "
			    "mismatch)\n", fname);
			free(exifbuf);
			return (1);
		}

		gotapp1 = TRUE;
		t = exifscan(exifbuf, len, FALSE);

		if (t && t->props) {
			if (bflag || com)
				rc = writecom(fp, fname, app1,
				    findprop(t->props, tags,
				    EXIF_T_USERCOMMENT), exifbuf, t->md.btiff);
			else
				rc = printcom(fname, findprop(t->props, tags,
				    EXIF_T_USERCOMMENT), t->md.btiff);
		} else {
			fprintf(stderr, "%s: couldn't find Exif properties\n",
			    fname);
			rc = 1;
		}
		exiffree(t);
		free(exifbuf);
	}

	if (!gotapp1) {
		fprintf(stderr, "%s: couldn't find Exif data\n", fname);
		return (1);
	}

	return (rc);
}


static
void usage()
{
	fprintf(stderr, "Usage: %s [options] [-w string] file ...\n", progname);
	fprintf(stderr, "Displays or sets Exif comment in the specified "
	    "files.\n");
	fprintf(stderr, "Version: %s\n\n", version);
	fprintf(stderr, "Available options:\n");
	fprintf(stderr, "  -b\tOverwrite the comment tag with zeros.\n");
	fprintf(stderr, "  -f\tAnswer 'yes' to any confirmation prompts.\n");
	fprintf(stderr, "  -i\tConfirm overwrites (default).\n");
	fprintf(stderr, "  -n\tAnswer 'no' to any confirmation prompts.\n");
	fprintf(stderr, "  -v\tBe verbose about comment information.\n");
	fprintf(stderr, "  -w\tSet comment to provided string.\n");
	fprintf(stderr, "  -s\tSet delimiter to provided string "
	    "(default: \": \").\n");

	exit(1);
}


int
main(int argc, char **argv)
{
	register int ch;
	int eval;
	char *rmode, *wmode;
	FILE *fp;

	progname = argv[0];
	eval = 0;
	debug = FALSE;
	bflag = nflag = vflag = FALSE;
	iflag = TRUE;
	com = NULL;
#ifdef WIN32
	rmode = "rb";
	wmode = "r+b";
#else
	rmode = "r";
	wmode = "r+";
#endif

	while ((ch = getopt(argc, argv, "bfinvw:s:")) != -1)
		switch (ch) {
		case 'b':
			bflag = TRUE;
			break;
		case 'f':
			iflag = FALSE;
			break;
		case 'i':
			iflag = TRUE;
			break;
		case 'n':
			iflag = nflag = TRUE;
			break;
		case 'v':
			vflag = TRUE;
			break;
		case 'w':
			com = optarg;
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

	if (!*argv)
		usage();

	for (fnum = 0; *argv; ++argv) {

		/* Only open for read/write if we need to. */

		if ((fp = fopen(*argv,
		    bflag || com ? wmode : rmode)) == NULL) {
			exifwarn2(strerror(errno), *argv);
			eval = 1;
			continue;
		}

		fnum++;

		if (argc > 1) {

			/* Print filenames if more than one. */

			if (vflag && !(bflag || com))
				printf("%s%s:\n",
				    fnum == 1 ? "" : "\n", *argv);
			else if (!(bflag || com))
				printf("%s%s", *argv, delim);

			/* Don't error >1 with multiple files. */

			eval = (doit(fp, *argv) == 1 || eval);

		} else
			eval = doit(fp, *argv);

		if (!vflag && !(bflag || com))
			printf("\n");

		fclose(fp);
	}

	return (eval);
}
