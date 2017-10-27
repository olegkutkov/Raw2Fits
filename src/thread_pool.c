/* 
   thread_pool.c
    - simple threads launcher and storage

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

#include <pthread.h>
#include <stdlib.h>
#include "thread_pool.h"

static pthread_t *threads = NULL;
static int total_threads = 0;
static int threads_pool_cursor = 0;

static pthread_mutex_t pool_lock;

void init_thread_pool(size_t num_threads)
{
	if (threads) {
		cleanup_thread_pool();
	}

	pthread_mutex_init(&pool_lock, NULL);

	threads = (pthread_t*) malloc (sizeof(pthread_t) * num_threads);
	total_threads = num_threads;
}

void thread_pool_add_task(thread_task task, void *task_arg)
{
	pthread_t *curr_thread = &threads[threads_pool_cursor];
	pthread_create(curr_thread, NULL, task, task_arg);

	threads_pool_cursor++;
}

void cleanup_thread_pool()
{
	int i;

	for (i = 0; i < total_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	if (threads) {
		free(threads);
		threads = NULL;
	}

	total_threads = 0;
	threads_pool_cursor = 0;

	pthread_mutex_destroy(&pool_lock);
}

void task_enter_critical_section()
{
	pthread_mutex_lock(&pool_lock);
}

void task_exit_critical_section()
{
	pthread_mutex_unlock(&pool_lock);
}

