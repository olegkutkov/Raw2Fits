

#include <sys/types.h>
#include <libraw/libraw.h>
#include <string.h>
#include <errno.h>
#include <fitsio.h>
#include <time.h>
#include "file_utils.h"
#include "raw2fits.h"

static int decoder_progress_callback(void *data, enum LibRaw_progress p,int iteration, int expected)
{
	converter_params_t *params = (converter_params_t *) data;

	params->logger_msg(params->logger_arg, "\t%s, step %i/%i\n", libraw_strprogress(p), iteration + 1, expected);

	return !params->converter_run;
}

void print_error(converter_params_t *arg, char *err_where, int err)
{
	char *err_descr;

	if (err > 0) {
		err_descr = strerror(err);
	} else {
		switch (err) {
			case LIBRAW_UNSUFFICIENT_MEMORY:
				err_descr = "Attempt to get memory from the system has failed";
				break;

			case  LIBRAW_DATA_ERROR:
				err_descr = "A fatal error emerged during data unpacking";
				break;

			case  LIBRAW_IO_ERROR:
				err_descr = "A fatal error emerged during file reading";
				break;

			case  LIBRAW_CANCELLED_BY_CALLBACK:
				err_descr = "Processing cancelled";
				break;

			case LIBRAW_BAD_CROP:
				err_descr = "Incorrect cropping coordinates are set";
				break;

			case LIBRAW_UNSPECIFIED_ERROR:
				err_descr = "An unknown error, sorry";
				break;

			case LIBRAW_FILE_UNSUPPORTED:
				err_descr = "Unsupported file format";
				break;

			case LIBRAW_REQUEST_FOR_NONEXISTENT_IMAGE:
				err_descr = "Attempt to retrieve a RAW image with a number absent in the data file";
				break;

			case LIBRAW_OUT_OF_ORDER_CALL:
				err_descr = "Incorrect order of API function calls. This is a problem with raw2fits";
				break;

			default:
				err_descr = "Unknown internal error";
				break;
		}
	}

	arg->logger_msg(arg->logger_arg, "%s. %s\n", err_where, err_descr);
}

void set_metadata_from_raw(libraw_data_t *rawdata, libraw_processed_image_t *proc_img, file_metadata_t *dst_meta)
{
	size_t tmplen;
	struct tm *utc_tm;

	if (strlen(dst_meta->instrument) == 0) {
		tmplen = strlen(rawdata->idata.make);
		strcpy(dst_meta->instrument, rawdata->idata.make);
		dst_meta->instrument[tmplen] = 0x20;
		strcpy(dst_meta->instrument + tmplen + 1, rawdata->idata.model);
	}

	if (strlen(dst_meta->observer) == 0) {
		strcpy(dst_meta->observer, rawdata->other.artist);
	}

	if (strlen(dst_meta->date) == 0) {
		utc_tm = gmtime(&rawdata->other.timestamp);
		strftime(dst_meta->date, strlen(dst_meta->date), "%Y-%m-%d %H:%M:%S", utc_tm);
	}

	if (dst_meta->exptime == 0) {
		dst_meta->exptime = rawdata->other.shutter;
	}

	dst_meta->bitpixel = proc_img->bits;
	dst_meta->width = proc_img->width;
	dst_meta->height = proc_img->height;
}

int create_new_fits(fitsfile **fptr, char *filename, int recreate)
{
	int status = 0;
	fitsfile *tmp;

	if (recreate) {
		fits_open_file(&tmp, filename, READONLY, &status);
		status = 0;
		fits_delete_file(tmp, &status);
	}

	status = 0;

	fits_create_file(fptr, filename, &status);

	return status;
}

void close_fits(fitsfile *fptr)
{
	int status = 0;

	fits_flush_file(fptr, &status);
	fits_close_file(fptr, &status);
}

