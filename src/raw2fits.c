/* 
   raw3fits.c
    - raw to fits conversion routines

   Copyright 2017  Oleg Kutkov <elenbert@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
 */

#include <sys/types.h>
#include <libraw/libraw.h>
#include <string.h>
#include <errno.h>
#include <fitsio.h>
#include <time.h>
#include "file_utils.h"
#include "raw2fits.h"
#include "coords_calc.h"
#include "version.h"

static char *FILENAME_CHANNEL_POSTFIX[6] = {
	"_AVG_GRAY.fits\0",
	".fits\0",
	"_RGB.fits\0",
	"_RED.fits\0",
	"_GREEN.fits\0",
	"_BLUE.fits\0"
};

static FRAME_MODE FRAME_COPY_MODES[3] = {
	RED_ONLY,
	GREEN_ONLY,
	BLUE_ONLY
};

static char *FITS_HEADER_COMMENT[6] = {
	"Average grayscale",
	"All channels by files",
	"All channels in one file",
	"RED channel",
	"GREEN channel",
	"BLUE channel"
};

static int decoder_progress_callback(void *data, enum LibRaw_progress p,int iteration, int expected)
{
	converter_params_t *params = (converter_params_t *) data;

	params->logger_msg(params->logger_arg, "\t%s, step %i/%i\n", libraw_strprogress(p), iteration + 1, expected);

	return !params->converter_run;
}

static void print_error(converter_params_t *arg, char *err_where, int err)
{
	const char *err_descr;

	if (err > 0) {
		err_descr = strerror(err);
	} else {
		err_descr = libraw_strerror(err);
	}

	arg->logger_msg(arg->logger_arg, "%s. %s\n", err_where, err_descr);
}

void set_metadata_from_raw(libraw_data_t *rawdata, file_metadata_t *dst_meta)
{
	size_t tmplen;
	struct tm *utc_tm;
	char time_buf[25];

#if !(LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,17))
	#pragma message ("LibRaw version <= 0.17. Metadata extraction may not work properly")
#endif

	if ((strlen(dst_meta->instrument) == 0) || dst_meta->overwrite_instrument) {
		tmplen = strlen(rawdata->idata.make);
		strcpy(dst_meta->instrument, rawdata->idata.make);
		dst_meta->instrument[tmplen] = 0x20;
		strcpy(dst_meta->instrument + tmplen + 1, rawdata->idata.model);
		dst_meta->overwrite_instrument = 1;
	}

	if ((strlen(dst_meta->observer) == 0) || dst_meta->overwrite_observer) {
		strcpy(dst_meta->observer, rawdata->other.artist);
		dst_meta->overwrite_observer = 1;
	}

	if ((strlen(dst_meta->date) == 0) || dst_meta->overwrite_date) {
		utc_tm = gmtime(&rawdata->other.timestamp);
		strftime(time_buf, 25, "%Y-%m-%dT%H:%M:%S", utc_tm);
		strcpy(dst_meta->date, time_buf);
		dst_meta->overwrite_date = 1;
	}

	if (dst_meta->exptime == 0 || dst_meta->overwrite_exptime) {
		dst_meta->exptime = rawdata->other.shutter;
		dst_meta->overwrite_exptime = 1;
	}
}

static int create_new_fits(fitsfile **fptr, char *filename)
{
	int status = 0;
	fits_create_file(fptr, filename, &status);

	return status;
}

void close_fits(fitsfile *fptr)
{
	int status = 0;

	fits_flush_file(fptr, &status);
	fits_close_file(fptr, &status);
}

int create_fits_image(fitsfile *fptr, int width, int height, int bitpixel)
{
	unsigned int naxis = 2;
	long naxes[2] = { width, height };
	int status = 0;

	fits_create_img(fptr, bitpixel, naxis, naxes, &status);

	return status;
}

void get_current_datetime(char *dst)
{
	time_t lt = time(NULL);
	struct tm *utc_tm = gmtime(&lt);

	strftime(dst, 25, "%Y-%m-%dT%H:%M:%S", utc_tm);
}

