/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "MediaBuffer.h"
#include "MetaData.h"


MediaBuffer::MediaBuffer(void *data, size_t size)
    : mNextBuffer(NULL),
      mRefCount(0),
      mData(data),
      mSize(size),
      mRangeOffset(0),
      mRangeLength(size),
      mOwnsData(false),
      mMetaData(new MetaData),
      mOriginal(NULL) {
}

MediaBuffer::MediaBuffer(size_t size)
    : mNextBuffer(NULL),
      mRefCount(0),
      mData(malloc(size)),
      mSize(size),
      mRangeOffset(0),
      mRangeLength(size),
      mOwnsData(true),
      mMetaData(new MetaData),
      mOriginal(NULL) {
}

void MediaBuffer::release() {
    int prevCount = mRefCount--;
    if (prevCount == 1) {
            delete this;
            return;
        }
}

void MediaBuffer::add_ref() {
   mRefCount++;
}

void *MediaBuffer::data() const {
    return mData;
}

size_t MediaBuffer::size() const {
    return mSize;
}

size_t MediaBuffer::range_offset() const {
    return mRangeOffset;
}

size_t MediaBuffer::range_length() const {
    return mRangeLength;
}

void MediaBuffer::set_range(size_t offset, size_t length) {
    if ( offset + length > mSize) {
        printf("error: offset = %zu, length = %zu, mSize = %zu", offset, length, mSize);
    }

    mRangeOffset = offset;
    mRangeLength = length;
}

sp<MetaData> MediaBuffer::meta_data() {
    return mMetaData;
}

void MediaBuffer::reset() {
    mMetaData->clear();
    set_range(0, mSize);
}

MediaBuffer::~MediaBuffer() {
    if (mOwnsData && mData != NULL) {
        free(mData);
        mData = NULL;
    }

    if (mOriginal != NULL) {
        mOriginal->release();
        mOriginal = NULL;
    }
}

void MediaBuffer::setNextBuffer(MediaBuffer *buffer) {
    mNextBuffer = buffer;
}

MediaBuffer *MediaBuffer::nextBuffer() {
    return mNextBuffer;
}

int MediaBuffer::refcount() const {
    return mRefCount;
}

MediaBuffer *MediaBuffer::clone() {
    MediaBuffer *buffer = new MediaBuffer(mData, mSize);
    buffer->set_range(mRangeOffset, mRangeLength);
    buffer->mMetaData = new MetaData(*mMetaData.get());

    add_ref();
    buffer->mOriginal = this;

    return buffer;
}
