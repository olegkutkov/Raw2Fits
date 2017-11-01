/* 
   file_utils.c
    - auxilarity file's functions specific for raw2fits

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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "file_utils.h"

struct file_vendors {
	char extensions[5];
	char vendor[25];
};

static struct file_vendors all_vendors[9] =
{
	{
		"cr2", "Canon"
	},
	{
		"nef", "Nikon"
	},
	{
		"arw", "Sony"
	},
	{
		"raf", "Fuji"
	},
	{
		"3fr", "Hasselblad"	
	},
	{
		"orf", "Olympus"
	},
	{
		"pef", "Pentax"
	},
	{
		"dng", "Adobe DNG"
	},
	{
		"mrw", "Konica Minolta"
	}
};

void get_file_info(char *fname, file_info_t *finf)
{
	size_t fname_len = strlen(fname);
	size_t il, vc;
	char *lowcase_buf = (char *) malloc (fname_len + 1);

	finf->file_supported = 0;

	for (il = 0; il < fname_len; il++) {
		lowcase_buf[il] = tolower(fname[il]);
	}

	lowcase_buf[fname_len] = '\0';

	char *extdot = strrchr(lowcase_buf, '.');

	if (extdot == NULL) {
		free(lowcase_buf);
		return;
	}

	char *file_extension = extdot + 1;

	for (vc = 0; vc < 9; vc++) {
		if (strstr(file_extension, all_vendors[vc].extensions) != NULL) {
			finf->file_supported = 1;
			strcpy(finf->file_vendor, all_vendors[vc].vendor);
			finf->file_size = get_file_size(fname);
		}
	}

	free(lowcase_buf);
}

long get_file_size(char *fname)
{
	struct stat statbuf;

	if (stat(fname, &statbuf) < 0) {
		return 0;
	}

	return statbuf.st_size;
}

int is_file_exist(char *filename)
{
	struct stat buffer;
	return (stat (filename, &buffer) == 0);
}

void make_target_fits_filename(converter_params_t *arg, char *raw_filename, char *out_filename, char *postfix)
{
	int iter = 0;
	char *out_file_name_base = NULL, *obj, *datetime, *filter;
	size_t outdir_len = strlen(arg->outpath);
	char *base_raw_filename = basename(raw_filename);
	size_t raw_filename_len = strlen(base_raw_filename);
	size_t obj_len = 0, datetime_len = 0, filter_len = 0;

	strncpy(out_filename, arg->outpath, outdir_len);
	out_filename[outdir_len] = '/';

	switch (arg->fsetup.naming) {
		case RAW_NAME:
			out_file_name_base = (char *) malloc(raw_filename_len);
			strncpy(out_file_name_base, base_raw_filename, raw_filename_len - 3);
			out_file_name_base[raw_filename_len - 3] = '\0';

			break;

		case RAW_DATETIME:
			if (strlen(arg->meta.date) > 0) {
				datetime = arg->meta.date;
			} else {
				datetime = "NO_DATE";
			}

			datetime_len = strlen(datetime);

			out_file_name_base = (char *) malloc(raw_filename_len + datetime_len + 4);
			strncpy(out_file_name_base, base_raw_filename, raw_filename_len - 3);
			strncpy(out_file_name_base + raw_filename_len - 4, "_", 1);
			strcpy(out_file_name_base + raw_filename_len - 3, datetime);

			raw_filename_len = raw_filename_len + datetime_len + 1;

			break;

		case OBJECT_DATETIME:
			if (strlen(arg->meta.object) > 0) {
				obj = arg->meta.object;
			} else {
				obj = base_raw_filename;
			}

			if (strlen(arg->meta.date) > 0) {
				datetime = arg->meta.date;
			} else {
				datetime = "NO_DATE";
			}

			obj_len = strlen(obj);
			datetime_len = strlen(datetime);

			out_file_name_base = (char *) malloc(obj_len + datetime_len + 4);
			strcpy(out_file_name_base, obj);
			strncpy(out_file_name_base + obj_len, "_", 1);
			strcpy(out_file_name_base + obj_len + 1, datetime);

			raw_filename_len = obj_len + datetime_len + 5;

			break;

		case OBJECT_FILTER_DATETIME:
			if (strlen(arg->meta.object) > 0) {
				obj = arg->meta.object;
			} else {
				obj = base_raw_filename;
			}

			if (strlen(arg->meta.filter) > 0) {
				filter = arg->meta.filter;
			} else {
				filter = "NO_FILTER";
			}

			if (strlen(arg->meta.date) > 0) {
				datetime = arg->meta.date;
			} else {
				datetime = "NO_DATE";
			}

			obj_len = strlen(obj);
			filter_len = strlen(filter);
			datetime_len = strlen(datetime);

			out_file_name_base = (char *) malloc(obj_len + filter_len + datetime_len + 5);
			strcpy(out_file_name_base, obj);
			strncpy(out_file_name_base + obj_len, "_", 1);
			strncpy(out_file_name_base + obj_len + 1, filter, filter_len);
			strncpy(out_file_name_base + obj_len + filter_len + 1, "_", 1);
			strcpy(out_file_name_base + obj_len + filter_len + 2, datetime);

			raw_filename_len = obj_len + filter_len + datetime_len + 6;

			break;
	}

	strncpy(out_filename + outdir_len + 1, out_file_name_base, raw_filename_len);
	strncpy(out_filename + outdir_len + 1 + raw_filename_len - 4, postfix, strlen(postfix));
	out_filename[outdir_len + raw_filename_len + strlen(postfix)] = '\0';

	for (; iter < strlen(out_filename); ++iter) {
		if (out_filename[iter] == 0x20) {
			out_filename[iter] = '_';
		}
	}

	if (out_file_name_base) {
		free(out_file_name_base);
	}
}

int remove_file(const char *filename)
{
	return unlink(filename);
}

