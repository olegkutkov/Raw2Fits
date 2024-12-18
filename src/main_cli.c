/* 
   main_cli.c
    - entry point of the non-gui application

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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include "config_loader.h"
#include "converter.h"
#include "file_utils.h"
#include "version.h"

static int QUIET_FLAG = 0;
static volatile int RUN_FLAG = 0;

static int QUITE_COUNTER_ANI = 0;
static int QUITE_MAX_COUNTER_ANI = 0;

static char *PROGRESS_ANI_CHARS = NULL;

static struct option cmd_long_options[] =
{
	{"quiet", no_argument, 0, 'q'},
	{"help",   no_argument, 0, 'h'},
	{"input",   required_argument, 0, 'i'},
	{"output",  required_argument, 0, 'o'},
	{"config",  required_argument, 0, 'c'},
	{0, 0, 0, 0}
};

void show_help()
{
	printf("raw2fits, version: %i.%i.%i\n"
			, RAW2FITS_VERSION_MAJOR, RAW2FITS_VERSION_MINOR, RAW2FITS_VERSION_PATCH);
	printf("Oleg Kutkov <contact@olegkutkov.me>\n\n");

	printf("\t-h, --help\t\tShow this help and exit\n");
	printf("\t-q, --quiet\t\tReduce ouptut messages\n");
	printf("\t-i, --input\t\tSet directory with RAW files\n");
	printf("\t-o, --output\t\tSet directory for output FITS files\n");
	printf("\t-c, --config <file>\tConfiguration file for converter\n");
}

void progress_setup(void *arg, int max_val)
{
	QUITE_MAX_COUNTER_ANI = max_val;

	PROGRESS_ANI_CHARS = (char *) calloc(1, QUITE_MAX_COUNTER_ANI + 1);
	PROGRESS_ANI_CHARS[0] = '[';
}

void progress_update(void *arg)
{
	int i;
	int pval = ((float)QUITE_COUNTER_ANI / (float)QUITE_MAX_COUNTER_ANI) * 100;

	for (i = 1; i < QUITE_MAX_COUNTER_ANI; i++) {
		if (i <= QUITE_COUNTER_ANI) {
			PROGRESS_ANI_CHARS[i] = '=';
		} else {
			PROGRESS_ANI_CHARS[i] = ' ';
		}

		if (i == QUITE_MAX_COUNTER_ANI -1) {
			PROGRESS_ANI_CHARS[i] = ']';
		}
	}

	if (QUITE_COUNTER_ANI == QUITE_MAX_COUNTER_ANI - 1) {
		pval = 100;
	}

	printf("\rOverall progress: %s - %i %%", PROGRESS_ANI_CHARS, pval);

	if (QUIET_FLAG) {
		fflush(stdout);
	} else {
		printf("\n");
	}

	QUITE_COUNTER_ANI++;
}

void logger_msg(void *arg, char *fmt, ...)
{
	if (QUIET_FLAG) {
		return;
	}

	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);

	va_end(args);
}

void converting_done(void *arg)
{
	RUN_FLAG = 0;
}

void interrupt_handler(int val)
{
	RUN_FLAG = 0;
}

int main(int argc, char **argv)
{
	int c, ret;
	char *indir = NULL, *outdir = NULL, *confile = NULL;
	converter_params_t conv_params;

	while (1) {
		int option_index = 0;

		c = getopt_long(argc, argv, "qhi:o:c:", cmd_long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch(c) {
			case 'q':
				QUIET_FLAG = 1;
				break;

			case 'h':
				show_help();
				return 0;

			case 'i':
				indir = optarg;
				break;

			case 'o':
				outdir = optarg;
				break;

			case 'c':
				confile = optarg;
				break;

			case '?':
				show_help();
				return -1;

			default:
				abort();
		}
	}

	if (!confile) {
		fprintf(stderr, "No configuration!\n");
		show_help();
		return -1;
	}

	conv_params.converter_run = 1;
	memset(&conv_params.meta, 0, sizeof(file_metadata_t));

	printf("raw2fits, version: %i.%i.%i\n"
		, RAW2FITS_VERSION_MAJOR, RAW2FITS_VERSION_MINOR, RAW2FITS_VERSION_PATCH);

	printf("Loading configuration from %s\n", confile);


	ret = load_configuration(confile, &conv_params);

	if (ret != 0) {
		printf("\nExiting...\n");
		return ret;
	}

	if (indir != NULL) {
		strcpy(conv_params.inpath, indir);
	}

	if (outdir != NULL) {
		strcpy(conv_params.outpath, outdir);
	}

	if (!is_file_exist(conv_params.inpath)) {
		fprintf(stderr, "Path %s doesn't exists\n", conv_params.inpath);
		return -1;
	}

	if (!is_file_exist(conv_params.outpath)) {
		fprintf(stderr, "Path %s doesn't exists\n", conv_params.outpath);
		return -1;
	}

	conv_params.progress.progr_arg = NULL;

	conv_params.progress.progr_setup = &progress_setup;
	conv_params.progress.progr_update = &progress_update;

	conv_params.logger_arg = NULL;
	conv_params.logger_msg = &logger_msg;

	conv_params.done_arg = NULL;
	conv_params.complete = &converting_done;

	printf("\nInput directory: %s\n", conv_params.inpath);
	printf("Output directory: %s\n\n", conv_params.outpath);

	dump_configuration(&conv_params);

	printf("Staring converter (press Ctrl-C to terminate procedure) ...\n\n");

	RUN_FLAG = 1;
	signal(SIGINT, interrupt_handler);

	convert_files(&conv_params);

	while (RUN_FLAG) {
		sleep(1);
	}

	converter_stop(&conv_params);

	if (QUIET_FLAG) {
		printf("\n");
	}

	printf("\nDone!\n");

	if (PROGRESS_ANI_CHARS) {
		free(PROGRESS_ANI_CHARS);
		PROGRESS_ANI_CHARS = NULL;
	}

	return 0;
}

