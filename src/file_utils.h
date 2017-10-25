
#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include <stdint.h>
#include "converter_types.h"

typedef struct file_info {
	char file_vendor[25];
	long file_size;
	uint8_t file_supported;
} file_info_t;

void get_file_info(char *fname, file_info_t *finf);
long get_file_size(char *fname);
int is_file_exist(char *filename);

void make_target_fits_filename(converter_params_t *arg, char *raw_filename, char *out_filename);

#endif

