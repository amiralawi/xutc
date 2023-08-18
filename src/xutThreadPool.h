#ifndef X_THREAD_POOL_H
#define X_THREAD_POOL_H

#include <stddef.h>
#include <stdint.h>

#include "xutThread.h"

#define XUT_POOL_INITIAL_JOB_CAPACITY 4096
//#define XTH_POOL_JOB_QUEUED 0
//#define XTH_POOL_JOB_WORKING 1
//#define XTH_POOL_JOB_DONE 2

typedef int64_t intHandle;

typedef void (*xutTaskPtr)(void*);

typedef struct xutJob{
    intHandle handle;
    //int status;
    void *dataptr;
    xutTaskPtr taskptr;
} xutJob;

typedef struct xutThreadPool{
    xutJob *queue;
    int queueSize;
    int queueCapacity;
    
    xutMutex queueMutex;
    xutMutex idleMutex;
    int idleCounter;

    xutCondition jobs_waiting;
    xutCondition work_complete;
    

    intHandle totalQueued;

    int soq;
    int eoq;

    int nWorkers;
    struct xutThreadPoolWorker *workers;

    //xutThreadCondition newJobCond;
} xutThreadPool;

typedef struct xutThreadPoolWorker{
    int jobHandle;
    int handle;
    xutThread thread;
    int terminate;

    struct xutThreadPool *pool;

    //xutThreadCondition *newJobCond;
} xutThreadPoolWorker;


xutThreadPool* xutThreadPool_init(int nworkers);
void xutThreadPool_destroy(xutThreadPool *pool);
intHandle xutThreadPool_queueJob(xutThreadPool *pool, xutTaskPtr taskptr, void *dataptr);
int xutThreadPool_waitAll(xutThreadPool *pool);
int xutThreadPool_launch(xutThreadPool *pool);


// needs implementation
int xutThreadPool_abort(xutThreadPool *pool);


#endif

