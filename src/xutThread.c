#include "xutThread.h"

#ifdef THD_WINDOWS_NATIVE

    // TODO

#else
// linux

#include <signal.h>

//
// xutThread
//

//todo 
int xutThread_cancel(xutThread *thread);

// xutThread
int xutThread_create(xutThread *thread, xutThreadFuncPtr funcptr, void *data){
    return pthread_create(&thread->_thread, NULL, (void *)funcptr, data);
}
int xutThread_join(xutThread *thread){
    return pthread_join(thread->_thread, NULL);
}


// xutMutex
int xutMutex_create(xutMutex *mutex){
    return pthread_mutex_init(&mutex->_mutex, NULL);
}
int xutMutex_destroy(xutMutex *mutex){
    return pthread_mutex_destroy(&mutex->_mutex);
}
int xutMutex_lock(xutMutex *mutex){
    return pthread_mutex_lock(&mutex->_mutex);
}
int xutMutex_trylock(xutMutex *mutex){
    return pthread_mutex_trylock(&mutex->_mutex);
}
int xutMutex_unlock(xutMutex *mutex){
    return pthread_mutex_unlock(&mutex->_mutex);
}

// xutCondition
int xutCondition_create(xutCondition *cond){
    pthread_cond_init(&cond->_cond, NULL);
}
int xutCondition_destroy(xutCondition *cond){
    return pthread_cond_destroy(&cond->_cond);
}
int xutCondition_signal(xutCondition *cond){
    return pthread_cond_signal(&cond->_cond);
}
int xutCondition_broadcast(xutCondition *cond){
    return pthread_cond_broadcast(&cond->_cond);
}
int xutCondition_wait(xutCondition *cond, xutMutex *mutex){
    return pthread_cond_wait(&cond->_cond, &mutex->_mutex);
}

#endif
