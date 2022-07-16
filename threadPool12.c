
#include "threadPool.h"
#include "osqueue.h"
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
static void *task_schdual(void *arg);
void threadpool_release(ThreadPool* threadPool);
ThreadPool* tpCreate(int numOfThreads){
    struct thread_pool *tp;
    tp = (ThreadPool*) malloc(sizeof (ThreadPool));
    if(tp == NULL){
        perror("Memory allocation error");
    }
    tp->threads = (pthread_t *)malloc(sizeof(pthread_t) * numOfThreads);
    tp->queue = osCreateQueue();
    tp->t_count = numOfThreads;
    tp->is_closed = false;
    pthread_cond_init(&(tp->notify), NULL);
    pthread_mutex_init(&(tp->m), NULL);
    int err;
    int i;
    for(i=0;i<numOfThreads;i++)
    {
        err = pthread_create(&tp->threads[i], NULL,task_schdual, tp);
        if (err != 0)
            perror("can't create thread\n");
    }
    return tp;
}
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    struct task_data *s;
    if(threadPool->0){
        return -1;
    }
    s = malloc(sizeof (struct task_data));
    if(s == NULL){
        perror(" Memory allocation error");
    }
    s->arg = param;
    s->function = computeFunc;
    pthread_mutex_lock(&(threadPool->m));
    osEnqueue(threadPool->queue,s);
   pthread_cond_broadcast(&(threadPool->notify));
    pthread_mutex_unlock(&(threadPool->m));
    return 0;
}
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks){
    int i;
    if(threadPool == NULL)
        return;
    pthread_mutex_lock(&(threadPool->m));
    if(shouldWaitForTasks == 0){
    threadPool->is_closed = stop_in_place;
    }
    else
    {
    threadPool->is_closed = clean_stop;
    }
    pthread_cond_broadcast(&(threadPool->notify));
    pthread_mutex_unlock(&(threadPool->m));
      if (threadPool->is_closed == stop_in_place) {
//        osDestroyQueue(threadPool->queue);
//        pthread_mutex_unlock(&(threadPool->m));
        for (i = 0; i < threadPool->t_count; i++) {
            pthread_join(threadPool->threads[i], NULL);
        }
        pthread_mutex_destroy(&(threadPool->m));
        pthread_cond_destroy(&(threadPool->notify));
        threadpool_release(threadPool);
    }
    else
    {
        if((threadPool->is_closed == clean_stop)&& (osIsQueueEmpty(threadPool->queue))){
           threadPool->is_closed = stop_in_place;
        }
    }

}
static void *task_schdual(void *arg){
    struct thread_pool *tp = arg;
    struct task_data *s;
    while (true){
        pthread_mutex_lock(&(tp->m));
        while (osIsQueueEmpty(tp->queue) && !tp->is_closed){
            pthread_cond_wait(&(tp->notify), &(tp->m));
        }
        if((tp->is_closed == stop_in_place) || ((tp->is_closed == clean_stop) &&(osIsQueueEmpty(tp->queue)))){
            break;
        }
        s = osDequeue(tp->queue);
        pthread_mutex_unlock(&(tp->m));
        if(s != NULL){
            s->function(s->arg);
            free(s);
        }
        pthread_mutex_unlock(&(tp->m));
        return NULL;
    }
}
void threadpool_release(ThreadPool* threadPool){
    free(threadPool->threads);
    osDestroyQueue(threadPool->queue);
    free(threadPool);
}