int write_fits_header(fitsfile *fptr, file_metadata_t *meta, char *add_comment)
{
	int status = 0;
	char time_now[25];
	float cpix1 = (meta->width + 1) / 2;
	float cpix2 = (meta->height + 1) / 2;
	char coord_buf[15];
	char ver_str[21];

	float ra = coordinates_to_deg(meta->ra.hour, meta->ra.min, meta->ra.sec, meta->ra.msec);
	float dec = coordinates_to_deg(meta->dec.hour, meta->dec.min, meta->dec.sec, meta->dec.msec);

	snprintf(ver_str, 21, "raw2fits %i.%i.%i"
			, RAW2FITS_VERSION_MAJOR, RAW2FITS_VERSION_MINOR, RAW2FITS_VERSION_PATCH);

	get_current_datetime(time_now);

	fits_write_key(fptr, TSTRING, "CREATOR", ver_str, "", &status);
	fits_write_key(fptr, TSTRING, "DATE", time_now, "Fits creation date, UTC", &status);
	fits_write_key(fptr, TSTRING, "OBJECT", meta->object, "Name of the object observed", &status);
	fits_write_key(fptr, TSTRING, "CTYPE1", "RA---TAN", "RA in tangent plane projection", &status);
	fits_write_key(fptr, TSTRING, "CTYPE2", "DEC--TAN", "DEC in tangent plane projection", &status);
	fits_write_key(fptr, TFLOAT, "CRPIX1", &cpix1, "The reference pixel coordinate 1", &status);
	fits_write_key(fptr, TFLOAT, "CRPIX2", &cpix2, "The reference pixel coordinate 2", &status);
	fits_write_key(fptr, TFLOAT, "CRVAL1", &ra, "Reference pixel value in degrees", &status);
	fits_write_key(fptr, TFLOAT, "CRVAL2", &dec, "Reference pixel value in degrees", &status);

// TODO: calculate and store CDELTx
//	fits_write_key(fptr, TFLOAT, "CDELT1", &pixel, "Coordinate increment per pixel in DEGREES/PIXEL", &status);
//	fits_write_key(fptr, TFLOAT, "CDELT2", &pixel, "Coordinate increment per pixel in DEGREES/PIXEL", &status);

	fits_write_key(fptr, TSTRING, "TELESCOP", meta->telescope, "Telescope", &status);
	fits_write_key(fptr, TFLOAT, "TELAPER", &meta->teleaper, "Clear aperture of the telescope [m]", &status);
	fits_write_key(fptr, TFLOAT, "TELFOC", &meta->telefoc, "Focal length of the telescope [m]", &status);
	fits_write_key(fptr, TSTRING, "INSTRUME", meta->instrument, "Detector type", &status);
	fits_write_key(fptr, TSTRING, "DATE-OBS", meta->date, "Observation date and time, UTC", &status);
	fits_write_key(fptr, TFLOAT, "EXPTIME", &meta->exptime, "Exposure time in seconds", &status);
	fits_write_key(fptr, TSTRING, "FILTER", meta->filter, "Filter used when taking image", &status);
	fits_write_key(fptr, TSTRING, "OBSERVAT", meta->observatory, "Observatory name", &status);
	fits_write_key(fptr, TSTRING, "SITENAME", meta->sitename, "Observatory site name", &status);
	fits_write_key(fptr, TFLOAT, "SITELAT", &meta->sitelat, "Latitude of the observing site, decimal degrees", &status);
	fits_write_key(fptr, TFLOAT, "SITELONG", &meta->sitelon, "East longitude of the observing site, decimal degrees", &status);
	fits_write_key(fptr, TFLOAT, "SITEELEV", &meta->sitelev, "Elevation of the observatory site [m].", &status);
	fits_write_key(fptr, TSTRING, "OBSERVER", meta->observer, "", &status);

	coordinates_to_sexigesimal_str(meta->ra.hour, meta->ra.min, meta->ra.sec, meta->ra.msec, coord_buf);
	fits_write_key(fptr, TSTRING, "RA", coord_buf, "Object Right Ascension", &status);

	coordinates_to_sexigesimal_str(meta->dec.hour, meta->dec.min, meta->dec.sec, meta->dec.msec, coord_buf);
	fits_write_key(fptr, TSTRING, "DEC", coord_buf, "Object Declination", &status);

	fits_write_key(fptr, TFLOAT, "TEMPERAT", &meta->temperature, "Camera temperature in C", &status);
	fits_write_key(fptr, TSTRING, "OBSNOTES", meta->note, "", &status);

	fits_write_comment(fptr, add_comment, &status);

	return status;
}

