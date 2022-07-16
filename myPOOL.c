// Veronika Merkulova 336249362

#include "threadPool.h"
#include <pthread.h>

void free_memory(ThreadPool *threadPool)
{
    printf("FREEEEE!\n");
    if (threadPool != NULL)
    {
        pthread_mutex_destroy(&(threadPool->queue_lock));
        pthread_cond_destroy(&(threadPool->condition));
        pthread_cond_destroy(&(threadPool->can_be_destroyed));
        while (!osIsQueueEmpty(threadPool->queue_of_tasks)) {
            free((Task *) osDequeue(threadPool->queue_of_tasks));
        }
        osDestroyQueue(threadPool->queue_of_tasks);
        free(threadPool->threads);
        free(threadPool);
    }
}


void *doTask(void *arg)
{
    ThreadPool *tp = arg;
    while (1)
    {
        pthread_mutex_lock(&(tp->queue_lock));
        while (osIsQueueEmpty(tp->queue_of_tasks) && !(tp->destroyed))
        {
            if (tp->destroyed)
            {
               pthread_mutex_unlock(&(tp->queue_lock));
               pthread_exit(NULL);
            }
            pthread_cond_wait(&(tp->condition), &(tp->queue_lock));
        }
        if (tp->destroyed)
        {
            printf("should break cause destroyed\n");
            pthread_mutex_unlock(&(tp->queue_lock));
            pthread_exit(NULL);
        }
        void (*function)(void *);
        void* par;
        Task *task;
        task = osDequeue(tp->queue_of_tasks);
        function = task->func;
        par = task->arg;
        if (task == NULL)
        {
            printf("task == null\n");
        }
        tp->task_count--;
        if( tp->shouldStopInsert && (!tp->destroyed) && (tp->task_count == 0))
        {
            pthread_cond_signal(&(tp->can_be_destroyed));
        }
        pthread_mutex_unlock(&(tp->queue_lock));
        printf("after mut unlock\n");
        function(par);
        free(task);
    }
}

ThreadPool *tpCreate(int numOfThreads)
{
    ThreadPool *t_p;
    t_p = (ThreadPool *)malloc(sizeof(ThreadPool));
    printf("create %d threads\n", numOfThreads);

    t_p->queue_of_tasks = osCreateQueue();
    printf("create\n");

    t_p->task_count = 0;
    t_p->destroyed = false;
    t_p->shouldContinue = true; // shouldContinue = var for destroying. if we get shouldWaitForTask = 1, tPool will continue to get tasks from queue while it`s not empty.
    t_p->shouldStopInsert = false;
    t_p->num_of_threads = numOfThreads;
    t_p->threads = (pthread_t *)malloc(numOfThreads * sizeof(pthread_t));
    printf("create3\n");

    if (t_p->threads == NULL)
    {
        perror("Memory allocation failed in tpCreate.");
    }
    else
    {
        pthread_mutex_init(&t_p->queue_lock, NULL);
        pthread_cond_init(&t_p->condition, NULL);
        pthread_cond_init(&t_p->can_be_destroyed, NULL);
        int i;
        for (i = 0; i < numOfThreads; i++)
        {
            int error;
            error = pthread_create(&(t_p->threads[i]), NULL, doTask, (void *)t_p);
            if (error != 0)
                perror("Can't create thread in tpCreate\n");
        }
    }
    return t_p;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks)
{
    printf("DESTROOYYYY!!!!!!!!!!\n");
    if (threadPool == NULL)
    {
        return;
    }
    threadPool->shouldStopInsert = true;
    if (shouldWaitForTasks == 0)
    {
        threadPool->destroyed = true;
        pthread_mutex_lock(&(threadPool->queue_lock));
        pthread_cond_broadcast(&(threadPool->condition));
        pthread_mutex_unlock(&(threadPool->queue_lock));
    }
    else
    {
        printf("else waiting\n");
        if (threadPool->task_count == 0)
        {
            threadPool->destroyed = true;
            pthread_mutex_lock(&(threadPool->queue_lock));
            pthread_cond_broadcast(&(threadPool->condition));
            pthread_mutex_unlock(&(threadPool->queue_lock));
        }
        else
        {
            pthread_cond_wait(&(threadPool->can_be_destroyed), &(threadPool->queue_lock));
            printf("GOOOTTT SIGNAAALLL\n");
            threadPool->destroyed = true;
            //pthread_mutex_lock(&(threadPool->queue_lock));
            pthread_cond_broadcast(&(threadPool->condition));
            //pthread_mutex_unlock(&(threadPool->queue_lock));

        }
    }
    int i;
    printf("SIZE OF THREADS: %d\n", threadPool->num_of_threads);
    for (i = 0; i < threadPool->num_of_threads; i++)
    {
        printf("###FOR##\n");
        if (pthread_join(threadPool->threads[i], NULL) != 0 )
        {
            perror("pthread_join failure in tpDestroy");
        }
        printf("after join\n");
    }
    free_memory(threadPool);
    return;
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param)
{
    
    if (threadPool == NULL)
    {
        printf("null\n");
    }
    
    if (threadPool->queue_of_tasks == NULL)
    {
        printf("null1\n");
    }

    if (!threadPool->shouldStopInsert)
    {
        Task *new_task;

        new_task = (Task *)malloc(sizeof(Task));
        if (new_task != NULL)
        {
            new_task->func = computeFunc;
            new_task->arg = param;
            pthread_mutex_lock(&(threadPool->queue_lock));
            osEnqueue(threadPool->queue_of_tasks, new_task);
            printf("INSERT TO QUEUE\n");
            pthread_cond_broadcast(&(threadPool->condition));
            threadPool->task_count++;
            pthread_mutex_unlock(&(threadPool->queue_lock));
        }
        printf("COUNT OF TASKS: %d\n", threadPool->task_count);
    }
    else
    {
        return -1;
    }
    return 0;
}
