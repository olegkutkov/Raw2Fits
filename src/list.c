
#include <stdlib.h>
#include <string.h>
#include "list.h"

list_node_t *add_object_to_list(list_node_t *list, char *object)
{
	list_node_t *new_item = (list_node_t *) malloc(sizeof(list_node_t));

	new_item->object = (char *) malloc(strlen(object) + 1);

	strcpy(new_item->object, object);
	new_item->next = list;

	list = new_item;

	return list;
}

void free_list(list_node_t *list)
{
	if (list == NULL) {
		return;
	}

	list_node_t *head = list;
	list_node_t *prev = NULL;

	while (head != NULL) {
		prev = head;
		head = head->next;

		if (prev->object) {
			free(prev->object);
			prev->object = NULL;
		}

		free(prev);
		prev = NULL;
	}
}

void iterate_list_cb(list_node_t *list, iterate_cb cb, void* arg, int offset, int count, char *sflag)
{
	int curr_offset = 0;
	list_node_t *tmp;

	for (tmp = list; tmp; tmp = tmp->next) {
		if (!*sflag) {
			break;
		}

		if (curr_offset >= offset) {
			if (curr_offset == count) {
				break;
			}

			(*cb) (tmp->object, arg);
		}
	}
}

