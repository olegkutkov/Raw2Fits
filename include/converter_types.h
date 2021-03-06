/* 
   converter_types.h
    - all structures and enums used by the raw2fits converter

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
	OBJECT_FILTER_DATETIME,
	RAW_DATETIME
} file_naming_t;

typedef struct coordinates {
	short hour;
	short min;
	short sec;
	short msec;
} coordinates_t;

typedef struct file_metadata {
	int bitpixel;
	unsigned int width;
	unsigned int height;
	coordinates_t ra;
	coordinates_t dec;
	float exptime;
	float temperature;
	float teleaper;
	float telefoc;
	float sitelat;
	float sitelon;
	float sitelev;
	char overwrite_instrument;
	char overwrite_observer;
	char overwrite_exptime;
	char overwrite_date;
	char object[72];
	char telescope[72];
	char instrument[72];
	char observer[72];
	char filter[72];
	char note[72];
	char date[72];
	char observatory[72];
	char sitename[72];
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
} progress_params_t;

typedef void (*logger_msg_cb) (void*, char*, ...);
typedef void (*done_cb) (void*);

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
	void *done_arg;
	done_cb complete;
} converter_params_t;

#endif