void write_fits_header(fitsfile *fptr, file_metadata_t *meta)
{
	int status = 0;

	fits_write_key(fptr, TSTRING, "OBJECT", meta->object, "", &status);
	fits_write_key(fptr, TSTRING, "OBSERVER", meta->observer, "", &status);
	fits_write_key(fptr, TSTRING, "TELESCOP", meta->telescope, "", &status);
	fits_write_key(fptr, TSTRING, "INSTRUME", meta->instrument, "", &status);
	fits_write_key(fptr, TSTRING, "FILTER", meta->filter, "Filter used when taking image", &status);
	fits_write_key(fptr, TFLOAT, "EXPTIME", &meta->exptime, "Exposure time in seconds", &status);
}


void raw2fits(char *file, converter_params_t *arg)
{
	libraw_decoder_info_t decoder_info;
	libraw_data_t *rawdata;
	libraw_processed_image_t *proc_img;
	char target_filename[512];
	fitsfile *fits;
	long *framebuf;
	int i, err, k = 0;
	int target_file_exists = 0;
	long red, green, blue;

	make_target_fits_filename(arg, file, target_filename);

	target_file_exists = is_file_exist(target_filename);

	if (target_file_exists && !arg->fsetup.overwrite) { 
		arg->logger_msg(arg->logger_arg, "File %s is already exists, skipping...\n", target_filename);
		return;
	}

	rawdata = libraw_init(0);

	if (!rawdata) {
		arg->logger_msg(arg->logger_arg, "Failed to init libraw, err: \n", strerror(errno));
		return;
	}

	err = libraw_open_file(rawdata, file);

	if (err != LIBRAW_SUCCESS) {
		print_error(arg, "Failed to open RAW file", err);
		libraw_close(rawdata);
		return;
	}

	libraw_set_progress_handler(rawdata, &decoder_progress_callback, arg);

	err = libraw_unpack(rawdata);

	if (err != LIBRAW_SUCCESS) {
		print_error(arg, "Failed to unpack RAW file", err);
		libraw_close(rawdata);
		return;
	}

	libraw_get_decoder_info(rawdata, &decoder_info);

	arg->logger_msg(arg->logger_arg, "\tConverting raw image using %s\n", decoder_info.decoder_name);

	err = libraw_raw2image(rawdata);

	if (err != LIBRAW_SUCCESS) {
		print_error(arg, "Failed to extract image", err);
		libraw_recycle(rawdata);
		libraw_close(rawdata);
		return;
	}

	err = libraw_dcraw_process(rawdata);

	if (err != LIBRAW_SUCCESS) {
		print_error(arg, "Dcraw process failed", err);
		libraw_free_image(rawdata);
		libraw_recycle(rawdata);
		libraw_close(rawdata);
		return;
	}

	proc_img = libraw_dcraw_make_mem_image(rawdata, &err);

	libraw_free_image(rawdata);

	if (!proc_img) {
		print_error(arg, "Failed to make mem image", err);
//		libraw_free_image(rawdata);
		libraw_recycle(rawdata);
		libraw_close(rawdata);
		return;
	}

	arg->logger_msg(arg->logger_arg, "\tImage decoded, size = %ix%i, bits = %i, colors = %i\n",
									proc_img->width, proc_img->height, proc_img->bits, proc_img->colors);

	set_metadata_from_raw(rawdata, proc_img, &arg->meta);

	framebuf = (long *) malloc(proc_img->width * proc_img->height * sizeof(long));

	if (!framebuf) {
		arg->logger_msg(arg->logger_arg, "Failed to allocate memory for the frame, err: \n", strerror(err));
		libraw_dcraw_clear_mem(proc_img);
//		libraw_free_image(rawdata);
		libraw_recycle(rawdata);
		libraw_close(rawdata);
	}

	for (i = 0; i < proc_img->width * proc_img->height; i++) {
		red = proc_img->data[k];
		green = proc_img->data[k + 1];
		blue = proc_img->data[k + 2];

		switch (arg->imsetup.mode) {
			case GRAYSCALE:
				framebuf[i] = red + green + blue;
				break;

			case RED_ONLY:
				framebuf[i] = red;
				break;

			case GREEN_ONLY:
				framebuf[i] = green;
				break;

			case BLUE_ONLY:
				framebuf[i] = blue;
				break;

			case ALL_CHANNELS_BY_FILES:
			case ALL_CHANNELS:
				break;
		}

		k += 3;
	}

	err = create_new_fits(&fits, target_filename, target_file_exists);

	arg->logger_msg(arg->logger_arg, "Creating FITS %s status = %i\n", target_filename, err);

	close_fits(fits);

	free(framebuf);

	libraw_dcraw_clear_mem(proc_img);
//	libraw_free_image(rawdata);
	libraw_recycle(rawdata);
	libraw_close(rawdata);
}

