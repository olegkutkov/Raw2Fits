/* 
  config_loader.c 
    - load, validate and dump configuration file

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

#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include "config_loader.h"
#include "coords_calc.h"

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

int load_configuration_fits_fields(config_setting_t *setting, converter_params_t *conv_params)
{
	const char *str;
	double fval;

	if (!config_setting_lookup_string(setting, "object", &str)) {
		fprintf(stderr, "Can't find mandatory param raw2fits.fits.object in the config file\n");
		return -1;
	}

	strcpy(conv_params->meta.object, str);

	if (config_setting_lookup_string(setting, "telescope", &str)) {
		strcpy(conv_params->meta.telescope, str);
	}

	if (config_setting_lookup_float(setting, "teleaper", &fval)) {
		conv_params->meta.teleaper = fval;
	}

	if (config_setting_lookup_float(setting, "telefoc", &fval)) {
		conv_params->meta.telefoc = fval;
	}

	if (config_setting_lookup_string(setting, "observatory", &str)) {
		strcpy(conv_params->meta.observatory, str);
	}

	if (config_setting_lookup_string(setting, "instrument", &str)) {
		strcpy(conv_params->meta.instrument, str);
	}

	if (config_setting_lookup_string(setting, "sitename", &str)) {
		strcpy(conv_params->meta.sitename, str);
	}

	if (config_setting_lookup_float(setting, "sitelat", &fval)) {
		conv_params->meta.sitelat = fval;
	}

	if (config_setting_lookup_float(setting, "sitelon", &fval)) {
		conv_params->meta.sitelon = fval;
	}

	if (config_setting_lookup_float(setting, "sitelev", &fval)) {
		conv_params->meta.sitelev = fval;
	}

	if (config_setting_lookup_string(setting, "observer", &str)) {
		strcpy(conv_params->meta.observer, str);
	}

	if (config_setting_lookup_string(setting, "filter", &str)) {
		strcpy(conv_params->meta.filter, str);
	}

	if (config_setting_lookup_string(setting, "date", &str)) {
		strcpy(conv_params->meta.date, str);
	}

	if (config_setting_lookup_float(setting, "exposure", &fval)) {
		conv_params->meta.exptime = fval;
	}

	if (config_setting_lookup_float(setting, "temperature", &fval)) {
		conv_params->meta.temperature = fval;
	}

	return 0;
}

int load_configuration_fits_object_coords(config_setting_t *setting, converter_params_t *conv_params)
{
	const char *str;

	if (!config_setting_lookup_string(setting, "ra", &str)) {
		fprintf(stderr, "Can't find RA coordinate in the raw2fits.fits.object_coordinates\n");
		return -1;
	}

	sexigesimal_str_to_coords(str, &conv_params->meta.ra.hour, &conv_params->meta.ra.min
								, &conv_params->meta.ra.sec, &conv_params->meta.ra.msec);

	if (!config_setting_lookup_string(setting, "dec", &str)) {
		fprintf(stderr, "Can't find RA coordinate in the raw2fits.fits.object_coordinates\n");
		return -1;
	}

	sexigesimal_str_to_coords(str, &conv_params->meta.dec.hour, &conv_params->meta.dec.min
								, &conv_params->meta.dec.sec, &conv_params->meta.dec.msec);

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

	setting = config_lookup(&cfg, "raw2fits.fits");

	if (!setting) {
		fprintf(stderr, "Can't find fits header params in the config file\n");

		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	if (load_configuration_fits_fields(setting, conv_params) < 0) {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	setting = config_lookup(&cfg, "raw2fits.fits.object_coordinates");

	if (!setting) {
		fprintf(stderr, "Waring: can't find raw2fits.fits.object_coordinates param in the config file, astrometry will not possible!\n");
	} else {
		load_configuration_fits_object_coords(setting, conv_params);
	}


	config_destroy(&cfg);

	return 0;
}

int validate_configuration(converter_params_t *conv_params)
{
	return 0;
}

void dump_configuration(converter_params_t *conv_params)
{
	char ra[17], dec[17];

	printf("OBJECT: %s\n", conv_params->meta.object);

	coordinates_to_sexigesimal_str(conv_params->meta.ra.hour, conv_params->meta.ra.min,
									conv_params->meta.ra.sec, conv_params->meta.ra.msec, ra);

	coordinates_to_sexigesimal_str(conv_params->meta.dec.hour, conv_params->meta.dec.min,
									conv_params->meta.dec.sec, conv_params->meta.dec.msec, dec);

	printf("OBJECT RA: %s\tDEC: %s\n", ra, dec);

}