void copy_image_buf(FRAME_MODE mode, libraw_processed_image_t *proc_img, long **dst)
{
	int i, k = 0;
	long rgb[3];

	for (i = 0; i < proc_img->width * proc_img->height; i++) {
		rgb[0] = proc_img->data[k];
		rgb[1] = proc_img->data[k + 1];
		rgb[2] = proc_img->data[k + 2];

		switch (mode) {
			case GRAYSCALE:
				(*dst)[i] = (rgb[0] + rgb[1] + rgb[2]) / 3;
				break;

			case RED_ONLY:
				(*dst)[i] = rgb[0];
				break;

			case GREEN_ONLY:
				(*dst)[i] = rgb[1];
				break;

			case BLUE_ONLY:
				(*dst)[i] = rgb[2];
				break;

			default:
				break;
		}

		k += 3;
	}
}

int write_fits_image(fitsfile *fptr, long *frame, int width, int height)
{
	int status = 0;
	long fpx[2] = { 1L, 1L };

	fits_write_pix(fptr, TLONG, fpx, width * height, frame, &status);

	return status;
}

void raw2fits(char *file, converter_params_t *arg)
{
	libraw_decoder_info_t decoder_info;
	libraw_data_t *rawdata;
	libraw_processed_image_t *proc_img;
	char target_filename[512] = { 0 };
	size_t target_filename_len;
	fitsfile *fits;
	long *framebuf;
	int i, err;
	int target_file_exists = 0;

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

	set_metadata_from_raw(rawdata, &arg->meta);

	make_target_fits_filename(arg, file, target_filename, FILENAME_CHANNEL_POSTFIX[arg->imsetup.mode]);

	target_file_exists = is_file_exist(target_filename);

	if (arg->imsetup.mode != ALL_CHANNELS_BY_FILES)
	{
		if (target_file_exists) {
			if (!arg->fsetup.overwrite) {
				arg->logger_msg(arg->logger_arg, "File %s is already exists, skipping...\n", target_filename);
				libraw_recycle(rawdata);
				libraw_close(rawdata);
				return;
			}

			if (remove_file(target_filename) < 0) {
				arg->logger_msg(arg->logger_arg, "Unable to remove old file %s, error: \n", strerror(errno));
				libraw_recycle(rawdata);
				libraw_close(rawdata);
				return;
			}
		}
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

#if (LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0,17))
	rawdata->params.no_auto_bright = !arg->imsetup.apply_auto_bright;
	rawdata->params.no_interpolation = !arg->imsetup.apply_interpolation;
	rawdata->params.no_auto_scale = !arg->imsetup.apply_autoscale;
#else
	#pragma message ("LibRaw version is to old, unable to use image corrections")
#endif

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
		libraw_recycle(rawdata);
		libraw_close(rawdata);
		return;
	}

	arg->logger_msg(arg->logger_arg, "\tImage decoded, size = %ix%i, bits = %i, colors = %i\n",
									proc_img->width, proc_img->height, proc_img->bits, proc_img->colors);

	libraw_recycle(rawdata);
	libraw_close(rawdata);

	arg->meta.bitpixel = proc_img->bits;
	arg->meta.width = proc_img->width;
	arg->meta.height = proc_img->height;

	framebuf = (long *) malloc(proc_img->width * proc_img->height * sizeof(long));

	if (!framebuf) {
		arg->logger_msg(arg->logger_arg, "Failed to allocate memory for the frame, err: \n", strerror(err));
		libraw_dcraw_clear_mem(proc_img);
	}

	if (arg->imsetup.mode == ALL_CHANNELS_BY_FILES) {
		target_filename_len = strlen(target_filename);

		for (i = 0; i < 3; i++) {
			strcpy(target_filename + target_filename_len - 5, FILENAME_CHANNEL_POSTFIX[i + 3]);
			/* at this point we need to check file exists again... */

			target_file_exists = is_file_exist(target_filename);

			if (target_file_exists) {
				if (!arg->fsetup.overwrite) {
					arg->logger_msg(arg->logger_arg, "File %s is already exists, skipping...\n", target_filename);
					continue;
				}

				if (remove_file(target_filename) < 0) {
					arg->logger_msg(arg->logger_arg, "Unable to remove old file %s, error: \n", strerror(errno));
					continue;
				}
			}

			arg->logger_msg(arg->logger_arg, "Creating FITS %s\n", target_filename);

			err = create_new_fits(&fits, target_filename);

			if (err != 0) {
				arg->logger_msg(arg->logger_arg, "Failed to create file, error %i\n", err);
				continue;
			}

			err = create_fits_image(fits, proc_img->width, proc_img->height, proc_img->bits);
			err = write_fits_header(fits, &arg->meta, FITS_HEADER_COMMENT[i + 3]);

			if (err != 0) {
				arg->logger_msg(arg->logger_arg, "Failed to write FITS header, error %i\n", err);
				continue;
			}

			copy_image_buf(FRAME_COPY_MODES[i], proc_img, &framebuf);

			write_fits_image(fits, framebuf, proc_img->width, proc_img->height);

			close_fits(fits);

		}

	} else if (arg->imsetup.mode == ALL_CHANNELS) {
		arg->logger_msg(arg->logger_arg, "Creating multi-image FITS %s\n", target_filename);

		err = create_new_fits(&fits, target_filename);

		if (err != 0) {
			arg->logger_msg(arg->logger_arg, "Failed to create file, error %i\n", err);
			free(framebuf);
			libraw_dcraw_clear_mem(proc_img);
			return;
		}

		for (i = 0; i < 3; i++) {
			copy_image_buf(FRAME_COPY_MODES[i], proc_img, &framebuf);
			err = create_fits_image(fits, proc_img->width, proc_img->height, proc_img->bits);
			err = write_fits_header(fits, &arg->meta, FITS_HEADER_COMMENT[i + 3]);
			write_fits_image(fits, framebuf, proc_img->width, proc_img->height);
		}

		close_fits(fits);

	} else {
		arg->logger_msg(arg->logger_arg, "Creating FITS %s\n", target_filename);

		err = create_new_fits(&fits, target_filename);

		if (err != 0) {
			arg->logger_msg(arg->logger_arg, "Failed to create file, error %i\n", err);
			free(framebuf);
			libraw_dcraw_clear_mem(proc_img);
			return;
		}

		err = create_fits_image(fits, proc_img->width, proc_img->height, proc_img->bits);
		err = write_fits_header(fits, &arg->meta, FITS_HEADER_COMMENT[arg->imsetup.mode]);

		if (err != 0) {
			arg->logger_msg(arg->logger_arg, "Failed to write FITS header, error %i\n", err);
			free(framebuf);
			libraw_dcraw_clear_mem(proc_img);
			return;
		}

		copy_image_buf(arg->imsetup.mode, proc_img, &framebuf);

		write_fits_image(fits, framebuf, proc_img->width, proc_img->height);

		close_fits(fits);
	}

	free(framebuf);
	libraw_dcraw_clear_mem(proc_img);
}

