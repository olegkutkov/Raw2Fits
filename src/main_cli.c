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
#include "config_loader.h"
#include "converter.h"
#include "version.h"

static int verbose_flag;

static struct option cmd_long_options[] =
{
	/* These options set a flag. */
	{"verbose", no_argument, &verbose_flag, 1},
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
	printf("Oleg Kutkov <elenbert@gmail.com>\nCrimean astrophysical observatory, 2017\n\n");

	printf("\t-h, --help\t\tShow this help and exit\n");
	printf("\t-v, --verbose\t\tEnable verbose logging\n");
	printf("\t-i, --input\t\tSet directory with RAW files\n");
	printf("\t-o, --output\t\tSet directory for output FITS files\n");
	printf("\t-c, --config <file>\tConfiguration file for converter\n");
}

int main(int argc, char **argv)
{
	int c, ret;
	char *indir = NULL, *outdir = NULL, *confile = NULL;
	converter_params_t conv_params;

	while (1) {
		int option_index = 0;

		c = getopt_long (argc, argv, "vhi:o:c:", cmd_long_options, &option_index);

		if (c == -1) {
			break;
		}

		switch(c) {
			case 'v':
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

	ret = load_configuration(confile, &conv_params);

	if (ret != 0) {
		return ret;
	}

	if (indir != NULL) {
		strcpy(conv_params.inpath, indir);
	}

	if (outdir != NULL) {
		strcpy(conv_params.outpath, outdir);
	}

	return 0;
}

