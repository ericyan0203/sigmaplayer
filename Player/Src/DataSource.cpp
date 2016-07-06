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
#include "FfmpegExtractor.h"

#include "DataSource.h"
#include "FileSource.h"
#include "String8.h"
#include "SIGM_Types.h"

#ifdef	WIN32 
#include <winsock2.h>
#pragma   comment(lib,"ws2_32.lib") 
//include "pthread.h"
#endif


#define hton64(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntoh64(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))



bool DataSource::getUInt16(off64_t offset, uint16_t *x) {
    *x = 0;

    uint8_t byte[2];
    if (readAt(offset, byte, 2) != 2) {
        return false;
    }

    *x = (byte[0] << 8) | byte[1];

    return true;
}

bool DataSource::getUInt24(off64_t offset, uint32_t *x) {
    *x = 0;

    uint8_t byte[3];
    if (readAt(offset, byte, 3) != 3) {
        return false;
    }

    *x = (byte[0] << 16) | (byte[1] << 8) | byte[2];

    return true;
}

bool DataSource::getUInt32(off64_t offset, uint32_t *x) {
    *x = 0;

    uint32_t tmp;
    if (readAt(offset, &tmp, 4) != 4) {
        return false;
    }

    *x = ntohl(tmp);

    return true;
}

bool DataSource::getUInt64(off64_t offset, uint64_t *x) {
    *x = 0;

    uint64_t tmp;
    if (readAt(offset, &tmp, 8) != 8) {
        return false;
    }

    *x = ntoh64(tmp);

    return true;
}

Error_Type_e DataSource::getSize(off64_t *size) {
    *size = 0;
    return SIGM_ErrorNotSupported;
}

////////////////////////////////////////////////////////////////////////////////

Mutex DataSource::gSnifferMutex;
List<DataSource::SnifferFunc> DataSource::gSniffers;
bool DataSource::gSniffersRegistered = false;

bool DataSource::sniff(
        String8 *mimeType, float *confidence) {
    *mimeType = "";
    *confidence = 0.0f;
   
    {
        Mutex::Autolock autoLock(gSnifferMutex);
        if (!gSniffersRegistered) {
            return false;
        }
    }

    for (List<SnifferFunc>::iterator it = gSniffers.begin();
         it != gSniffers.end(); ++it) {
        String8 newMimeType;
        float newConfidence;
        if ((*it)(this, &newMimeType, &newConfidence)) {
            if (newConfidence > *confidence) {
                *mimeType = newMimeType;
                *confidence = newConfidence;
            }
        }
    }

    return *confidence > 0.0;
}

// static
void DataSource::RegisterSniffer_l(SnifferFunc func) {
    for (List<SnifferFunc>::iterator it = gSniffers.begin();
         it != gSniffers.end(); ++it) {
        if (*it == func) {
            return;
        }
    }

    gSniffers.push_back(func);
}

// static
void DataSource::RegisterDefaultSniffers() {
    Mutex::Autolock autoLock(gSnifferMutex);
    if (gSniffersRegistered) {
        return;
    }

	RegisterSniffer_l(SniffFfmpeg);

    gSniffersRegistered = true;
}

// static
sp<DataSource> DataSource::CreateFromURI(const char *uri) {
   

    bool isWidevine = !strncasecmp("widevine://", uri, 11);

    sp<DataSource> source;
    if (!strncasecmp("file://", uri, 7)) {
        source = new FileSource(uri + 7);
    } 
	
    if (source == NULL || source->initCheck() != SIGM_ErrorNone) {
        return NULL;
    }

    return source;
}

String8 DataSource::getMIMEType() const {
    return String8("application/octet-stream");
}
