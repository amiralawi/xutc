#ifndef XUT_THREAD_H
#define XUT_THREAD_H

#include "xut.h"

#ifdef _WIN32
    #include <windows.h>
#else
    // linux probably
    #include <pthread.h>
#endif

// cross-platform type definitions
typedef void (*xutThreadFuncPtr)(void *);



// implementation-dependent definitions
#ifdef XUT_PLATFORM_LINUX
    typedef struct xutThread{
        pthread_t _thread;
    } xutThread;

    typedef struct xutMutex{
        pthread_mutex_t _mutex;
    } xutMutex;

    typedef struct xutCondition{
        pthread_cond_t _cond;
    } xutCondition;
#endif // XUT_PLATFORM_LINUX
#ifdef XUT_PLATFORM_WINDOWS
    // TODO
#endif //XUT_PLATFORM_WINDOWS




// cross-platform function prototypes
// xutThread
int xutThread_create(xutThread *thread, xutThreadFuncPtr funcptr, void *data);
int xutThread_join(xutThread *thread);
int xutThread_cancel(xutThread *thread);

// xutMutex
int xutMutex_create(xutMutex *mutex);
int xutMutex_destroy(xutMutex *mutex);
int xutMutex_lock(xutMutex *mutex);
int xutMutex_trylock(xutMutex *mutex);
int xutMutex_unlock(xutMutex *mutex);

// xutCondition
int xutCondition_create(xutCondition *cond);
int xutCondition_destroy(xutCondition *cond);
int xutCondition_signal(xutCondition *cond);
int xutCondition_broadcast(xutCondition *cond);
int xutCondition_wait(xutCondition *cond, xutMutex *mutex);


#endif // XUT_THREAD_H
