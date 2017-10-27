/* 
   thread_pool.h

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

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

typedef void* (*thread_task) (void *arg);

void init_thread_pool(size_t num_threads);
void thread_pool_add_task(thread_task task, void *task_arg);
void cleanup_thread_pool();

void task_enter_critical_section();
void task_exit_critical_section();

#endif

