// Veronika Merkulova 336249362

#include "threadPool.h"

/*
* free_memory is function for freeing memory
* that was dynamic allocated.
* It is get a pointer to threadPool as an argument.* 
*/
void free_memory(ThreadPool *threadPool)
{
    if (threadPool != NULL)
    {
        pthread_mutex_destroy(&(threadPool->queue_lock));
        pthread_mutex_destroy(&(threadPool->wait_for_stop));
        pthread_cond_destroy(&(threadPool->condition));
        pthread_cond_destroy(&(threadPool->can_be_destroyed));
        while (!osIsQueueEmpty(threadPool->queue_of_tasks))
        {
            free((Task *)osDequeue(threadPool->queue_of_tasks));
        }
        if (threadPool->queue_of_tasks != NULL)
        {
            osDestroyQueue(threadPool->queue_of_tasks);
        }
        if (threadPool->threads != NULL)
        {
            free(threadPool->threads);
        }
        free(threadPool);
        
    }
}

/*
* doTask is function for execute task from queue.
* It is get a pointer to threadPool as an argument. * 
*/
void *doTask(void *arg)
{
    ThreadPool *tp = arg;
    while (1)
    {
        if (pthread_mutex_lock(&(tp->queue_lock)) != 0)
        {
            perror("pthread_mutex_lock failed in doTask");
            free_memory(tp);
            exit(-1);
        }
        while (osIsQueueEmpty(tp->queue_of_tasks) && !(tp->destroyed)) //thread here while there is no tasks in queue_of_tasks
        {
            if (tp->destroyed) //to catch thread if he is executed this part of code and notify him that pool is gonna be destroyed
            {
                if (pthread_mutex_unlock(&(tp->queue_lock)) != 0)
                {
                    perror("pthread_mutex_unlock failed in doTask");
                    free_memory(tp);
                    exit(-1);
                }
                pthread_exit(NULL);
            }
            if (pthread_cond_wait(&(tp->condition), &(tp->queue_lock)) != 0) //waiting for task to execute
            {
                perror("pthread_cond_wait failed in doTask");
                free_memory(tp);
                exit(-1);
            }
        }
        if (tp->destroyed)
        {
            if (pthread_mutex_unlock(&(tp->queue_lock)) != 0)
            {
                perror("pthread_mutex_unlock failed in doTask");
                free_memory(tp);
                exit(-1);
            }
            pthread_exit(NULL);
        }
        void (*function)(void *);
        void *par;
        Task *task;
        task = osDequeue(tp->queue_of_tasks);
        function = task->func;
        par = task->arg;
        tp->task_count--;
        // if pool is gonna be destroyed but we want to wait for all task we can catch here a thread that get a last task
        // and send signal to main thread to change status of tp->destroyed to 1
        if (tp->shouldStopInsert && (!tp->destroyed) && (tp->task_count == 0))
        {
            if (pthread_cond_signal(&(tp->can_be_destroyed)) != 0)
            {
                perror("pthread_cond_signal failed in doTask");
                free_memory(tp);
                exit(-1);
            }
        }
        if (pthread_mutex_unlock(&(tp->queue_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed in doTask");
            free_memory(tp);
            exit(-1);
        }
        function(par);
        free(task);
    }
}

/*
* tpCreate is function for creatin thread pool
* It is get a number of threads to create and do
* needed allocations of memory.
*/
ThreadPool *tpCreate(int numOfThreads)
{
    ThreadPool *t_p;
    t_p = (ThreadPool *)malloc(sizeof(ThreadPool));
    if (t_p == NULL)
    {
        perror("Memory allocation failed in tpCreate");
        exit(-1);
    }
    t_p->queue_of_tasks = osCreateQueue();
    if (t_p->queue_of_tasks == NULL)
    {
        perror("Memory allocation failed in tpCreate");
        free(t_p);
        exit(-1);
    }
    t_p->task_count = 0;
    t_p->destroyed = false;
    t_p->shouldStopInsert = false;
    t_p->num_of_threads = numOfThreads;
    t_p->threads = (pthread_t *)malloc(numOfThreads * sizeof(pthread_t));
    if (t_p->threads == NULL)
    {
        perror("Memory allocation failed in tpCreate");
        free(t_p);
        osDequeue(t_p->queue_of_tasks);
        exit(-1);
    }
    if (pthread_mutex_init(&t_p->queue_lock, NULL) != 0)
    {
        perror("pthread_mutex_init failed in tpCreate");
        free(t_p);
        osDequeue(t_p->queue_of_tasks);
        exit(-1);
    }
    if (pthread_mutex_init(&t_p->wait_for_stop, NULL) != 0)
    {
        perror("pthread_mutex_init failed in tpCreate");
        pthread_mutex_destroy(&(t_p->queue_lock));
        free(t_p);
        osDequeue(t_p->queue_of_tasks);
        exit(-1);
    }
    if (pthread_cond_init(&t_p->condition, NULL) != 0)
    {
        perror("pthread_cond_init failed in tpCreate");
        pthread_mutex_destroy(&(t_p->queue_lock));
        pthread_mutex_destroy(&(t_p->wait_for_stop));
        free(t_p);
        osDequeue(t_p->queue_of_tasks);
        exit(-1);
    }
    if (pthread_cond_init(&t_p->can_be_destroyed, NULL) != 0)
    {
        perror("pthread_cond_init failed in tpCreate");
        pthread_mutex_destroy(&(t_p->queue_lock));
        pthread_mutex_destroy(&(t_p->wait_for_stop));
        pthread_cond_destroy(&(t_p->condition));
        free(t_p);
        osDequeue(t_p->queue_of_tasks);
        exit(-1);
    }
    int i;
    for (i = 0; i < numOfThreads; i++)
    {
        int error;
        error = pthread_create(&(t_p->threads[i]), NULL, doTask, (void *)t_p);
        if (error != 0)
        {
            perror("Can't create thread in tpCreate");
            free_memory(t_p);
            exit(-1);
        }
    }
    return t_p;
}

/*
* tpDestroy is function for destroying pool
* It is get a pointer to threadPool to destroy and var that tell if we need to wait for
* all tasks finished or stop immediently.
*/
void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks)
{
    if (threadPool == NULL)
    {
        perror("Get NULL ThreadPool in tpDestroy");
        exit(-1);
    }
    threadPool->shouldStopInsert = true;
    if (shouldWaitForTasks == 0)
    {
        threadPool->destroyed = true;
        if (pthread_mutex_lock(&(threadPool->queue_lock)) != 0)
        {
            perror("pthread_mutex_lock failed in tpDestroy");
            free_memory(threadPool);
            exit(-1);
        }
        if (pthread_cond_broadcast(&(threadPool->condition)) != 0)
        {
            perror("pthread_cond_broadcast failed in tpDestroy");
            free_memory(threadPool);
            exit(-1);
        }
        if (pthread_mutex_unlock(&(threadPool->queue_lock)) != 0)
        {
            perror("pthread_mutex_unlock failed in tpDestroy");
            free_memory(threadPool);
            exit(-1);
        }
    }
    else
    {
        if (threadPool->task_count == 0)
        {
            threadPool->destroyed = true;
            if (pthread_cond_broadcast(&(threadPool->condition)) != 0)
            {
                perror("pthread_cond_broadcast failed in tpDestroy");
                free_memory(threadPool);
                exit(-1);
            }
        }
        else
        {
            if (pthread_mutex_lock(&(threadPool->wait_for_stop)) != 0)
            {
                perror("pthread_mutex_lock failed in tpDestroy");
                free_memory(threadPool);
                exit(-1);
            }
            if (pthread_cond_wait(&(threadPool->can_be_destroyed), &(threadPool->wait_for_stop)) != 0)
            {
                perror("pthread_cond_wait failed in tpDestroy");
                free_memory(threadPool);
                exit(-1);
            }
            threadPool->destroyed = true;
            if (pthread_cond_broadcast(&(threadPool->condition)) != 0)
            {
                perror("pthread_cond_broadcast failed in tpDestroy");
                free_memory(threadPool);
                exit(-1);
            }
        }
    }
    int i;
    for (i = 0; i < threadPool->num_of_threads; i++)
    {
        if (pthread_join(threadPool->threads[i], NULL) != 0)
        {
            perror("pthread_join failed in tpDestroy");
            free_memory(threadPool);
            exit(-1);
        }
    }
    free_memory(threadPool);
    return;
}

/*
* tpInsertTask is function for inserting new tasks to queue of pool
* It is get a pointer to threadPool and function to execute with parameters.* 
*/
int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param)
{

    if (threadPool == NULL || threadPool->queue_of_tasks == NULL)
    {
        perror("Segmentation fault: threadPool unreachable in tpInsertTask");
        exit(-1);
    }
    if (!threadPool->shouldStopInsert)
    {
        Task *new_task;
        new_task = (Task *)malloc(sizeof(Task)); //allocation for new task
        if (new_task != NULL)
        {
            new_task->func = computeFunc;
            new_task->arg = param;
            if (pthread_mutex_lock(&(threadPool->queue_lock)) != 0)
            {
                perror("pthread_mutex_lock failed in tpInsertTask");
                free_memory(threadPool);
                exit(-1);
            }
            osEnqueue(threadPool->queue_of_tasks, new_task);
            if (pthread_cond_broadcast(&(threadPool->condition)) != 0)
            {
                perror("pthread_cond_broadcast failed in tpInsertTask");
                free_memory(threadPool);
                exit(-1);
            }
            threadPool->task_count++;
            if (pthread_mutex_unlock(&(threadPool->queue_lock)) != 0)
            {
                perror("pthread_mutex_unlock failed in tpInsertTask");
                free_memory(threadPool);
                exit(-1);
            }
        }
        else
        {
            perror("Memory allocation failed in tpCreate.");
            free_memory(threadPool);
            exit(-1);
        }
    }
    else
    {
        return -1;
    }
    return 0;
}
