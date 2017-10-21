////

typedef enum frame_mode {
	GRAYSCALE = 0,
	RED_ONLY,
	BLUE_ONLY,
	GREEN_ONLY,
	ALL_CHANNELS,
	ALL_CHANNELS_BY_FILES
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

typedef void (*progress_setup_cb) (int, void*);
typedef void (*progress_update_cb) (void*);
typedef void (*logger_msg_cb) (void*, char*);
typedef void (*done_cb) (void);

typedef struct converter_params {
	char inpath[256];
	char outpath[256];
	file_metadata_t meta;
	image_setup_t imsetup;
	file_setup_t fsetup;
	void *progr_arg;
	progress_setup_cb progr_setup;
	progress_update_cb progr_update;
	void *logger_arg;
	logger_msg_cb logger_msg;
	done_cb complete;
} converter_params_t;

void convert_files(converter_params_t *params);
void converter_stop();

//void convert_files(char *inpath, char *outpath);

