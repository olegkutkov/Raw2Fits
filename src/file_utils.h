
#include <stdint.h>

typedef struct file_info {
	char file_vendor[25];
	long file_size;
	uint8_t file_supported;
} file_info_t;

void get_file_info(char *fname, file_info_t *finf);
long get_file_size(char *fname);

