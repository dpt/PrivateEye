/*
 * Copyright (c) 2004, 2005, Eric M. Johnston <emj@postal.net>
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
 * $Id: exiftime.c,v 1.2 2008-02-18 01:08:39 dpt Exp $
 */

/*
 * exiftime: display or adjust date & time Exif tags; list files
 * ordered by time.
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
#include "timevary.h"


struct linfo {
	const char *fn;
	time_t ts;
};

static const char *version = "1.00";
static int iflag, lflag, qflag, wflag, ttags;
static const char *delim = ": ";
static const char *fname;
static struct vary *v;
static struct linfo *lorder;

#define EXIFTIMEFMT	"%Y:%m:%d %H:%M:%S"
#define EXIFTIMELEN	20

#define ET_CREATE	0x01
#define ET_GEN		0x02
#define ET_DIGI		0x04


/*
 * Some helpful info...
 */
static
void usage()
{
	fprintf(stderr, "Usage: %s [options] file ...\n", progname);
	fprintf(stderr, "Displays or adjusts Exif timestamps in the specified "
	    "files.\n");
	fprintf(stderr, "Version: %s\n\n", version);
	fprintf(stderr, "Available options:\n");
	fprintf(stderr, "  -f\tAnswer 'yes' to any confirmation prompts.\n");
	fprintf(stderr, "  -i\tConfirm overwrites (default).\n");
	fprintf(stderr, "  -l\tList files ordered by image created timestamp; "
	    "overrides -t, -v, -w.\n");
	fprintf(stderr, "  -q\tBe quiet when writing timestamps.\n");
	fprintf(stderr, "  -s\tSet delimiter to provided string "
	    "(default: \": \").\n");
	fprintf(stderr, "  -t[acdg]\n\tDisplay/adjust specified timestamp(s), "
	    "if they exist:\n");
	fprintf(stderr, "\t  a: All available timestamps (default);\n\t  "
	    "c: Image created;\n\t  g: Image generated; or\n\t  d: Image "
	    "digitized.\n");
	fprintf(stderr, "  -v[+|-]val[ymwdHMS]\n\tAdjust the timestamp(s) by "
	    "the given amount.\n");
	fprintf(stderr, "  -w\tWrite adjusted timestamp(s).\n");

	vary_destroy(v);
	exit(1);
}


/*
 * Compare two linfo members for our sort.
 */
static int
lcomp(const void *a, const void *b)
{

	if (((struct linfo *)a)->ts < ((struct linfo *)b)->ts)
		return (-1);
	if (((struct linfo *)a)->ts == ((struct linfo *)b)->ts)
		return (0);
	return (1);
}


/*
 * Stuff a standard Exif timestamp into a struct *tm.
 * I've got to say that it's pretty annoying that Win32 doesn't have
 * strptime()...
 */
static int
etstrptm(const char *buf, struct tm *tp)
{
	int n;

	memset(tp, 0, sizeof(struct tm));
	n = sscanf(buf, "%d:%d:%d %d:%d:%d", &tp->tm_year, &tp->tm_mon,
	    &tp->tm_mday, &tp->tm_hour, &tp->tm_min, &tp->tm_sec);
	tp->tm_year -= 1900;
	tp->tm_mon -= 1;

	return (n != 6);
}


/*
 * Grab the timestamps for listing.  Doesn't modify the file.
 */
static int
listts(struct exiftags *t, struct linfo *li)
{
	struct exifprop *p;
	struct tm tv;

	/*
	 * Try for DateTime, DateTimeOriginal, then DateTimeDigitized.
	 * If none found, print error and list first.
	 */

	p = findprop(t->props, tags, EXIF_T_DATETIME);

	if (!p || !p->str || etstrptm(p->str, &tv)) {
		p = findprop(t->props, tags, EXIF_T_DATETIMEORIG);

		if (!p || !p->str || etstrptm(p->str, &tv)) {
			p = findprop(t->props, tags, EXIF_T_DATETIMEDIGI);

			if (!p || !p->str || etstrptm(p->str, &tv)) {
				exifwarn("no timestamp available");
				li->ts = 0;
				return (1);
			}
		}
	}

	li->ts = mktime(&tv);
	return (0);
}


