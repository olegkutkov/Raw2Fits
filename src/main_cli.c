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
#include <libconfig.h>
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

int load_configuration_io_paths(config_setting_t *setting, converter_params_t *conv_params)
{
	const char *str;

	if (!config_setting_lookup_string(setting, "raw_dir", &str)) {
		fprintf(stderr, "Can't find raw2fits.io.raw_dir param in the config file\n");
		return -1;
	}

	strcpy(conv_params->inpath, str);

	if (!config_setting_lookup_string(setting, "fits_dir", &str)) {
		fprintf(stderr, "Can't find raw2fits.io.raw_dir param in the config file\n");
		return -1;
	}

	strcpy(conv_params->outpath, str);

	return 0;
}

void load_configuration_io_filters(config_setting_t *setting, converter_params_t *conv_params)
{
	int count, i;
	const char *str;

	count = config_setting_length(setting);

	for (i = 0; i < count; ++i) {
		str = config_setting_get_string_elem(setting, i);
		printf("raw_filter: %s\n", str);
	}
}

int load_configuration_io_filenaming(config_setting_t *setting, converter_params_t *conv_params)
{
	int val;

	if (!config_setting_lookup_int(setting, "mode", &val)) {
		fprintf(stderr, "Can't find raw2fits.io.mode param in the config file\n");
		return -1;
	}

	conv_params->fsetup.naming = val;

	if (!config_setting_lookup_bool(setting, "overwrite", &val)) {
		fprintf(stderr, "Can't find raw2fits.io.overwrite param in the config file\n");
		return -1;
	}

	conv_params->fsetup.overwrite = (char) val;

	return 0;
}

int load_configuration(char *confile, converter_params_t *conv_params)
{
	config_t cfg;
	config_setting_t *setting;

	config_init(&cfg);

	if (!config_read_file(&cfg, confile)) {
		fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg)
				, config_error_line(&cfg), config_error_text(&cfg));

		config_destroy(&cfg);

		return (EXIT_FAILURE);
	}

	setting = config_lookup(&cfg, "raw2fits");

	if (!setting) {
		fprintf(stderr, "Given file is not raw2fits configuration\n");

		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	setting = config_lookup(&cfg, "raw2fits.io");

	if (!setting) {
		fprintf(stderr, "Can't find input/output params in the config file\n");

		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	if (load_configuration_io_paths(setting, conv_params) < 0) {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	setting = config_lookup(&cfg, "raw2fits.io.raw_filter_name");

	if (setting) {
		load_configuration_io_filters(setting, conv_params);
	}

	setting = config_lookup(&cfg, "raw2fits.io.filenaming");

	if (!setting) {
		fprintf(stderr, "Can't find raw2fits.io.filenaming param in the config file\n");

		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	if (load_configuration_io_filenaming(setting, conv_params) < 0) {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	config_destroy(&cfg);

	return 0;
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

	return 0;
}

