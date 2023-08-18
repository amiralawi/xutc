#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "xutThreadPool.h"



int _get_eoq_index(xutThreadPool *pool);
int _get_soq_index(xutThreadPool *pool);

static int _increment_queue_index(xutThreadPool *pool, int i){
    return (i + 1) % pool->queueCapacity;
}

static int _xutThreadPool_resizeQueue(xutThreadPool *pool, int newsize){
    if(newsize <= pool->queueCapacity){
        return pool->queueCapacity;
    }

    xutJob *oldQueue = pool->queue;
    xutJob *newQueue = malloc(newsize * sizeof(xutJob));

    memset(newQueue, 0, newsize * sizeof(xutJob));

    int i0, i1, j0, j1;
    if(pool->eoq >= pool->soq){
        // no wrap-around - usually only happens if manually resized and queue is small
        // can also happen if queue happens to be perfectly aligned
        memcpy(newQueue, oldQueue + pool->soq, sizeof(xutJob) * pool->queueSize);
    }
    else{
        //queue is wrapping around the end -> break copy into two segments
        int n_segment_0 = pool->queueCapacity - pool->soq;
        int n_segment_1 = pool->soq;
        
        //int i0 = pool->soq;
        //int i1 = pool->queueCapacity - 1;
        //int j0 = 0;
        //int j1 = pool->soq;

        memcpy(newQueue, oldQueue + pool->soq,   n_segment_0 * sizeof(xutJob));
        memcpy(newQueue + n_segment_0, oldQueue, n_segment_1 * sizeof(xutJob));
    }

    //cleanup
    pool->queue = newQueue;
    free(oldQueue);

    pool->queueCapacity = newsize;
    pool->soq = 0;
    pool->eoq = pool->queueSize;

    return newsize;
}



static intHandle _xutThreadPool_pushJob(xutThreadPool *pool, xutTaskPtr taskptr, void *dataptr){
    if(pool->queueSize >= pool->queueCapacity){
        _xutThreadPool_resizeQueue(pool, pool->queueCapacity * 2);
    }

    pool->queueSize++;
    pool->totalQueued++;

    intHandle newHandle = pool->totalQueued;
    
    pool->queue[pool->eoq].taskptr = taskptr;
    pool->queue[pool->eoq].dataptr = dataptr;
    pool->queue[pool->eoq].handle  = newHandle;

    pool->eoq = _increment_queue_index(pool, pool->eoq);
    return newHandle;
}

static int _xutThreadPool_popJob(xutThreadPool *pool, xutJob *job){
    if(pool->queueSize == 0){
        return 0;
    }

    *job = pool->queue[pool->soq];
    
    //cleanup
    pool->soq++;
    pool->queueSize--;
    return 1;
}


static void xutWorkerFunc(void *arg){
    xutThreadPoolWorker *w = (xutThreadPoolWorker*)arg;
    
    xutJob job;
    int found_new_job = 0;
    while(!w->terminate){
        xutMutex_lock(&w->pool->queueMutex);
        found_new_job = _xutThreadPool_popJob(w->pool, &job);
        

        if(found_new_job){
            xutMutex_unlock(&w->pool->queueMutex);
            w->jobHandle = job.handle;
            job.taskptr(job.dataptr);
        }
        else{
            
            xutMutex_lock(&w->pool->idleMutex);
            w->pool->idleCounter++;
            if(w->pool->idleCounter == w->pool->nWorkers){
                xutCondition_broadcast(&w->pool->work_complete);
            }
            xutMutex_unlock(&w->pool->idleMutex);

            // condition_wait automatically/atomically unlocks queueMutex, but reacquires lock prior to returning
            xutCondition_wait(&w->pool->jobs_waiting, &w->pool->queueMutex);
            xutMutex_unlock(&w->pool->queueMutex);

            xutMutex_lock(&w->pool->idleMutex);
            w->pool->idleCounter--;
            xutMutex_unlock(&w->pool->idleMutex);
        }
    }
    
}


int xutThreadPool_launch(xutThreadPool *pool){
    // initialize workers & kickoff threads
    for(int i = 0; i < pool->nWorkers; i++){
        xutThread_create(&pool->workers[i].thread, xutWorkerFunc, pool->workers + i);
    }
}

xutThreadPool* xutThreadPool_init(int nworkers){
    xutThreadPool *pool = malloc(sizeof(xutThreadPool));

    pool->nWorkers = nworkers;
    pool->workers = malloc(sizeof(xutThreadPoolWorker) * nworkers);

    xutCondition_create(&pool->jobs_waiting);
    xutMutex_create(&pool->queueMutex);
    xutMutex_create(&pool->idleMutex);

    pool->idleCounter = 0;
    pool->queueSize = 0;
    pool->totalQueued = 0;
    pool->queueCapacity = 0;
    _xutThreadPool_resizeQueue(pool, XUT_POOL_INITIAL_JOB_CAPACITY);

    for(int i = 0; i < nworkers; i++){
        pool->workers[i].terminate = 0;
        pool->workers[i].pool = pool;
        pool->workers[i].handle = i;
    }

    return pool;
}

void xutThreadPool_destroy(xutThreadPool *pool){
    for(int i = 0; i < pool->nWorkers; i++){
        pool->workers[i].terminate = 1;
    }
    xutCondition_broadcast(&pool->jobs_waiting);
    xutThreadPool_waitAll(pool);

    xutMutex_destroy(&pool->queueMutex);
    xutMutex_destroy(&pool->idleMutex);
    xutCondition_destroy(&pool->jobs_waiting);
    free(pool->queue);
    free(pool);
}

intHandle xutThreadPool_queueJob(xutThreadPool *pool, xutTaskPtr taskptr, void *dataptr){
    if(taskptr == NULL){
        return 0;
    }

    // add job to pool
    xutMutex_lock(&pool->queueMutex);
    intHandle ret = _xutThreadPool_pushJob(pool, taskptr, dataptr);
    

    // if there is a waiting worker then one of them will be woken up to process job
    // if not, all workers are busy and one will automatically pick up the new job when
    // done with its current task
    xutCondition_signal(&pool->jobs_waiting);
    xutMutex_unlock(&pool->queueMutex);

    return ret;
    
}

int xutThreadPool_waitAll(xutThreadPool *pool){
    xutMutex_lock(&pool->idleMutex);
    if(pool->idleCounter == pool->nWorkers){
        xutMutex_unlock(&pool->idleMutex);
        return 1;
    }
    xutCondition_wait(&pool->work_complete, &pool->idleMutex);
    xutMutex_unlock(&pool->idleMutex);

    return 1;
}
