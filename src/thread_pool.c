
#include <pthread.h>
#include <stdlib.h>
#include "thread_pool.h"

static pthread_t *threads = NULL;
static int total_threads = 0;
static int threads_pool_cursor = 0;

void init_thread_pool(size_t num_threads)
{
	if (threads) {
		free(threads);
	}

	threads = (pthread_t*) malloc (sizeof(pthread_t) * num_threads);
	total_threads = num_threads;
}

void thread_pool_add_task(thread_task task, void *task_arg)
{
	pthread_t *curr_thread = &threads[threads_pool_cursor];
	pthread_create(curr_thread, NULL, task, task_arg);

	threads_pool_cursor++;
}

void thread_pool_stop_tasks()
{
	int i;

	for (i = 0; i < total_threads; i++) {
		pthread_join(threads[i], NULL);
	}
}

void cleanup_thread_pool()
{
	if (threads) {
		free(threads);
		threads = NULL;
	}

	total_threads = 0;
	threads_pool_cursor = 0;
}