/*
 * Grab the specified timestamp and vary it, if necessary.
 * The provided buffer must be at least EXIFTIMELEN bytes.
 */
static int
ettime(char *b, struct exifprop *p)
{
	struct tm tv;
	const struct vary *badv;

	/* Slurp the timestamp into tv. */

	if (!p || !p->str || etstrptm(p->str, &tv))
		return (1);

	/* Apply any adjustments.  (Bad adjustment = fatal.) */

	badv = vary_apply(v, &tv);
	if (badv) {
		exifwarn2("cannot apply adjustment", badv->arg);
		usage();
	}

	if (strftime(b, EXIFTIMELEN, EXIFTIMEFMT, &tv) != EXIFTIMELEN - 1)
		return (1);

	return (0);
}


/*
 * Overwrite a timestamp with the adjusted value.
 */
static int
writets(FILE *fp, long pos, struct exiftags *t, struct exifprop *p,
    const unsigned char *buf, const char *ttype, const char *nts)
{
	int ch, checkch;
	long psave;

	/* Some sanity checking. */

	if (strlen(nts) != EXIFTIMELEN - 1) {
		fprintf(stderr, "%s: invalid timestamp -- %s\n", fname, nts);
		return (1);
	}

	if (!strcmp(nts, p->str)) {
		fprintf(stderr, "%s: new %s timestamp identical to old\n",
		    fname, ttype);
		return (1);
	}

	/* Prompt user, if desired. */

	if (iflag) {
		fprintf(stderr, "adjust time %s in %s from\n  %s to %s? "
		    "(y/n [n]) ", ttype, fname, p->str, nts);
		checkch = ch = getchar();
		while (ch != '\n' && ch != EOF)
			ch = getchar();
		if (checkch != 'y' && checkch != 'Y') {
			fprintf(stderr, "not adjusted\n");
			return (1);
		}
	}

	/* Remember where we are and move to the comment in our file. */

	psave = ftell(fp);
	if (fseek(fp, pos + (t->md.btiff - buf) + p->value, SEEK_SET))
		exifdie((const char *)strerror(errno));

	/* Write the new timestamp. */

	if (fwrite(nts, EXIFTIMELEN, 1, fp) != 1)
		exifdie((const char *)strerror(errno));

	/* Restore the file pointer. */

	if (fseek(fp, psave, SEEK_SET))
		exifdie((const char *)strerror(errno));

	if (!qflag)
		printf("%s%s%s -> %s\n", p->descr, delim, p->str, nts);
	return (0);
}


/*
 * Process a timestamp.
 * Returns 0 on success, 1 if it had trouble writing, 2 if the tag wasn't
 * found (and it was explictly requested), or -1 if the tag wasn't found.
 */
static int
procts(FILE *fp, long pos, struct exiftags *t, const unsigned char *buf,
    u_int16_t tag, const char *ttype)
{
	int rc;
	char nts[EXIFTIMELEN];
	struct exifprop *p;

	p = findprop(t->props, tags, tag);
	if (ettime(nts, p)) {

		/*
		 * If ttags != 0, then the user explicitly requested the
		 * timestamp, so print an error if it doesn't exist.
		 */
		if (ttags) {
			fprintf(stderr, "%s: image %s time not available\n",
			    fname, ttype);
			return (2);
		}

		return (-1);
	}

	if (wflag)
		rc = writets(fp, pos, t, p, buf, ttype, nts);
	else {
		printf("%s%s%s\n", p->descr, delim, nts);
		rc = 0;
	}

	return (rc);
}


/*
 * Process the file's timestamps according to the options chosen.
 */
