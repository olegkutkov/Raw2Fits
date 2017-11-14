/* 
   list.c
    - simple list implementation with some specifics for raw2fits

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
	int curr_offset = 0, total_count = 0;
	list_node_t *tmp;

	for (tmp = list; tmp; tmp = tmp->next) {
		if (!*sflag) {
			break;
		}

		if (curr_offset >= offset) {
			if (total_count == count) {
				break;
			}

			(*cb) (tmp->object, arg);

			total_count++;
		}

		curr_offset++;
	}
}

