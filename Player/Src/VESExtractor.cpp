/*
 * Copyright (C) 2010 The Android Open Source Project
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


#include "VESExtractor.h"
#include "DataSource.h"
#include "MediaBuffer.h"
#include "MediaSource.h"
#include "MetaData.h"
#include "MediaDefs.h"
#include "SigmaMediaPlayerImpl.h"
#include <String8.h>
#include <string.h>
#include <Utils.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "SIGM_Media_API.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define FUN_START
#define FUN_END

#define MAX_BUFFER_LEN (1024*1024)
#define DATA_UNIT_SIZE 	(1024 * 512)

char buf[MAX_BUFFER_LEN] = {0};

////////////////////////////////////////////////////////////////////////////////
static void hexdump(const void *_data, size_t size) {
		const uint8_t *data = (const uint8_t *)_data;
		size_t offset = 0;
		while (offset < size) {
			printf("0x%04x  ", offset);

			size_t n = size - offset;
			if (n > 16) {
						n = 16;
			}

			for (size_t i = 0; i < 16; ++i) {
					if (i == 8) {
							printf(" ");
					}

					if (offset + i < size) {
							printf("%02x ", data[offset + i]);
					} else {
							printf("   ");
					}
			}

			printf(" ");

			for (size_t i = 0; i < n; ++i) {
					if (isprint(data[offset + i])) {
							printf("%c", data[offset + i]);
					} else {
							printf(".");
					}
			}

			printf("\n");

			offset += 16;
	}
}

static Video_CodingType_e convertType(unsigned int type){
	Video_CodingType_e eType = SIGM_VIDEO_CodingUnused;
	switch(type){
		case 0x10:
		case 0x11:
			eType = SIGM_VIDEO_CodingVC1;
			break;
		case 0x28:
		case 0x29:
			eType = SIGM_VIDEO_CodingRV;
			break;
		case 0x36:
			eType = SIGM_VIDEO_CodingVP6;
			break;
		case 0x37:
			eType = SIGM_VIDEO_CodingVP8;
			break;
		case 0x38:
		case 0x39:
			eType = SIGM_VIDEO_CodingMPEG4;
			break;
		default:
			break;
	}
	return eType;
}

static int ves_find_stream_info(FILE * fp, unsigned int * vType){	
	int size = MAX_BUFFER_LEN;
	int i = 0, type = 0;
	uint32_t code = -1;
	
	int ret = -1;

	if(fp == NULL) {
		return -1;
	}


	ret = _fseeki64(fp,0,SEEK_END);
    if(ret >= 0) 
		size = ftell(fp);
	
	if(size > MAX_BUFFER_LEN) size = MAX_BUFFER_LEN;

	_fseeki64(fp,0L, SEEK_SET);
	//rewind(fp);

	ret  = fread((void *)buf, 1, size, fp);

	if(ret != size){
		ret = -1;
		return ret;
	}
	*vType = SIGM_VIDEO_CodingUnused;
    for (i = 0; i < size; i++) {
		code = (code << 8) + buf[i];
        if ((code & 0xffffff00) == 0x100) {
			type = code & 0xFF;
			if( 0x31 != type) continue;
			if(size-i>12) {
				if(0x4e == buf[i+3] && 0x58 == buf[i+9] && 0x50 == buf[i+12]){
					if( 0x36 == buf[i+5]|| 0x10 == buf[i+5] || 0x11 == buf[i+5] || 0x28 == buf[i+5] || 0x29 == buf[i+5] || 0x37 == buf[i+5] || 0x38 == buf[i+5]){
						type = buf[i+5];
						//both vp6 and vp8 are using 0x36
						if(44 == buf[i+4]) type +=1; //change vp8 to 0x37
						*vType = (unsigned int)convertType(type);
						return 0;
					}
					else {
						printf("unknown type %x\n",buf[i+5]);
						return -1;
					}
				}
			}
			else 
				return -1;
        }
    }
	return -1;
}

static FILE * ves_open_input(const char * fileName) {
	FILE* fp = NULL;
	int32_t res = 0;
	
	if(fileName == NULL) {
		return NULL;
	}

	res = fopen_s(&fp,fileName, "rb");

    if (res >= 0) {
		printf("Open file ok! file:%s\n", fileName);
    } else {
        printf("Can't Open file %s error %s\n", fileName, strerror(errno));
        fp = NULL;
    }
	return fp;
}

static int ves_close_input(FILE * fp) {
	if(fp == NULL) {
		return -1;
	}
	fclose(fp);
	return 0;
}

VideoESSource::VideoESSource(
				const sp<VideoESExtractor> &extractor, size_t trackindex)
		: mExtractor(extractor),
		mTrackIndex(trackindex),
		mBuffer(NULL),
		bEOS(false),
		mStatus(NONE),
		eType(SIGM_VIDEO_CodingUnused){
			const char *mime;
			mExtractor->mTracks.itemAt(trackindex).mMeta->findCString(kKeyMIMEType, &mime);
			if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)) {
				eType = SIGM_VIDEO_CodingMPEG4;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_WMV3)) {
				eType = SIGM_VIDEO_CodingVC1;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_RV)) {
				eType = SIGM_VIDEO_CodingRV;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP6)) {
			    eType = SIGM_VIDEO_CodingVP6;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP8)) {
				eType = SIGM_VIDEO_CodingVP8;
			}else {
				eType = SIGM_VIDEO_CodingUnused;
			}

#ifdef DEBUGFILE
			 mFile = fopen (DEBUGFILE, "wb+");
#endif

}

Error_Type_e VideoESSource::start(MetaData *params) {
		run();
		return SIGM_ErrorNone;
}

Error_Type_e VideoESSource::stop() {
	    requestExitAndWait();
		return SIGM_ErrorNone;
}

sp<MetaData> VideoESSource::getFormat() {
		return mExtractor->mTracks.itemAt(mTrackIndex).mMeta;
}

Error_Type_e VideoESSource::read(
				MediaBuffer **out, const ReadOptions *options) {
		*out = NULL;

		int64_t seekTimeUs = -1;//default no seek
		ReadOptions::SeekMode mode;
		//set the current track as active
		mExtractor->setTrackActive(mTrackIndex,true);

		mOptions.getSeekTo(&seekTimeUs, &mode);
	    
		*out = mExtractor->getNextEncFrame(mTrackIndex,-1,mode); 
		
		if(NULL == *out){
				*out = new MediaBuffer(0);
				printf("ves Extractor returned EOS\n");
				return SIGM_ErrorNone;
		}
		return SIGM_ErrorNone;
}

bool VideoESSource::threadLoop()
{
	Error_Type_e err =  SIGM_ErrorNone;
	int64_t timeUs;
	int64_t startMs;
#ifdef HALSYS
	Media_Buffer_t buffer = {0};
#endif
	int32_t size = 0;
	uint32_t flags = 0;

	{
		Mutex::Autolock autoLock(mLock);

		if( mBuffer == NULL && bEOS){
			mStatus |= EOS;
		}	

		if(mStatus & PAUSE_PENDING) {	
			utils_log(AV_DUMP_ERROR,"Video PAUSE_PENDING->PAUDED \n");
			mStatus &= ~PAUSE_PENDING;
			mStatus |= PAUSED;
			mCondition.broadcast();
			return true;
		}else if(mStatus & PAUSED)
		{
			utils_log(AV_DUMP_ERROR,"Video PAUSE_PAUSED \n");
			Sleep(1);
			return true;
		}	

		if(mStatus & EOS) {
			utils_log(AV_DUMP_ERROR,"Video Quit the Thread \n");
			return false;
		}
	}

	if(mBuffer == NULL){
		//utils_log(AV_DUMP_ERROR,"%s read buffer\n",isVideo?"Video":"Audio");
		err = read(&mBuffer, NULL);
	}
	
	//mOptions.clearSeekTo();
	
	size = mBuffer->size();
		
    mBuffer->meta_data()->findInt64(kKeyTime, &timeUs);

	if(size > 0) utils_log(AV_DUMP_ERROR,"Video pts %lld  size %d\n",timeUs,mBuffer->range_length());
	else  utils_log(AV_DUMP_ERROR,"Video last packet  size %d\n",timeUs,mBuffer->range_length());
#ifdef DEBUGFILE
	{
		fwrite((void *)mBuffer->data(),mBuffer->range_length(),1,mFile);
		fflush(mFile);
	}
#endif

	timeUs = (timeUs == -1ULL)? -1ULL:timeUs/1000; //change to ms

#ifdef HALSYS
	buffer.nAllocLen = size;
	buffer.nOffset = mBuffer->range_offset();	
	buffer.nSize = mBuffer->range_length();
	buffer.nTimeStamp = timeUs;
	buffer.nFilledLen =  mBuffer->range_length();
	buffer.pBuffer = (trid_uint8 *)mBuffer->data();
#endif

	if( size == 0)
	{
		flags |= SIGM_BUFFERFLAG_ENDOFSTREAM;
		bEOS = true;
		utils_log(AV_DUMP_ERROR,"Video EOS got \n");
#ifdef DEBUGFILE
	    fclose(mFile);
#endif   
	}

	flags |= SIGM_BUFFERFLAG_VIDEO_BL | SIGM_BUFFERFLAG_BLOCKCALL;
	
#ifdef HALSYS	
	buffer.nFlags = flags;

	 if (mListener != NULL) {
        sp<Listener> listener = mListener.promote();

        if (listener != NULL) {
            err = listener->sendEvent(MEDIA_BUFFERING_UPDATE,0,0,&buffer);
        }
    }
	
	if(SIGM_ErrorNone == err) {
		mBuffer->release();
    	mBuffer = NULL;
	}
	else{
	//	utils_log(AV_DUMP_ERROR,"push error ret %x\n",err);
	}
#else
	mBuffer->release();
    mBuffer = NULL;
#endif
	if(SIGM_ErrorNone == err && size != 0){
 		sp<MetaData> meta = getFormat();
		meta->setInt64(kKeyTargetTime,timeUs);

		if(!meta->findInt64(kKeyStartTime,&startMs)) {
			meta->setInt64(kKeyStartTime,timeUs);
		}
	}
	//else  keep the back up the data
	Sleep(5);
	return true;
}

int VideoESSource::requestExitAndWait() {
	int ret = Thread::requestExitAndWait();
	bEOS = false;
	return ret;
}


Error_Type_e VideoESSource::pause() {
	Mutex::Autolock autoLock(mLock);

	if(mStatus & EOS)
	{
		utils_log(AV_DUMP_ERROR,"PAUSED Video EOS \n");
		mStatus |= PAUSED;
		return SIGM_ErrorNone;
	}

	mStatus |= PAUSE_PENDING;

	utils_log(AV_DUMP_ERROR,"Video PAUSE_PENDING \n");
	
	while((mStatus&PAUSED)== NONE)
		mCondition.wait(mLock);	

	utils_log(AV_DUMP_ERROR,"Video PAUSED \n");
	return SIGM_ErrorNone;
	
}

Error_Type_e VideoESSource::resume() {
	Mutex::Autolock autoLock(mLock);

	mStatus &= ~PAUSED;
	mStatus |= RESUMED;

	utils_log(AV_DUMP_ERROR,"Video RESUME \n");

#ifdef DEBUGFILE
//	rewind(mFile);	
#endif
	return SIGM_ErrorNone;
}

Error_Type_e VideoESSource::seekTo(uint64_t timeMS) {	
	int64_t seekTimeUs = -1;//default no seek
	ReadOptions::SeekMode mode;

	utils_log(AV_DUMP_ERROR,"Video SeekTo %lld ms \n",timeMS);

	if(mBuffer != NULL) {
		mBuffer->release();
    	mBuffer = NULL;
	}

	mOptions.setSeekTo(
             timeMS*1000,
 			 MediaSource::ReadOptions::SEEK_CLOSEST_SYNC);

	mOptions.getSeekTo(&seekTimeUs, &mode);

	//don't carer about the mode
	mExtractor->seekTo(mTrackIndex,seekTimeUs);

	mOptions.clearSeekTo();

	return SIGM_ErrorNone;
}

////////////////////////////////////////////////////////////////////////////////

VideoESExtractor::VideoESExtractor(const sp<DataSource> &source)
	: mDataSource(source),
	mEof(false),
	mCurrentOffset(0),
	mFp(NULL){
	addTracks();
}

VideoESExtractor::~VideoESExtractor() {
		//close the stream
	printf("ves_close_input...\n");
	if(mFp){
		ves_close_input(mFp);
		mFp = NULL;
	}
	mCurrentOffset = 0;
}

size_t VideoESExtractor::countTracks() {
	return mTracks.size(); //only one track for video
}

sp<MediaSource> VideoESExtractor::getTrack(size_t index) {
	if (index >= mTracks.size()) {
		return NULL;
	}
	return new VideoESSource(this, index);
}

sp<MetaData> VideoESExtractor::getTrackMetaData(
				size_t index, uint32_t flags) {
	if (index >= mTracks.size()) {
		return NULL;
	}
		
	return mTracks.itemAt(index).mMeta;
}

int VideoESExtractor::addTracks() {
		Mutex::Autolock autoLock(mLock);
		String8 mStreamBuf;
		Video_CodingType_e type;
		int index = 0;

		if(mTracks.size() > 0){
			printf("why are we trying to add tracks when it is already done!!\n");
			return -1;
		}

		printf("%s %d\n",__FUNCTION__,__LINE__);
		mStreamBuf = mDataSource->getUri();
	
		if(NULL == mFp){	  	
			if((mFp = ves_open_input((const char *)mStreamBuf.string())) == NULL)
			{
				/* Couldn't open file*/ 
				printf("ves_open_input failed for file!!\n");
				return -1;
			} 
		}

		/* Retrieve stream information */
		if(ves_find_stream_info(mFp,(unsigned int *)&type) < 0){
			/* Couldn't find stream information */
			printf("ves_find_stream_info failed \n");
			return -1;
		}
		
		_fseeki64(mFp,0L, SEEK_SET);

		sp<MetaData> meta = new MetaData;
	    switch(type){
			case SIGM_VIDEO_CodingVC1:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_WMV3);
				break;
			case SIGM_VIDEO_CodingRV:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_RV);
				break;
			case SIGM_VIDEO_CodingVP6:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_VP6);
				break;
			case SIGM_VIDEO_CodingVP8:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_VP8);
				break;
			case SIGM_VIDEO_CodingMPEG4:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MPEG4);
				break;
			default:
				printf("unknown video type\n");
				ves_close_input(mFp);
				mFp = NULL;
				return -1;
	    }

		mTracks.push();
		TrackInfo *trackInfo = &mTracks.editItemAt(mTracks.size() - 1);
		trackInfo->mTrackNum = index;
		trackInfo->mMeta = meta;
		printf("Track %d successfully added\n",index);
		return 0;
}



