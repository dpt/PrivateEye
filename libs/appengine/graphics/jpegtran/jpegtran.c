/* --------------------------------------------------------------------------
 *    Name: jpegtran.c
 * Purpose: JPEG cleaner
 * ----------------------------------------------------------------------- */

#include <setjmp.h>

#include "flex.h"

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/hourglass.h"

#include "jpeg/jinclude.h"
#include "jpeg/jpeglib.h"
#include "jpeg/jerror.h"
#include "jpeg/transupp.h"

#include "appengine/graphics/jpegtran.h"

/* ----------------------------------------------------------------------- */

enum { OUTPUT_BUF_SIZE = 256 * 1024 }; /* initial allocation */

typedef struct
{
  struct jpeg_destination_mgr pub; /* public fields */

  JOCTET    *buffer;
  size_t     length;

  /* output */
  JOCTET   **newbuffer;
  size_t    *newlength;
}
destination_mgr;

typedef destination_mgr *dest_ptr;

METHODDEF(void) init_destination(j_compress_ptr cinfo)
{
  dest_ptr dest = (dest_ptr) cinfo->dest;
  size_t   length;

  length = OUTPUT_BUF_SIZE * SIZEOF(JOCTET);

  /* Allocate the output buffer */
  if (flex_alloc((flex_ptr) &dest->buffer, length) == 0)
    return;

  dest->length = length;

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

METHODDEF(boolean) empty_output_buffer(j_compress_ptr cinfo)
{
  dest_ptr dest = (dest_ptr) cinfo->dest;

  /* doubling strategy */
  if (flex_extend((flex_ptr) &dest->buffer, dest->length * 2) == 0)
    return TRUE;

  /* fprintf(stderr, "+ %p %d\n", dest->pub.next_output_byte, dest->pub.free_in_buffer); */

  /* this didn't work:
  dest->pub.next_output_byte += biggerbuffer - dest->buffer;
  dest->pub.free_in_buffer += biggerlength - dest->length; */

  /* but this version appears to: */
  /* is free_in_buffer potentially lying here? when we're called the buffer
   * is actually empty, but free_in_buffer indicates otherwise (because it
   * would seem to have not been updated yet */
  dest->pub.next_output_byte = dest->buffer + dest->length;
  dest->pub.free_in_buffer = dest->length * 2 - dest->length;

  /* fprintf(stderr, "- %p %d\n", dest->pub.next_output_byte, dest->pub.free_in_buffer); */

  dest->length *= 2;

  return TRUE;
}

METHODDEF(void) term_destination(j_compress_ptr cinfo)
{
  dest_ptr dest = (dest_ptr) cinfo->dest;
  int      length;

  length = dest->length - dest->pub.free_in_buffer;

  /* shrink to fit */
  if (flex_extend((flex_ptr) &dest->buffer, length) == 0)
    return;

  flex_reanchor((flex_ptr) dest->newbuffer, (flex_ptr) &dest->buffer);
  *dest->newlength = length;
}

static void jpeg_flexmem_dest(j_compress_ptr cinfo,
                              JOCTET       **newbuffer,
                              size_t        *newlength)
{
  dest_ptr dest;

  if (cinfo->dest == NULL)
  {
    cinfo->dest = (*cinfo->mem->alloc_small)(
                              (j_common_ptr) cinfo,
                                             JPOOL_PERMANENT,
                                             SIZEOF(destination_mgr));
  }

  dest = (dest_ptr) cinfo->dest;

  dest->pub.init_destination    = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination    = term_destination;

  dest->newbuffer = newbuffer;
  dest->newlength = newlength;
}

/* ----------------------------------------------------------------------- */

struct my_progress_mgr
{
  struct jpeg_progress_mgr pub;  /* fields known to JPEG library */
  int    completed_extra_passes; /* extra passes completed */
  int    total_extra_passes;     /* total extra */
  /* last printed percentage stored here to avoid multiple printouts */
  int    percent_done;
};

typedef struct my_progress_mgr *my_progress_ptr;

METHODDEF(void) progress_monitor(j_common_ptr cinfo)
{
  my_progress_ptr prog = (my_progress_ptr) cinfo->progress;
  int             total_passes;
  int             percent_done;

  total_passes = prog->pub.total_passes + prog->total_extra_passes;
  percent_done = (int) (prog->pub.pass_counter * 100L / prog->pub.pass_limit);

  if (percent_done == prog->percent_done)
    return;

  prog->percent_done = percent_done;

  /* I'm unsure whether we'll be using the multiple pass code in practice.
   * IIRC, it mainly happens when there's a second quantization stage like
   * when converting to a GIF. */
  if (total_passes > 1)
  {
    /* prog->pub.completed_passes + prog->completed_extra_passes + 1 */
    hourglass_leds(prog->pub.completed_passes, 0);
  }

  hourglass_percentage(percent_done);
}

static void start_progress_monitor(j_common_ptr cinfo, my_progress_ptr progress)
{
  /* Enable progress display, unless trace output is on */
  if (cinfo->err->trace_level != 0)
    return;

  hourglass_on();

  progress->pub.progress_monitor   = progress_monitor;
  progress->completed_extra_passes = 0;
  progress->total_extra_passes     = 0;
  progress->percent_done           = -1;

  cinfo->progress = &progress->pub;
}

static void end_progress_monitor(j_common_ptr cinfo)
{
  if (cinfo->err->trace_level != 0)
    return;

  hourglass_off();
}

/* ----------------------------------------------------------------------- */

static struct
{
  char *buf;
  int   max;
  int   cur;
}
msgbuf =
{
  NULL, 0, 0
};

METHODDEF(void) my_output_message(j_common_ptr cinfo)
{
  char     *next;
  /* keep at least JMSG_LENGTH_MAX bytes free in the buffers */
  const int grow = JMSG_LENGTH_MAX + 2; /* account for separator */
  int       newmax;

  next = msgbuf.buf + msgbuf.cur;

  if (msgbuf.buf != NULL)
  {
    /* add a separator */

    *next++ = ',';
    msgbuf.cur++;

    *next++ = ' ';
    msgbuf.cur++;
  }

  newmax = msgbuf.cur + grow;
  if (msgbuf.max < newmax)
  {
    char *newbuf;

    newbuf = realloc(msgbuf.buf, newmax);
    if (newbuf == NULL)
      return; /* out of memory */

    msgbuf.buf = newbuf;
    msgbuf.max = newmax;

    next = msgbuf.buf + msgbuf.cur;
  }

  /* Create the message */
  (*cinfo->err->format_message)(cinfo, next);
  msgbuf.cur += strlen(next);
}

void jpegtran_discard_messages(void)
{
  free(msgbuf.buf);
  msgbuf.buf = NULL;
  msgbuf.max = 0;
  msgbuf.cur = 0;
}

const char *jpegtran_get_messages(void)
{
  return msgbuf.buf;
}

/* ----------------------------------------------------------------------- */

struct my_error_mgr
{
  struct jpeg_error_mgr pub;
  jmp_buf               setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

/* ----------------------------------------------------------------------- */

static int jpegtran(const unsigned char    *data,
                    size_t                  length,
                    unsigned char         **newbuffer,
                    size_t                 *newlength,
                    jpegtran_transform_type args)
{
  static const JXFORM_CODE map[] =
  {
    JXFORM_NONE,      /* no transformation                */
    JXFORM_ROT_270,   /* 270-degree clockwise (or 90 ccw) */
    JXFORM_ROT_180,   /* 180-degree rotation              */
    JXFORM_ROT_90,    /* 90-degree clockwise rotation     */
    JXFORM_FLIP_H,    /* horizontal flip                  */
    JXFORM_TRANSPOSE, /* transpose across UL-to-LR axis   */
    JXFORM_FLIP_V,    /* vertical flip                    */
    JXFORM_TRANSVERSE /* transpose across UR-to-LL axis   */
  };

  enum { MaxMem = 64 * 1024 * 1024 };

  /* This needs to be volatile to avoid potential clobbering by setjmp. */
  volatile int                  init;
  struct my_error_mgr           jsrcerr;
  struct jpeg_decompress_struct srcinfo;
  struct my_error_mgr           jdsterr;
  struct jpeg_compress_struct   dstinfo;
  jvirt_barray_ptr             *src_coef_arrays;
  jvirt_barray_ptr             *dst_coef_arrays;
  struct my_progress_mgr        progress;
  JCOPY_OPTION                  copyoption;
  jpeg_transform_info           transformoption;

  jpegtran_discard_messages();

  init = 0;

  /* Initialize the JPEG decompression object */

  if (setjmp(jsrcerr.setjmp_buffer))
    goto Failure;

  srcinfo.err = jpeg_std_error(&jsrcerr.pub);
  jsrcerr.pub.error_exit = my_error_exit;
  jsrcerr.pub.output_message = my_output_message;
  /* srcinfo.err->trace_level += 1; */
  jpeg_create_decompress(&srcinfo);

  init++;

  /* Initialize the JPEG compression object */

  if (setjmp(jdsterr.setjmp_buffer))
    goto Failure;

  dstinfo.err = jpeg_std_error(&jdsterr.pub);
  jdsterr.pub.error_exit = my_error_exit;
  jdsterr.pub.output_message = my_output_message;
  /* dstinfo.err->trace_level += 1; */
  jpeg_create_compress(&dstinfo);

  init++;

  /* set options */
  copyoption = JCOPYOPT_ALL;
  transformoption.transform = map[args & 0x07];
  transformoption.trim = (args & jpegtran_TRANSFORM_FLAG_TRIM) != 0;
  transformoption.perfect = FALSE;
  transformoption.force_grayscale = FALSE;
  transformoption.crop = FALSE;
  srcinfo.mem->max_memory_to_use = MaxMem;
  dstinfo.mem->max_memory_to_use = MaxMem;
  dstinfo.optimize_coding = FALSE; /* no need yet */

  /* progress monitor monitors destination progress only */
  start_progress_monitor((j_common_ptr) &dstinfo, &progress);

  /* Specify data source for decompression */
  jpeg_mem_src(&srcinfo, (unsigned char *) data, length);

  /* Enable saving of extra markers that we want to copy */
  jcopy_markers_setup(&srcinfo, copyoption);

  /* Read file header */
  (void) jpeg_read_header(&srcinfo, TRUE);

  /* Any space needed by a transform option must be requested before
   * jpeg_read_coefficients so that memory allocation will be done right.
   */
  jtransform_request_workspace(&srcinfo, &transformoption);

  /* Read source file as DCT coefficients */
  src_coef_arrays = jpeg_read_coefficients(&srcinfo);

  /* Initialize destination compression parameters from source values */
  jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

  /* Adjust destination parameters if required by transform options;
   * also find out which set of coefficient arrays will hold the output.
   */
  dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
                                                 src_coef_arrays,
                                                 &transformoption);

  /* Specify data destination for compression */
  jpeg_flexmem_dest(&dstinfo, newbuffer, newlength);

  /* Start compressor (note no image data is actually written here) */
  jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

  /* Copy to the output file any extra markers that we want to preserve */
  jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);

  /* Execute image transformation, if any */
  jtransform_execute_transformation(&srcinfo, &dstinfo,
                                    src_coef_arrays,
                                    &transformoption);

  /* Finish compression and release memory */
  jpeg_finish_compress(&dstinfo);
  jpeg_destroy_compress(&dstinfo);
  (void) jpeg_finish_decompress(&srcinfo);
  jpeg_destroy_decompress(&srcinfo);

  end_progress_monitor((j_common_ptr) &dstinfo);

  return 0;


Failure:

  if (init >= 2)
  {
    jpeg_destroy_compress(&dstinfo);
    end_progress_monitor((j_common_ptr) &dstinfo);
  }

  if (init >= 1)
  {
    jpeg_destroy_decompress(&srcinfo);
  }

  return 1;
}

/* ----------------------------------------------------------------------- */

int jpegtran_clean(const unsigned char *data,
                   size_t               length,
                   unsigned char      **newdata,
                   size_t              *newlength)
{
  return jpegtran(data, length, newdata, newlength, jpegtran_TRANSFORM_NONE);
}

/* ----------------------------------------------------------------------- */

int jpegtran_transform(const unsigned char    *data,
                       size_t                  length,
                       unsigned char         **newdata,
                       size_t                 *newlength,
                       jpegtran_transform_type args)
{
  return jpegtran(data, length, newdata, newlength, args);
}
