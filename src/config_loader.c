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

static const char *color_mode_dump_desc[] =
{
	"Convert RGB to average grayscale",
	"R, G and B channels to the separate FITS's",
	"R, G and B channels to the one FITS with separate headers",
	"Only R channel",
	"Only G channel",
	"Only B channel"
};

static const char *out_filenaming_dump_des[] =
{
	"<RAW file name>.fits",
	"<object>_<datetime>.fits",
	"<object>_<filter>_<datetime>.fits",
	"<RAW file name>_<datetime>.fits"
};

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

	if (val < 0 || val > 3) {
		printf("Invalid raw2fits.io.mode value = %i, possible range is 0-3\n", val);
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

	if (config_setting_lookup_string(setting, "notes", &str)) {
		strcpy(conv_params->meta.note, str);
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

int load_image_colors_options(config_setting_t *setting, converter_params_t *conv_params)
{
	int val;

	if (!config_setting_lookup_int(setting, "mode", &val)) {
		fprintf(stderr, "Can't find raw2fits.colors.mode param in the config file\n");
		return -1;
	}

	if (val < 0 || val > 5) {
		printf("Invalid raw2fits.colors.mode value = %i, possible range is 0-5\n", val);
		return -1;
	}

	conv_params->imsetup.mode = val;

	if (!config_setting_lookup_bool(setting, "autobright", &val)) {
		fprintf(stderr, "Can't find raw2fits.colors.autobright param in the config file\n");
		return -1;
	}

	conv_params->imsetup.apply_auto_bright = (char) val;

	if (!config_setting_lookup_bool(setting, "interpolation", &val)) {
		fprintf(stderr, "Can't find raw2fits.colors.interpolation param in the config file\n");
		return -1;
	}

	conv_params->imsetup.apply_interpolation = (char) val;

	if (!config_setting_lookup_bool(setting, "autoscale", &val)) {
		fprintf(stderr, "Can't find raw2fits.colors.autoscale param in the config file\n");
		return -1;
	}

	conv_params->imsetup.apply_autoscale = (char) val;

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

	setting = config_lookup(&cfg, "raw2fits.colors");

	if (!setting) {
		fprintf(stderr, "Can't find image & colors processing options in the config file\n");

		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	if (load_image_colors_options(setting, conv_params) < 0) {
		config_destroy(&cfg);
		return (EXIT_FAILURE);
	}

	config_destroy(&cfg);

	return 0;
}

void dump_configuration(converter_params_t *conv_params)
{
	char ra[17], dec[17];

	printf("FITS header data loaded from the configuration file: \n");

	printf("OBJECT:\t\t\t%s\n", conv_params->meta.object);

	coordinates_to_sexigesimal_str(conv_params->meta.ra.hour, conv_params->meta.ra.min,
									conv_params->meta.ra.sec, conv_params->meta.ra.msec, ra);

	coordinates_to_sexigesimal_str(conv_params->meta.dec.hour, conv_params->meta.dec.min,
									conv_params->meta.dec.sec, conv_params->meta.dec.msec, dec);

	printf("OBJECT RA:\t\t%s\n", ra);
	printf("OBJECT DEC:\t\t%s\n\n", dec);

	printf("TELESCOPE:\t\t%s\n", conv_params->meta.telescope);
	printf("TELESCOPE APERTURE:\t%f meters\n", conv_params->meta.teleaper);
	printf("TELESCOPE FOCAL LENGTH: %f meters\n", conv_params->meta.telefoc);
	printf("INSTRUMENT:\t\t%s\n\n", conv_params->meta.instrument);

	printf("OBSERVATORY:\t\t%s\n", conv_params->meta.observatory);
	printf("SITENAME:\t\t%s\n", conv_params->meta.sitename);
	printf("SITE LAT:\t\t%f\n", conv_params->meta.sitelat);
	printf("SITE LON:\t\t%f\n", conv_params->meta.sitelon);
	printf("SITE ELEVATION:\t\t%f meters\n\n", conv_params->meta.sitelev);

	printf("FILTER:\t\t\t%s\n", conv_params->meta.filter);
	printf("EXPOSURE:\t\t%.4f seconds\n\n", conv_params->meta.exptime);

	printf("TEMPERATURE:\t\t%.2f C\n\n", conv_params->meta.temperature);

	printf("OBSERVER:\t\t%s\n", conv_params->meta.observer);
	printf("DATE:\t\t%s\n", conv_params->meta.date);
	printf("NOTES:\t\t\t%s\n", conv_params->meta.note);

	printf("\nEnd of FITS header data\n");


	printf("\nImage & colors options:\n");

	printf("Mode: %i (%s)\n", conv_params->imsetup.mode, color_mode_dump_desc[conv_params->imsetup.mode]);
	printf("Apply autobright by histogram: %s\n", (conv_params->imsetup.apply_auto_bright ? "Yes" : "No"));
	printf("Apply pixels interpolation: %s\n", (conv_params->imsetup.apply_interpolation ? "Yes" : "No"));
	printf("Apply pixels autoscale: %s\n", (conv_params->imsetup.apply_autoscale ? "Yes" : "No"));


	printf("\nOutput options:\n");
	printf("Mode: %i (%s)\n", conv_params->fsetup.naming, out_filenaming_dump_des[conv_params->fsetup.naming]);
	printf("Overwrite existing files: %s\n", (conv_params->fsetup.overwrite ? "Yes" : "No"));

	printf("\nEnd of configuration\n\n");
}

