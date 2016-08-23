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

//#define LOG_NDEBUG 0


#include "FfmpegExtractor.h"
#include "VESExtractor.h"
#include "DataSource.h"
#include "MediaDefs.h"
#include "MediaExtractor.h"
#include "MetaData.h"
#include <String8.h>

sp<MetaData> MediaExtractor::getMetaData() {
    return new MetaData;
}

uint32_t MediaExtractor::flags() const {
    return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE | CAN_SEEK;
}

// static
sp<MediaExtractor> MediaExtractor::Create(
        const sp<DataSource> &source, const char *mime) {

	float confidence;
	String8 tmp;
    if (mime == NULL) {
        float confidence;
        if (!source->sniff(&tmp, &confidence)) {
            printf("FAILED to autodetect media content.");

            return NULL;
        }

        mime = tmp.string();
        printf("Autodetected media content as '%s' with confidence %.2f",
             mime, confidence);
    }

    MediaExtractor *ret = NULL;

	if  (!strcasecmp(mime,MEDIA_MIMETYPE_CONTAINER_FFMPEG)){
	    ret = new FfmpegExtractor(source);
	}else if(!strcasecmp(mime,MEDIA_MIMETYPE_CONTAINER_VES)){
	    ret = new VideoESExtractor(source);
	}
	return ret;
}// namespace android
