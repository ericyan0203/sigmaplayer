#ifndef _LIB_LISTENER_H
#define _LIB_LISTENER_H

#include <Mutex.h>


// callback mechanism for passing messages to MediaPlayer object
typedef void (*notify_callback_t)(void* cookie,
        int msg, int ext1, int ext2, unsigned int *obj);


class Listener: /*public VirtualLightRefBase*/public virtual RefBase {
public:
    Listener(){};                                                     \
    virtual ~Listener(){}; 

	 void   setNotifyCallback(
            void* cookie, notify_callback_t notifyFunc) {
        Mutex::Autolock autoLock(mNotifyLock);
        mCookie = cookie; mNotify = notifyFunc;
    }

    void   sendEvent(int msg, int ext1 = 0, int ext2 = 0,
                          unsigned int *obj=NULL) {
        Mutex::Autolock autoLock(mNotifyLock);
        if (mNotify) mNotify(mCookie, msg, ext1, ext2, obj);
    }

private:	
    Mutex               mNotifyLock;
    void*               mCookie;
    notify_callback_t   mNotify;
};

#endif

