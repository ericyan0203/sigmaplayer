/*
 * Copyright (C) 2005 The Android Open Source Project
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <typeinfo>

#include <RefBase.h>

#include <Threads.h>

#ifndef __unused
#define __unused 
#endif

// compile with refcounting debugging enabled
#define DEBUG_REFS                      0

// whether ref-tracking is enabled by default, if not, trackMe(true, false)
// needs to be called explicitly
#define DEBUG_REFS_ENABLED_BY_DEFAULT   0

// whether callstack are collected (significantly slows things down)
#define DEBUG_REFS_CALLSTACK_ENABLED    0

// folder where stack traces are saved when DEBUG_REFS is enabled
// this folder needs to exist and be writable
#define DEBUG_REFS_CALLSTACK_PATH       "/data/debug"

// log all reference counting operations
#define PRINT_REFS                      0

#define INITIAL_STRONG_VALUE (1<<28)

// ---------------------------------------------------------------------------

#ifndef WIN32
static int32_t atomic_cmpxchg(int32_t oldvalue, int32_t newvalue,  volatile int32_t* addr)
{
	if(*addr == oldvalue)
	{
		*addr = newvalue;
		return 0;
	}
	else
	{	
		return 1;
	}
}
#else
static int32_t atomic_cmpxchg(int32_t oldvalue, int32_t newvalue,  volatile int32_t* addr)
{
	const int32_t c = InterlockedCompareExchange((volatile LONG *)addr,newvalue,oldvalue);

	if(c == oldvalue)
	{
		return 0;
	}
	else
	{	
		return 1;
	}
}

#endif

class RefBase::weakref_impl : public RefBase::weakref_type
{
public:
    volatile int32_t    mStrong;
    volatile int32_t    mWeak;
    RefBase* const      mBase;
    volatile int32_t    mFlags;

    weakref_impl(RefBase* base)
        : mStrong(INITIAL_STRONG_VALUE)
        , mWeak(0)
        , mBase(base)
        , mFlags(0)
    {
    }

    void addStrongRef(const void* /*id*/) { }
    void removeStrongRef(const void* /*id*/) { }
    void renameStrongRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void addWeakRef(const void* /*id*/) { }
    void removeWeakRef(const void* /*id*/) { }
    void renameWeakRefId(const void* /*old_id*/, const void* /*new_id*/) { }
    void printRefs() const { }
    void trackMe(bool, bool) { }

};

// ---------------------------------------------------------------------------

void RefBase::incStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->incWeak(id);
    
    refs->addStrongRef(id);
#ifndef WIN32	
    const int32_t c = refs->mStrong;
	refs->mStrong++;
    //ALOG_ASSERT(c > 0, "incStrong() called on %p after last strong ref", refs);

    if (c != INITIAL_STRONG_VALUE)  {
        return;
    }
#else
	const int32_t c = InterlockedIncrement((volatile LONG *)&refs->mStrong);

	if(c != (INITIAL_STRONG_VALUE+1)) {
		return;
	}
#endif
	refs->mStrong -= INITIAL_STRONG_VALUE;

	refs->mBase->onFirstRef();
}

void RefBase::decStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->removeStrongRef(id);

#ifndef WIN32	
    const int32_t c = refs->mStrong;
	refs->mStrong--;

    if (c == 1) {
        refs->mBase->onLastStrongRef(id);
        if ((refs->mFlags&OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            delete this;
        }
    }
#else
	const int32_t c = InterlockedDecrement((volatile LONG *)&refs->mStrong);
	if (c == 0) {
        refs->mBase->onLastStrongRef(id);
        if ((refs->mFlags&OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            delete this;
        }
    }
#endif
    refs->decWeak(id);
}

void RefBase::forceIncStrong(const void* id) const
{
    weakref_impl* const refs = mRefs;
    refs->incWeak(id);
    
    refs->addStrongRef(id);
#ifndef WIN32
    const int32_t c = refs->mStrong;
	refs->mStrong++;
#else
	const int32_t c = InterlockedIncrement((volatile LONG *)&refs->mStrong);
#endif
    switch (c) {
#ifndef WIN32
    case INITIAL_STRONG_VALUE:
#else
	case INITIAL_STRONG_VALUE + 1:
#endif
		refs->mStrong -= INITIAL_STRONG_VALUE;
        // fall through...
    case 0:
        refs->mBase->onFirstRef();
    }
}

int32_t RefBase::getStrongCount() const
{
    return mRefs->mStrong;
}

RefBase* RefBase::weakref_type::refBase() const
{
    return static_cast<const weakref_impl*>(this)->mBase;
}

void RefBase::weakref_type::incWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    impl->addWeakRef(id);
#ifndef WIN32
    const int32_t c  =  impl->mWeak;
	impl->mWeak++;
#else
	 const int32_t c = InterlockedIncrement((volatile LONG *)&impl->mWeak);
#endif
   // ALOG_ASSERT(c >= 0, "incWeak called on %p after last weak ref", this);
}


