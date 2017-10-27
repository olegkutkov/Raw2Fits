/* 
   file_utils.h

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
int remove_file(const char *filename);

void make_target_fits_filename(converter_params_t *arg, char *raw_filename, char *out_filename);

#endif

