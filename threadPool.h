// Veronika Merkulova 336249362

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {false, true} bool;
/* struct of task:
*  void (*func) (void *) - is func pointer to func to do
*  void* arg - parameters to function to do
*/
typedef struct task
{
    void (*func) (void *);
    void* arg;

} Task;

typedef struct thread_pool
{
    OSQueue* queue_of_tasks;          // queue of tasks to execute 
    pthread_t* threads;               // array of threads of pool
    pthread_mutex_t queue_lock;       // mutex for queue of tasks
    pthread_mutex_t wait_for_stop;    // mutex for main thread to notufy him that there is no tasks to do and he can destroy pool
    pthread_cond_t condition;         // condition of queue
    pthread_cond_t can_be_destroyed;  // condition that main thread is waiting for
    int num_of_threads;               
    bool task_count;
    bool destroyed;                   // boolean var for changing status of pool from work to stop work
    bool shouldStopInsert;            // boolean var for changing status of queue from insert task to stop do this
    

} ThreadPool;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