void RefBase::weakref_type::decWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    impl->removeWeakRef(id);
#ifndef WIN32	
    const int32_t c = impl->mWeak;
	impl->mWeak--;
    
    if (c != 1) return;
#else
	 const int32_t c = InterlockedDecrement((volatile LONG *)&impl->mWeak);
	 if(c != 0) return;
#endif
    if ((impl->mFlags&OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_STRONG) {
        // This is the regular lifetime case. The object is destroyed
        // when the last strong reference goes away. Since weakref_impl
        // outlive the object, it is not destroyed in the dtor, and
        // we'll have to do it here.
        if (impl->mStrong == INITIAL_STRONG_VALUE) {
            // Special case: we never had a strong reference, so we need to
            // destroy the object now.
            delete impl->mBase;
        } else {
            // ALOGV("Freeing refs %p of old RefBase %p\n", this, impl->mBase);
            delete impl;
        }
    } else {
        // less common case: lifetime is OBJECT_LIFETIME_{WEAK|FOREVER}
        impl->mBase->onLastWeakRef(id);
        if ((impl->mFlags&OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_WEAK) {
            // this is the OBJECT_LIFETIME_WEAK case. The last weak-reference
            // is gone, we can destroy the object.
            delete impl->mBase;
        }
    }
}

bool RefBase::weakref_type::attemptIncStrong(const void* id)
{
    incWeak(id);
    
    weakref_impl* const impl = static_cast<weakref_impl*>(this);
    int32_t curCount = impl->mStrong;

    while (curCount > 0 && curCount != INITIAL_STRONG_VALUE) {
        // we're in the easy/common case of promoting a weak-reference
        // from an existing strong reference.
        if (atomic_cmpxchg(curCount, curCount+1, &impl->mStrong) == 0) {
            break;
        }
        // the strong count has changed on us, we need to re-assert our
        // situation.
        curCount = impl->mStrong;
    }
    
    if (curCount <= 0 || curCount == INITIAL_STRONG_VALUE) {
        // we're now in the harder case of either:
        // - there never was a strong reference on us
        // - or, all strong references have been released
        if ((impl->mFlags&OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_STRONG) {
            // this object has a "normal" life-time, i.e.: it gets destroyed
            // when the last strong reference goes away
            if (curCount <= 0) {
                // the last strong-reference got released, the object cannot
                // be revived.
                decWeak(id);
                return false;
            }

            // here, curCount == INITIAL_STRONG_VALUE, which means
            // there never was a strong-reference, so we can try to
            // promote this object; we need to do that atomically.
            while (curCount > 0) {
                if (atomic_cmpxchg(curCount, curCount + 1,
                        &impl->mStrong) == 0) {
                    break;
                }
                // the strong count has changed on us, we need to re-assert our
                // situation (e.g.: another thread has inc/decStrong'ed us)
                curCount = impl->mStrong;
            }

            if (curCount <= 0) {
                // promote() failed, some other thread destroyed us in the
                // meantime (i.e.: strong count reached zero).
                decWeak(id);
                return false;
            }
        } else {
            // this object has an "extended" life-time, i.e.: it can be
            // revived from a weak-reference only.
            // Ask the object's implementation if it agrees to be revived
            if (!impl->mBase->onIncStrongAttempted(FIRST_INC_STRONG, id)) {
                // it didn't so give-up.
                decWeak(id);
                return false;
            }
            // grab a strong-reference, which is always safe due to the
            // extended life-time.
#ifndef WIN32
            curCount = impl->mStrong;
			impl->mStrong++;
#else
			curCount =  impl->mStrong;
			InterlockedIncrement((volatile LONG *)&impl->mStrong);
#endif
		}

        // If the strong reference count has already been incremented by
        // someone else, the implementor of onIncStrongAttempted() is holding
        // an unneeded reference.  So call onLastStrongRef() here to remove it.
        // (No, this is not pretty.)  Note that we MUST NOT do this if we
        // are in fact acquiring the first reference.
        if (curCount > 0 && curCount < INITIAL_STRONG_VALUE) {
            impl->mBase->onLastStrongRef(id);
        }
    }
    
    impl->addStrongRef(id);

    // now we need to fix-up the count if it was INITIAL_STRONG_VALUE
    // this must be done safely, i.e.: handle the case where several threads
    // were here in attemptIncStrong().
    curCount = impl->mStrong;
    while (curCount >= INITIAL_STRONG_VALUE) {
      
        if (atomic_cmpxchg(curCount, curCount-INITIAL_STRONG_VALUE,
                &impl->mStrong) == 0) {
            break;
        }
        // the strong-count changed on us, we need to re-assert the situation,
        // for e.g.: it's possible the fix-up happened in another thread.
        curCount = impl->mStrong;
    }

    return true;
}

bool RefBase::weakref_type::attemptIncWeak(const void* id)
{
    weakref_impl* const impl = static_cast<weakref_impl*>(this);

    int32_t curCount = impl->mWeak;

    while (curCount > 0) {
        if (atomic_cmpxchg(curCount, curCount+1, &impl->mWeak) == 0) {
            break;
        }
        curCount = impl->mWeak;
    }

    if (curCount > 0) {
        impl->addWeakRef(id);
    }

    return curCount > 0;
}

int32_t RefBase::weakref_type::getWeakCount() const
{
    return static_cast<const weakref_impl*>(this)->mWeak;
}

void RefBase::weakref_type::printRefs() const
{
    static_cast<const weakref_impl*>(this)->printRefs();
}

void RefBase::weakref_type::trackMe(bool enable, bool retain)
{
    static_cast<weakref_impl*>(this)->trackMe(enable, retain);
}

RefBase::weakref_type* RefBase::createWeak(const void* id) const
{
    mRefs->incWeak(id);
    return mRefs;
}

RefBase::weakref_type* RefBase::getWeakRefs() const
{
    return mRefs;
}

RefBase::RefBase()
    : mRefs(new weakref_impl(this))
{
}

RefBase::~RefBase()
{
    if (mRefs->mStrong == INITIAL_STRONG_VALUE) {
        // we never acquired a strong (and/or weak) reference on this object.
        delete mRefs;
    } else {
        // life-time of this object is extended to WEAK or FOREVER, in
        // which case weakref_impl doesn't out-live the object and we
        // can free it now.
        if ((mRefs->mFlags & OBJECT_LIFETIME_MASK) != OBJECT_LIFETIME_STRONG) {
            // It's possible that the weak count is not 0 if the object
            // re-acquired a weak reference in its destructor
            if (mRefs->mWeak == 0) {
                delete mRefs;
            }
        }
    }
    // for debugging purposes, clear this.
    const_cast<weakref_impl*&>(mRefs) = NULL;
}

void RefBase::extendObjectLifetime(int32_t mode)
{
    //android_atomic_or(mode, &mRefs->mFlags);
#ifndef WIN32
    mRefs->mFlags |= mode;
#else
	InterlockedOr((volatile LONG *)&mRefs->mFlags,mode);
#endif
}

void RefBase::onFirstRef()
{
}

void RefBase::onLastStrongRef(const void* /*id*/)
{
}

bool RefBase::onIncStrongAttempted(uint32_t flags, const void* /*id*/)
{
    return (flags&FIRST_INC_STRONG) ? true : false;
}

void RefBase::onLastWeakRef(const void* /*id*/)
{
}

// ---------------------------------------------------------------------------

void RefBase::renameRefs(size_t /*n*/, const ReferenceRenamer& /*renamer*/) { }


void RefBase::renameRefId(weakref_type* ref,
        const void* old_id, const void* new_id) {
    weakref_impl* const impl = static_cast<weakref_impl*>(ref);
    impl->renameStrongRefId(old_id, new_id);
    impl->renameWeakRefId(old_id, new_id);
}

void RefBase::renameRefId(RefBase* ref,
        const void* old_id, const void* new_id) {
    ref->mRefs->renameStrongRefId(old_id, new_id);
    ref->mRefs->renameWeakRefId(old_id, new_id);
}