sp<MetaData> VideoESExtractor::getMetaData() {
		sp<MetaData> meta = new MetaData;
		meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_VES);
		return meta;
}

//This is valid if we have multiple video tracks
void VideoESExtractor::setTrackActive(int trackIndex, bool enable) {
}

//Get Next frame of corresponding track..
MediaBuffer * VideoESExtractor::getNextEncFrame(int trackIndex, int64_t seekTime, int seekMode){
		Mutex::Autolock autoLock(mLock);
		MediaBuffer * frame = NULL;
		uint32_t size = 0;

		if(NULL == mFp){
			printf("No valid video file context FATAL!\n");
			return NULL;
		}

		size = fread((void *)buf, 1, DATA_UNIT_SIZE, mFp);

		if(size > 0) {
			MediaBuffer *buffer = new MediaBuffer(size);
			memcpy((void *)buffer->data(),(void *)buf,size);
			buffer->meta_data()->setInt64(kKeyTime, -1ULL);
		
			buffer->set_range(0, size);
			mCurrentOffset += size;
			return buffer;
		}else {
			mEof = true;
			return NULL;
		}
}

void  VideoESExtractor::seekTo(int trackIndex,int64_t time_us){
	bool bIsseekmode = false;
	Mutex::Autolock autoLock(mLock);
	
	return;
}