static int
procall(FILE *fp, long pos, struct exiftags *t, const unsigned char *buf)
{
	int r, rc, found;

	found = rc = 0;

	/*
	 * If ttags = 0, process them all or an error if there are none.
	 */

	if (ttags & ET_CREATE || !ttags) {
		r = procts(fp, pos, t, buf, EXIF_T_DATETIME, "created");
		if (r == 0 || r == 1) found++;
		if (r > 0) rc = 1;
	}

	if (ttags & ET_GEN || !ttags) {
		r = procts(fp, pos, t, buf, EXIF_T_DATETIMEORIG, "generated");
		if (r == 0 || r == 1) found++;
		if (r > 0) rc = 1;
	}

	if (ttags & ET_DIGI || !ttags) {
		r = procts(fp, pos, t, buf, EXIF_T_DATETIMEDIGI, "digitized");
		if (r == 0 || r == 1) found++;
		if (r > 0) rc = 1;
	}

	/* No timestamp tags. */

	if (!ttags && !found) {
		fprintf(stderr, "%s: no timestamps available\n", fname);
		return (1);
	}

	return (rc);
}


/*
 * Scan the JPEG file for Exif data and parse it.
 */
static int
doit(FILE *fp, int n)
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

		t = exifscan(exifbuf, len, FALSE);

		if (t && t->props) {
			gotapp1 = TRUE;
			if (lflag)
				rc = listts(t, &lorder[n]);
			else
				rc = procall(fp, app1, t, exifbuf);
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


int
main(int argc, char **argv)
{
	register int ch;
	int eval, fnum, wantall;
	char *rmode, *wmode;
	FILE *fp;

	progname = argv[0];
	debug = FALSE;
	ttags = wantall = eval = 0;
	lflag = qflag = wflag = FALSE;
	iflag = TRUE;
	v = NULL;
#ifdef WIN32
	rmode = "rb";
	wmode = "r+b";
#else
	rmode = "r";
	wmode = "r+";
#endif

	while ((ch = getopt(argc, argv, "filqs:t:v:w")) != -1)
		switch (ch) {
		case 'f':
			iflag = FALSE;
			break;
		case 'i':
			iflag = TRUE;
			break;
		case 'l':
			lflag = TRUE;
			break;
		case 'q':
			qflag = TRUE;
			break;
		case 's':
			delim = optarg;
			break;
		case 't':
			while (*optarg) {
				switch (*optarg) {
				case 'c':
					ttags |= ET_CREATE;
					break;
				case 'd':
					ttags |= ET_DIGI;
					break;
				case 'g':
					ttags |= ET_GEN;
					break;
				case 'a':
					wantall = 1;
					break;
				default:
					usage();
				}
				optarg++;
			}
			if (wantall) ttags = 0;
			break;
		case 'v':
			v = vary_append(v, optarg);
			break;
		case 'w':
			wflag = TRUE;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (!*argv)
		usage();

	/* Don't be quiet if we're not writing. */

	if (qflag && !wflag)
		qflag = FALSE;

	/* Prepare an array for sorting. */

	if (lflag) {
		wflag = 0;
		lorder = (struct linfo *)calloc(argc, sizeof(struct linfo));
		if (!lorder)
			exifdie((const char *)strerror(errno));
	}

	/* Run through the files... */

	for (fnum = 0; *argv; ++argv, fnum++) {

		fname = *argv;

		/* Only open for read+write if we need to. */

		if ((fp = fopen(*argv, wflag ? wmode : rmode)) == NULL) {
			exifwarn2(strerror(errno), *argv);
			eval = 1;
			continue;
		}

		/* Print filenames if more than one. */

		if (argc > 1 && !lflag && !qflag)
			printf("%s%s:\n", fnum == 0 ? "" : "\n", *argv);

		if (lflag)
			lorder[fnum].fn = fname;

		if (doit(fp, fnum))
			eval = 1;
		fclose(fp);
	}

	/*
	 * We'd like to use mergesort() here (instead of qsort()) because
	 * qsort() isn't stable w/members that compare equal and we exepect
	 * the list of files to frequently already be in order.  However,
	 * FreeBSD seems to be the only platform with mergesort(), and I'm
	 * not inclined to include the function...
	 */
	if (lflag) {
		qsort(lorder, argc, sizeof(struct linfo), lcomp);
		for (fnum = 0; fnum < argc; fnum++)
			printf("%s\n", lorder[fnum].fn);
		free(lorder);	/* XXX Over in usage()? */
	}

	vary_destroy(v);
	return (eval);
}
