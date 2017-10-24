////

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "list.h"
#include <unistd.h>
#include "converter.h"
#include "file_utils.h"
#include "thread_pool.h"

static list_node_t *file_list = NULL;

typedef struct thread_arg {
	converter_params_t *conv_param;
	list_node_t *filelist;
	int file_list_offset;
	int file_list_len;
} thread_arg_t;

void convert_one_file(char *file, void *arg)
{
	converter_params_t *params = (converter_params_t *) arg;

	printf("convert_one_file from thread, file: %s\n", file);

	params->progress.progr_update(&params->progress);
}

void *thread_func(void *arg)
{
	thread_arg_t th_arg_local;
	thread_arg_t *th_arg = (thread_arg_t *) arg;
	converter_params_t *params;

	memcpy(&th_arg_local, th_arg, sizeof(thread_arg_t));

	free(th_arg);

	params = th_arg_local.conv_param;

	iterate_list_cb(th_arg_local.filelist, &convert_one_file, params, th_arg_local.file_list_offset, th_arg_local.file_list_len, &params->converter_run);

	return NULL;
}

void convert_files(converter_params_t *params)
{
	int file_count = 0;
	DIR *dp;
	struct dirent *ep;
	long int cpucnt;
	int files_per_cpu_int, left_files, i;
	int file_list_offset_next = 0;
	thread_arg_t *thread_params;

	params->logger_msg(params->logger_arg, "Reading directory %s\n", params->inpath);

	dp = opendir(params->inpath);

	if (dp == NULL) {
		return;
	}

	while ((ep = readdir(dp))) {
		size_t inpath_len = strlen(params->inpath);
		size_t fname_len = strlen(ep->d_name);
		char *full_path = (char *) malloc(inpath_len + fname_len + 2);

		strncpy(full_path, params->inpath, inpath_len);
		full_path[inpath_len] = '/';
		strncpy(full_path + inpath_len + 1, ep->d_name, fname_len);
		full_path[inpath_len + fname_len + 1] = '\0';

		file_info_t finfo;
		get_file_info(full_path, &finfo);

		if (!finfo.file_supported) {
			free(full_path);
			continue;
		}

		params->logger_msg(params->logger_arg, " Found %s raw file %s  size: %liK\n",
							finfo.file_vendor, ep->d_name, finfo.file_size / 1024);
	
		file_list = add_object_to_list(file_list, full_path);

		free(full_path);

		file_count++;
	}

	closedir (dp);

	if (file_count == 0) {
		params->logger_msg(params->logger_arg, "Can't find RAW files, sorry\n");
		free_list(file_list);
		return;
	}

	params->progress.progr_setup(&params->progress, file_count);

	cpucnt = sysconf(_SC_NPROCESSORS_ONLN);

	params->logger_msg(params->logger_arg, "\nStarting conveter on %li processor cores...\n", cpucnt);

	if (cpucnt > file_count) {
		files_per_cpu_int = 1;
		left_files = 0;
	} else {
		files_per_cpu_int = file_count / cpucnt;
		left_files = file_count % cpucnt;
	}

	params->logger_msg(params->logger_arg, "Total files to convert: %i\n", file_count);
	params->logger_msg(params->logger_arg, "Files per CPU core: %i, left: %i\n", files_per_cpu_int, left_files);

	init_thread_pool(cpucnt);
	
	for (i = 0; i < cpucnt; i++) {
		thread_params = (thread_arg_t*) malloc(sizeof(thread_arg_t));

		thread_params->conv_param = params;
		thread_params->filelist = file_list;

		thread_params->file_list_offset = file_list_offset_next;
		thread_params->file_list_len = files_per_cpu_int;

		if (i == cpucnt - 1) {
			thread_params->file_list_len += left_files;
		}
 
		thread_pool_add_task(thread_func, thread_params);

		file_list_offset_next += files_per_cpu_int;
	}
}

void converter_stop(converter_params_t *params)
{
	params->converter_run = 0;

	thread_pool_stop_tasks();
	cleanup_thread_pool();

	if (file_list) {
		free_list(file_list);
		file_list = NULL;
	}
}

