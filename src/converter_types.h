
#ifndef __CONVERTER_TYPES_H__
#define __CONVERTER_TYPES_H__

typedef enum frame_mode {
	GRAYSCALE = 0,
	ALL_CHANNELS_BY_FILES,
	ALL_CHANNELS,
	RED_ONLY,
	GREEN_ONLY,
	BLUE_ONLY
} FRAME_MODE;

typedef enum file_naming {
	RAW_NAME = 0,
	OBJECT_DATETIME,
	OBJECT_FILTER_DATETIME
} file_naming_t;

typedef struct file_metadata {
	int bitpixel;
	unsigned int width;
	unsigned int height;
	double exptime;
	double temperature;
	char object[72];
	char telescope[72];
	char instrument[72];
	char observer[72];
	char filter[72];
	char note[72];
	char date[72];
} file_metadata_t;

typedef struct image_setup {
	FRAME_MODE mode;
	char apply_auto_bright;
	char apply_interpolation;
	char apply_autoscale;
} image_setup_t;

typedef struct file_setup {
	file_naming_t naming;
	char overwrite;
} file_setup_t;

typedef void (*progress_setup_cb) (void*, int);
typedef void (*progress_update_cb) (void*);

typedef struct progress_params {
	void *progr_arg;
	progress_setup_cb progr_setup;
	progress_update_cb progr_update;
	float fraction;
	int current_value;
} progress_params_t;

typedef void (*logger_msg_cb) (void*, char*, ...);
typedef void (*done_cb) (void);

typedef struct converter_params {
	char converter_run;
	char inpath[256];
	char outpath[256];
	file_metadata_t meta;
	image_setup_t imsetup;
	file_setup_t fsetup;
	progress_params_t progress;
	void *logger_arg;
	logger_msg_cb logger_msg;
	done_cb complete;
} converter_params_t;

#endif

