Thread Pool
===========
This is a implementation of a simple thread pool.
>“A thread pool is a design pattern where a number of threads are created to perform a number of tasks, which are usually organized in a queue. Typically, there are many more tasks than threads. As soon as a thread completes its task, it will request the next task from the queue until all tasks have been completed. The thread will then sleep until there are new tasks available.” (Wikipedia)

This thread-pool will be created with N threads - so that it can handle at most N tasks at a time.
The __tpInsertTask()__ function is responsible for inserting a new task into a queue of tasks within the thread pool.
A task that has been queued will remain there until one of the threads takes it out in order to handle it. If there is no available thread to perform it, the task will only be performed after one of the threads becomes available. If there is a free thread, he will of course do it. If a thread from the pool thread finishes performing its task, and there are no tasks entered in the queue, it will wait (without waiting busy)! until a new task is queued.
