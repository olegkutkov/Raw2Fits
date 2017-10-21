
#include <string.h>
#include <ctype.h>
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
	char *lowcase_buf = (char *) malloc (fname_len);

	finf->file_supported = 0;

	for (il = 0; il < fname_len; il++) {
		lowcase_buf[il] = tolower(fname[il]);
	}

	lowcase_buf[il] = '\0';

	char *extdot = strrchr(lowcase_buf, '.');

	if (extdot == NULL) {
		return;
	}

	char *file_extension = extdot + 1;

	for (vc = 0; vc < 9; vc++) {
		if (strstr(file_extension, all_vendors[0]) != NULL) {
			printf("%s\n", all_vendors[1]);
		}
	}

	free(lowcase_buf);
}

