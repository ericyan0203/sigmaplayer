/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LIBS_UTILS_THREAD_H
#define _LIBS_UTILS_THREAD_H

#include <sys/types.h>
#include <time.h>

#include <pthread.h>

#include <Condition.h>
#include <Mutex.h>
#include <RefBase.h>
#include <StrongPointer.h>

typedef int (*thread_func_t)(void*);

enum {
    THREAD_OK                = 0,    // Everything's swell.
    THREAD_NO_ERROR          = 0,    // No errors.

    THREAD_UNKNOWN_ERROR     = -256, // INT32_MIN value

    THREAD_NO_MEMORY           = (THREAD_UNKNOWN_ERROR + 1),
    THREAD_INVALID_OPERATION   = (THREAD_UNKNOWN_ERROR + 2),
    THREAD_BAD_VALUE           = (THREAD_UNKNOWN_ERROR + 3),
    THREAD_BAD_TYPE            = (THREAD_UNKNOWN_ERROR + 4),
    THREAD_NAME_NOT_FOUND      = (THREAD_UNKNOWN_ERROR + 5),
    THREAD_PERMISSION_DENIED   = (THREAD_UNKNOWN_ERROR + 6),
    THREAD_NO_INIT             = (THREAD_UNKNOWN_ERROR + 7),
    THREAD_ALREADY_EXISTS      = (THREAD_UNKNOWN_ERROR + 8),
    THREAD_DEAD_OBJECT         = (THREAD_UNKNOWN_ERROR + 9),
    THREAD_FAILED_TRANSACTION  = (THREAD_UNKNOWN_ERROR + 10),
    THREAD_JPARKS_BROKE_IT     = (THREAD_UNKNOWN_ERROR + 11),
    THREAD_BAD_INDEX           = (THREAD_UNKNOWN_ERROR + 12),
    THREAD_NOT_ENOUGH_DATA     = (THREAD_UNKNOWN_ERROR + 13),
    THREAD_WOULD_BLOCK         = (THREAD_UNKNOWN_ERROR + 14),
    THREAD_TIMED_OUT           = (THREAD_UNKNOWN_ERROR + 15),
    THREAD_UNKNOWN_TRANSACTION = (THREAD_UNKNOWN_ERROR + 16),   
    THREAD_FDS_NOT_ALLOWED     = (THREAD_UNKNOWN_ERROR + 17),
};


class Thread : virtual public VirtualLightRefBase
{
public:
    // Create a Thread object, but doesn't create or start the associated
    // thread. See the run() method.
                        Thread();
    virtual             ~Thread();

    // Start the thread in threadLoop() which needs to be implemented.
    virtual int    run(    const char* name = 0,
                                int priority = 0,
                                int stack = 0);
    
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();

    // Good place to do one-time initializations
    virtual int   readyToRun();
    
    // Call requestExit() and wait until this object's thread exits.
    // BE VERY CAREFUL of deadlocks. In particular, it would be silly to call
    // this function from this object's thread. Will return WOULD_BLOCK in
    // that case.
	int requestExitAndWait();

    // Wait until this object's thread exits. Returns immediately if not yet running.
    // Do not call from this object's thread; will return WOULD_BLOCK in that case.
	int join();

    // Indicates whether this thread is running or not.
    bool        isRunning() const;

protected:
    // exitPending() returns true if requestExit() has been called.
    bool        exitPending() const;
    
private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop() = 0;

private:
    Thread& operator=(const Thread&);
    static  int             _threadLoop(void* user);
 
    // always hold mLock when reading or writing
    void *	mThread;
    mutable Mutex           mLock;
    Condition       mThreadExitedCondition;
    int        mStatus;
    // note that all accesses of mExitPending and mRunning need to hold mLock
    volatile bool           mExitPending;
    volatile bool           mRunning;
	sp<Thread>      mHoldSelf;

};


#endif // _LIBS_UTILS_THREAD_H
// ---------------------------------------------------------------------------
