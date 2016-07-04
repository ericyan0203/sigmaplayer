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
    UTILS_OK                = 0,    // Everything's swell.
    UTILS_NO_ERROR          = 0,    // No errors.

    UTILS_UNKNOWN_ERROR     = -256, // INT32_MIN value

    UTILS_NO_MEMORY           = (UTILS_UNKNOWN_ERROR + 1),
    UTILS_INVALID_OPERATION   = (UTILS_UNKNOWN_ERROR + 2),
    UTILS_BAD_VALUE           = (UTILS_UNKNOWN_ERROR + 3),
    UTILS_BAD_TYPE            = (UTILS_UNKNOWN_ERROR + 4),
    UTILS_NAME_NOT_FOUND      = (UTILS_UNKNOWN_ERROR + 5),
    UTILS_PERMISSION_DENIED   = (UTILS_UNKNOWN_ERROR + 6),
    UTILS_NO_INIT             = (UTILS_UNKNOWN_ERROR + 7),
    UTILS_ALREADY_EXISTS      = (UTILS_UNKNOWN_ERROR + 8),
    UTILS_DEAD_OBJECT         = (UTILS_UNKNOWN_ERROR + 9),
    UTILS_FAILED_TRANSACTION  = (UTILS_UNKNOWN_ERROR + 10),
    UTILS_JPARKS_BROKE_IT     = (UTILS_UNKNOWN_ERROR + 11),
    UTILS_BAD_INDEX           = (UTILS_UNKNOWN_ERROR + 12),
    UTILS_NOT_ENOUGH_DATA     = (UTILS_UNKNOWN_ERROR + 13),
    UTILS_WOULD_BLOCK         = (UTILS_UNKNOWN_ERROR + 14),
    UTILS_TIMED_OUT           = (UTILS_UNKNOWN_ERROR + 15),
    UTILS_UNKNOWN_TRANSACTION = (UTILS_UNKNOWN_ERROR + 16),   
    UTILS_FDS_NOT_ALLOWED     = (UTILS_UNKNOWN_ERROR + 17),
};


class Thread :  public VirtualLightRefBase
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
