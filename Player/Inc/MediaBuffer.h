#ifndef MEDIA_BUFFER_H_

#define MEDIA_BUFFER_H_

#include <pthread.h>
#include <RefBase.h>

#include "MetaData.h"
#include "StrongPointer.h"

class MediaBufferBase {
public:
    MediaBufferBase() {}

    virtual void release() = 0;
    virtual void add_ref() = 0;

protected:
    virtual ~MediaBufferBase() {}

private:
    MediaBufferBase(const MediaBufferBase &);
    MediaBufferBase &operator=(const MediaBufferBase &);
};

class MediaBuffer : public MediaBufferBase {
public:
    // The underlying data remains the responsibility of the caller!
    MediaBuffer(void *data, size_t size);

    MediaBuffer(size_t size);

    // Decrements the reference count and returns the buffer to its
    // associated MediaBufferGroup if the reference count drops to 0.
    virtual void release();

    // Increments the reference count.
    virtual void add_ref();

    void *data() const;
    size_t size() const;

    size_t range_offset() const;
    size_t range_length() const;

    void set_range(size_t offset, size_t length);

    sp<MetaData> meta_data();

    // Clears meta data and resets the range to the full extent.
    void reset();

    // Returns a clone of this MediaBuffer increasing its reference count.
    // The clone references the same data but has its own range and
    // MetaData.
    MediaBuffer *clone();

    int refcount() const;

protected:
    virtual ~MediaBuffer();

private:

    MediaBuffer *mNextBuffer;
    int mRefCount;

    void *mData;
    size_t mSize, mRangeOffset, mRangeLength;

    bool mOwnsData;

    sp<MetaData> mMetaData;

    MediaBuffer *mOriginal;

    void setNextBuffer(MediaBuffer *buffer);
    MediaBuffer *nextBuffer();

    MediaBuffer(const MediaBuffer &);
    MediaBuffer &operator=(const MediaBuffer &);
};

#endif  // MEDIA_BUFFER_H_

