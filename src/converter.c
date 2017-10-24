////

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <libraw/libraw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fitsio.h>
#include <libgen.h>
#include "list.h"
#include <unistd.h>
#include "converter.h"
#include "file_utils.h"
#include "thread_pool.h"

typedef struct thread_arg {
	list_node_t *filelist;
	converter_params_t *conv_param;
} thread_arg_t;

typedef struct frame {
	long *grayscale_combined;
	long *red_channel;
	long *blue_channel;
	long *green_channel;
	FRAME_MODE mode;
} frame_t;

static int my_progress_callback(void *data,enum LibRaw_progress p,int iteration, int expected)
{
	printf("Decoding progress: %s  iter = %i  exp = %i\n", libraw_strprogress(p), iteration, expected);

	return 0;
}

int is_file_exist(char *filename)
{
	struct stat buffer;
	return (stat (filename, &buffer) == 0);
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

void write_fits(libraw_data_t *raw, char *outdir, char *filename, frame_t *frame, file_metadata_t *meta)
{
	int status = 0;
	fitsfile *fptr;
	long fpx[2] = { 1L, 1L };

	unsigned int naxis = 2;
	long naxes[2] = { meta->width, meta->height };

	long *frame_buf = NULL;
	int write_all = 0;

	size_t outdir_len = strlen(outdir);
	size_t filename_len = strlen(filename);
	char *outname = (char *) malloc(outdir_len + filename_len + 5);

	strncpy(outname, outdir, outdir_len);
	outname[outdir_len] = '/';
	strncpy(outname + outdir_len + 1, filename, filename_len);
	strncpy(outname + outdir_len + 1 + filename_len - 3, "fits", 4);

	outname[outdir_len + filename_len + 2] = '\0';

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
/*	long int i = 0;

	float *frameGray;//, *frameGreen, *frameBlue, *frameGrayscale;

	frame_t frame_to_fits;

	conv_cb_arg_t *cb_arg = (conv_cb_arg_t *) arg;

	printf("convert_one_file called with arg %s %s\n", file, cb_arg->outdir);

	libraw_data_t *rawdata = libraw_init(0);

	if (!rawdata) {
		return;
	}

	libraw_set_progress_handler(rawdata, &my_progress_callback, NULL);

	libraw_open_file(rawdata, file);


	printf("iso: %f  shutter speed: %f \n", rawdata->other.iso_speed, rawdata->other.shutter);


	libraw_unpack(rawdata);
	
	libraw_decoder_info_t decoder_info;

	libraw_get_decoder_info(rawdata, &decoder_info);

	printf("raw decoder: %s\n", decoder_info.decoder_name);

	libraw_raw2image(rawdata);

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
	

	int k = 0;
	long red, green, blue;

	for (i = 0; i < proc_img->width * proc_img->height; i++) {
		red = proc_img->data[k];
		green = proc_img->data[k + 1];
		blue = proc_img->data[k + 2];

		frame_to_fits.red_channel[i] = red;

		//frame_to_fits.green1_channel[i] = proc_img->data[k] + 1;
		k += 3;
	}

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

	libraw_free_image(rawdata);
	libraw_recycle(rawdata);
	libraw_close(rawdata);
*/
}

void *thread_func(void *arg)
{
	converter_params_t *params = (converter_params_t *)arg;

	while (params->converter_run) {
		printf("i am thread\n");
		sleep(1);
	}
}

void convert_files(converter_params_t *params)
{
	int file_count = 0;
	DIR *dp;
	struct dirent *ep;
	list_node_t *file_list = NULL;
	long int cpucnt;
	int files_per_cpu_int, left_files, i;

	params->logger_msg(params->logger_arg, "Reading directory %s\n", params->inpath);

	dp = opendir(params->inpath);

	if (dp == NULL) {
		return;
	}

	while ((ep = readdir(dp))) {
		size_t inpath_len = strlen(params->inpath);
		size_t fname_len = strlen(ep->d_name);
		char *full_path = (char *) malloc(inpath_len + fname_len + 2);

		strncpy(full_path, params->inpath, inpath_len);
		full_path[inpath_len] = '/';
		strncpy(full_path + inpath_len + 1, ep->d_name, fname_len);
		full_path[inpath_len + fname_len + 1] = '\0';

		file_info_t finfo;
		get_file_info(full_path, &finfo);

		if (!finfo.file_supported) {
			free(full_path);
			continue;
		}

		params->logger_msg(params->logger_arg, " Found %s raw file %s  size: %liK\n",
							finfo.file_vendor, ep->d_name, finfo.file_size / 1024);
	
		file_list = add_object_to_list(file_list, full_path);

		free(full_path);

		file_count++;
	}

	closedir (dp);

	if (file_count == 0) {
		params->logger_msg(params->logger_arg, "Can't find RAW files, sorry\n");
		free_list(file_list);
		return;
	}

	params->progress.progr_setup(&params->progress, file_count);

	cpucnt = sysconf(_SC_NPROCESSORS_ONLN);

	params->logger_msg(params->logger_arg, "\nStarting conveter on %li processor cores...\n", cpucnt);

	for (i = 0; i < file_count; i++) {
		params->progress.progr_update(&params->progress);
		sleep(2);
	}

	if (cpucnt > file_count) {
		files_per_cpu_int = 1;
		left_files = 0;
	} else {
		files_per_cpu_int = file_count / cpucnt;
		left_files = file_count % cpucnt;
	}

	printf("Files per cpu %i, left files: %i\n", files_per_cpu_int, left_files);

	init_thread_pool(cpucnt);

	for (i = 0; i < cpucnt; i++) {
		thread_pool_add_task(thread_func, params);
	}

//	int files_on_cpus_minus_one = files_per_cpu_int * (cpucnt - 1);
//	int files_lef_for_last_core = file_count - files_on_cpus_minus_one;

//	printf("files per cpu: %i  files_on_cpus_minus_one: %i\n", files_per_cpu_int, files_on_cpus_minus_one);
//	printf("files_lef_for_last_core: %i\n", files_lef_for_last_core);
	
//	printf("simple: %li\n", (file_count % cpucnt));

//	thread_arg_t th_arg;

//	int thret = pthread_create( &thread1, NULL, print_message_function, (void*) message1);

//	iterate_list_cb(file_list, &convert_one_file, params);

//	free_list(file_list);
//	file_list = NULL;
}

void converter_stop(converter_params_t *params)
{
	params->converter_run = 0;

	thread_pool_stop_tasks();
	cleanup_thread_pool();
}

