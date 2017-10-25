
#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

typedef void* (*thread_task) (void *arg);

void init_thread_pool(size_t num_threads);
void thread_pool_add_task(thread_task task, void *task_arg);
void thread_pool_stop_tasks();
void cleanup_thread_pool();

#endif

