/* 
   list.h

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

#ifndef __LIST_H__
#define __LIST_H__

typedef struct node {
    char *object;
    struct node *next;
} list_node_t;

typedef void (*iterate_cb)(char*, void*);

list_node_t *add_object_to_list(list_node_t *list, char *object);
void free_list(list_node_t *list);
void iterate_list_cb(list_node_t *list, iterate_cb cb, void* arg, int offset, int count, char *sflag);

#endif