/*
typedef struct frame {
	long *grayscale_combined;
	long *red_channel;
	long *blue_channel;
	long *green_channel;
	FRAME_MODE mode;
} frame_t;

void write_fits_header(fitsfile *fptr, file_metadata_t *meta)
{
	int status = 0;

	fits_write_key(fptr, TSTRING, "OBJECT", meta->object, "", &status);
	fits_write_key(fptr, TSTRING, "OBSERVER", meta->observer, "", &status);
	fits_write_key(fptr, TSTRING, "TELESCOP", meta->telescope, "", &status);
	fits_write_key(fptr, TSTRING, "INSTRUME", meta->instrument, "", &status);
	fits_write_key(fptr, TSTRING, "FILTER", meta->filter, "Filter used when taking image", &status);
	fits_write_key(fptr, TFLOAT, "EXPTIME", &meta->exptime, "Exposure time in seconds", &status);
}

void write_fits(libraw_data_t *raw, char *outdir, char *filename, frame_t *frame, file_metadata_t *meta)
{
	int status = 0;
	fitsfile *fptr;
	long fpx[2] = { 1L, 1L };

	unsigned int naxis = 2;
	long naxes[2] = { meta->width, meta->height };

	long *frame_buf = NULL;
	int write_all = 0;


	if (is_file_exist(outname)){
		printf("File \"%s\" is already exists, skipping...\n", outname);
		free(outname);
		return;
	}

	printf("creating fits file: %s\n", outname);

	fits_create_file(&fptr, outname, &status);

	status = 0;

	switch (frame->mode) {
		case GRAYSCALE:
			frame_buf = frame->grayscale_combined;
			break;

		case RED_ONLY:
			frame_buf = frame->red_channel;
			break;

		case BLUE_ONLY:
			frame_buf = frame->blue_channel;
			break;

		case GREEN_ONLY:
			frame_buf = frame->green_channel;
			break;

		case ALL_CHANNELS:
			write_all = 1;
			break;

		default:
			break;
	}

	if (!write_all) {
		fits_create_img(fptr, meta->bitpixel, naxis, naxes, &status);
		write_fits_header(fptr, meta);
		fits_write_pix(fptr, TLONG, fpx, naxes[0] * naxes[1], frame_buf, &status);
	} else {
		fits_create_img(fptr, meta->bitpixel, naxis, naxes, &status);
		write_fits_header(fptr, meta);
		fits_write_comment(fptr, "R channel", &status);
		fits_write_pix(fptr, TLONG, fpx, naxes[0] * naxes[1], frame->red_channel, &status);

		fits_create_img(fptr, meta->bitpixel, naxis, naxes, &status);
		write_fits_header(fptr, meta);
		fits_write_comment(fptr, "G channel", &status);
		fits_write_pix(fptr, TLONG, fpx, naxes[0] * naxes[1], frame->green_channel, &status);

		fits_create_img(fptr, meta->bitpixel, naxis, naxes, &status);
		write_fits_header(fptr, meta);
		fits_write_comment(fptr, "B channel", &status);
		fits_write_pix(fptr, TLONG, fpx, naxes[0] * naxes[1], frame->blue_channel, &status);
	}

	fits_close_file(fptr, &status);

	free(outname);
}

static void convert_one_file(char *file, void *arg)
{
	long int i = 0;

	float *frameGray;//, *frameGreen, *frameBlue, *frameGrayscale;

	frame_t frame_to_fits;

	conv_cb_arg_t *cb_arg = (conv_cb_arg_t *) arg;

	printf("convert_one_file called with arg %s %s\n", file, cb_arg->outdir);


	rawdata->params.no_auto_bright = 1;
	rawdata->params.no_interpolation = 1;
	rawdata->params.no_auto_scale = 1;

	int err = libraw_dcraw_process(rawdata);
	
	libraw_processed_image_t *proc_img = libraw_dcraw_make_mem_image(rawdata, &err);

	//cb_arg->meta->width = rawdata->sizes.width ;// / 2.0;
	//cb_arg->meta->height = rawdata->sizes.height ;// / 2.0;

	printf("Reading RAW file, image size = %ix%i rotate = %i\n"
			, rawdata->sizes.width, rawdata->sizes.height, rawdata->sizes.flip);

	printf("Processed image. size = %ix%i  bits = %i  colors = %i\n", proc_img->width, proc_img->height, proc_img->bits, proc_img->colors);

	cb_arg->meta->width = proc_img->width;
	cb_arg->meta->height = proc_img->height;

//	frameRed = (float *) malloc((rawdata->sizes.raw_width /2.0)*(rawdata->sizes.raw_height /2.0) * sizeof(float));

//	frameGray = (float *) malloc(cb_arg->meta->width * cb_arg->meta->height * sizeof(float));

//	frame_to_fits.red_channel = (float *) malloc(cb_arg->meta->width * cb_arg->meta->height * sizeof(float));
	frame_to_fits.red_channel = (long *) malloc(cb_arg->meta->width * cb_arg->meta->height * sizeof(long));

	//frame_to_fits.green1_channel = (char *) malloc( proc_img->width * proc_img->height * sizeof(char));
//	frame_to_fits.blue_channel = (float *) malloc( proc_img->width * proc_img->height * sizeof(float));
//	frame_to_fits.red_channel = (float *) malloc( proc_img->width * proc_img->height * sizeof(float));

//	frame_to_fits.blue_channel = (float *) malloc(cb_arg->meta->width * cb_arg->meta->height * sizeof(float));
//	frame_to_fits.green2_channel = (float *) malloc(cb_arg->meta->width * cb_arg->meta->height * sizeof(float));

//	frame_to_fits.grayscale_combined = frameGray;
	frame_to_fits.mode = RED_ONLY;//GRAYSCALE;

//
//	for (i = 0; i < rawdata->sizes.iwidth * rawdata->sizes.iheight; i++) {
//		frameGray[i] = rawdata->image[i][0] + rawdata->image[i][1] 
//						+ rawdata->image[i][2] + rawdata->image[i][3];

		//frame_to_fits.green1_channel[i] = rawdata->image[i][1];
//		frame_to_fits.green1_channel[i] = rawdata->image[i][2];

//		frame_to_fits.red_channel[i] = rawdata->image[i][0];
//		frame_to_fits.green1_channel[i] = rawdata->image[i][1];
//		frame_to_fits.blue_channel[i] = rawdata->image[i][2];
//		frame_to_fits.green2_channel[i] = rawdata->image[i][3];
//	}
//

//	for (i = 0; i < cb_arg->meta->width * cb_arg->meta->height; i++) {
//		frame_to_fits.green1_channel[i] = rawdata->image[i][0];
//	}
	


	write_fits(rawdata, cb_arg->outdir, basename(file), &frame_to_fits, cb_arg->meta);

//	free(frameGray);

	libraw_dcraw_clear_mem(proc_img);

	free(frame_to_fits.red_channel);
//	free(frame_to_fits.green_channel);
//	free(frame_to_fits.blue_channel);
//	free(frame_to_fits.green2_channel);

//           printf("i=%d R=%d G=%d B=%d G2=%d\n",
//                        1,
//                        rawdata->image[0][0],
//                        rawdata->image[0][1],
//                        rawdata->image[0][2],
//                        rawdata->image[0][3]
//                );

}
*/

