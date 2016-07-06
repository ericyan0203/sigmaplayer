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

#include "FileSource.h"
#include <sys/types.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

FileSource::FileSource(const char *filename)
    : mFd(NULL),
      mOffset(0),
      mLength(-1){
	int res = 0;
	
    res = fopen_s(&mFd,filename, "rb");

    if (res >= 0) {
		res = _fseeki64(mFd,0,SEEK_END);
        if(res >= 0) {
			mLength = ftell(mFd);
        }else {
        	printf("Failed to seek file '%s'. (%s)", filename, strerror(errno));
			mLength = 0;
        }
    } else {
        printf("Failed to open file '%s'. (%s)", filename, strerror(errno));
    }

	mUri = filename;
}

FileSource::~FileSource() {
    if (mFd != NULL) {
       	fclose(mFd);
        mFd = NULL;
    }
}

Error_Type_e FileSource::initCheck() const {
    return mFd != NULL ? SIGM_ErrorNone : SIGM_ErrorNotReady;
}

ssize_t FileSource::readAt(off64_t offset, void *data, size_t size) {
    if (mFd < 0) {
        return SIGM_ErrorNotReady;
    }

    Mutex::Autolock autoLock(mLock);

    if (mLength >= 0) {
        if (offset >= mLength) {
            return 0;  // read beyond EOF.
        }
        int64_t numAvailable = mLength - offset;
        if ((int64_t)size > numAvailable) {
            size = numAvailable;
        }
    }

   
    int result = _fseeki64(mFd, offset + mOffset, SEEK_SET);
    if (result != 0) {
        printf("seek to %lld failed", offset + mOffset);
        return SIGM_ErrorFailed;
    }

    return fread(data,1, size,mFd);
    
}

Error_Type_e FileSource::getSize(off64_t *size) {
    Mutex::Autolock autoLock(mLock);

    if (mFd == NULL) {
        return SIGM_ErrorNotReady;
    }

    *size = mLength;

    return SIGM_ErrorNone;
}

String8 FileSource::getUri()
{
     return mUri;
}