bool SniffVideoES(
				const sp<DataSource> &source, String8 *mimeType, float *confidence) {
		String8 streamBuf;
		int ret = 0;
		FILE * fp = NULL;

		mimeType->setTo(MEDIA_MIMETYPE_CONTAINER_VES);
		
		streamBuf = source->getUri();
		printf("VES sniff %s\n",streamBuf.string());
		
		//try opening file 
		if ((fp = ves_open_input(streamBuf.string())) == NULL)
		{
				/* Couldn't open file*/ 
				printf("ves_open_input failed for file while sniffing ret %x!!\n",fp);
				*confidence = 0;
				return false;
		}
		else{
				*confidence = 0.0001;
				printf("sniff pass with 0.0001 confidence by ves\n");
				if(fp){

						Video_CodingType_e eType = SIGM_VIDEO_CodingUnused;
						/* Retrieve stream information */
						if((ret = ves_find_stream_info(fp,(unsigned int *)&eType))<0)
						{
								/* Couldn't find stream information */
								*confidence = 0.0f;	   
								printf("ves_find_stream_info failed %d %s ret %x",__LINE__,__FUNCTION__,ret);
								return false;
						}

						printf("Sniff ves type %x\n",eType);
					
						*confidence = 0.2f;	     		
						
						ves_close_input(fp);
						fp = NULL;
				}
		}
		return true;
}
