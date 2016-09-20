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


#include "MultipleFilesExtractor.h"
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
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#if 0
#include <dirent.h>
#endif
#include "SIGM_Media_API.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define FUN_START
#define FUN_END

#define		H264_SUFFIX 	"h264"
#define 	AVC_SUFFIX  	"avc"
#define 	H265_SUFFIX		"h265"
#define 	HEVC_SUFFIX     "hevc"
#define 	VC1_SUFFIX      "vc1"
#define 	VP8_SUFFIX      "vp8"
#define 	VP9_SUFFIX      "vp9"

#define MAX_BUFFER_LEN (1024*1024)
#define DATA_UNIT_SIZE 	(1024 * 512)
#define MAX_LINE 	1024
static char buf[MAX_BUFFER_LEN] = {0};

const char * prefix_list[] = {"640_480","720_576","1280_720","1920_1080","3840_2160"};

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

static int ves_find_stream_info(const char * fileName, unsigned int * vType){	
	const char *ext;
	int namelen = 0;
	int i =0;
	int match = 0;
	
	if(fileName == NULL) {
		*vType = SIGM_VIDEO_CodingUnused;
		return -1;
	}
	
	for(i = 0; i< sizeof(prefix_list)/sizeof(const char *); i++) {
    	if (strncmp(prefix_list[i], fileName, strlen(prefix_list[i])) == 0) {
        	match = 1;
			break;
    	}
	}

	if(!match) {
		*vType = SIGM_VIDEO_CodingUnused;
		return -1;
	}
	//check the stream type by suffix
	ext = strrchr(fileName, '.');
    if (ext) {
		namelen = strlen(ext+1)-1; //no end '0x2d'
		if(!utils_strncasecmp(H264_SUFFIX,(ext+1),namelen)||!utils_strncasecmp(AVC_SUFFIX,(ext+1),namelen)) {
			*vType = SIGM_VIDEO_CodingAVC;
			return 0;
		}else if(!utils_strncasecmp(H265_SUFFIX,(ext+1),namelen)||!utils_strncasecmp(HEVC_SUFFIX,(ext+1),namelen)) {
			*vType = SIGM_VIDEO_CodingHEVC;
			return 0;
		}else if( !utils_strncasecmp(VC1_SUFFIX,(ext+1),namelen)) {
			*vType = SIGM_VIDEO_CodingVC1;
			return 0;
		}else if( !utils_strncasecmp(VP8_SUFFIX,(ext+1),namelen)) {
			*vType = SIGM_VIDEO_CodingVP8;
			return 0;
		}else if( !utils_strncasecmp(VP9_SUFFIX,(ext+1),namelen)) {
			*vType = SIGM_VIDEO_CodingVP9;
			return 0;
		}
    }
	*vType = SIGM_VIDEO_CodingUnused;
	return -1;
}

static FILE * ves_open_input(const char * fileName, bool isBinary) {
	FILE* fp = NULL;
	int32_t res = 0;
	
	if(fileName == NULL) {
		return NULL;
	}

	if(isBinary) res = fopen_s(&fp,fileName, "rb");
	else res = fopen_s(&fp,fileName, "r");

    if (res >= 0) {
		utils_log(AV_DUMP_ERROR,"Open file ok! file:%s fp %p\n", fileName,fp);
    } else {
        utils_log(AV_DUMP_ERROR,"Can't Open file %s error %s\n", fileName, strerror(errno));
        fp = NULL;
    }
	return fp;
}

static int ves_close_input(FILE * fp) {
	if(fp == NULL) {
		utils_log(AV_DUMP_ERROR,"ves_close_input \n");
		return -1;
	}
	fclose(fp);
	return 0;
}

#if 0
static int custom_filter(const struct dirent *pDir)
{		
	int i =0;
	for(i = 0; i< sizeof(prefix_list)/sizeof(const char *); i++) {
    	if (strncmp(prefix_list[i], pDir->d_name, strlen(prefix_list[i])) == 0 )
        	return 1;
	}
    return 0;
}
#endif

static int get_rand(int n) {//0-n-1
	return (int)(n*rand()/(RAND_MAX+1.0));
}

MultipleVideoESSource::MultipleVideoESSource(const sp<MultipleVideoESExtractor> &extractor, size_t trackindex)
		:mExtractor(extractor),
		mTrackIndex(trackindex),
		mBuffer(NULL),
		bEOS(false),
		mStatus(NONE),
		eType(SIGM_VIDEO_CodingUnused){
			const char *mime;
			mExtractor->mTracks.itemAt(trackindex).mMeta->findCString(kKeyMIMEType, &mime);
			if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
				eType = SIGM_VIDEO_CodingAVC;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_WMV3)) {
				eType = SIGM_VIDEO_CodingVC1;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC)) {
				eType = SIGM_VIDEO_CodingHEVC;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP9)) {
			    eType = SIGM_VIDEO_CodingVP9;
			}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP8)) {
				eType = SIGM_VIDEO_CodingVP8;
			}else {
				eType = SIGM_VIDEO_CodingUnused;
			}

#ifdef DEBUGFILE
			 mFile = fopen (DEBUGFILE, "wb+");
#endif

}

Error_Type_e MultipleVideoESSource::start(MetaData *params) {
		run();
		return SIGM_ErrorNone;
}

Error_Type_e MultipleVideoESSource::stop() {
	    requestExitAndWait();
		return SIGM_ErrorNone;
}

sp<MetaData> MultipleVideoESSource::getFormat() {
		return mExtractor->mTracks.itemAt(mTrackIndex).mMeta;
}

Error_Type_e MultipleVideoESSource::read(
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
				utils_log(AV_DUMP_ERROR,"ves Extractor returned EOS\n");
				return SIGM_ErrorNone;
		}
		return SIGM_ErrorNone;
}

bool MultipleVideoESSource::threadLoop()
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
	if(size > 0){
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
            listener->sendEvent(MEDIA_BUFFERING_UPDATE,0,0,(unsigned int *)&buffer);
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
	Sleep(20);
	return true;
}

int MultipleVideoESSource::requestExitAndWait() {
	int ret = Thread::requestExitAndWait();
	bEOS = false;
	return ret;
}


Error_Type_e MultipleVideoESSource::pause() {
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

Error_Type_e MultipleVideoESSource::resume() {
	Mutex::Autolock autoLock(mLock);

	mStatus &= ~PAUSED;
	mStatus |= RESUMED;

	utils_log(AV_DUMP_ERROR,"Video RESUME \n");

#ifdef DEBUGFILE
//	rewind(mFile);	
#endif
	return SIGM_ErrorNone;
}

Error_Type_e MultipleVideoESSource::seekTo(uint64_t timeMS) {	
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

MultipleVideoESExtractor::MultipleVideoESExtractor(const sp<DataSource> &source)
	: mDataSource(source),
	mEof(false),
	mCurrentOffset(0),
	eType(SIGM_VIDEO_CodingUnused),
	mFp(NULL){
	addTracks();
}

MultipleVideoESExtractor::~MultipleVideoESExtractor() {
		//close the stream
	printf("ves_close_input...\n");
	if(mFp){
		ves_close_input(mFp);
		mFp = NULL;
	}
	mCurrentOffset = 0;
}

size_t MultipleVideoESExtractor::countTracks() {
	return mTracks.size(); //only one track for video
}

sp<MediaSource> MultipleVideoESExtractor::getTrack(size_t index) {
	if (index >= mTracks.size()) {
		return NULL;
	}
	return new MultipleVideoESSource(this, index);
}

sp<MetaData> MultipleVideoESExtractor::getTrackMetaData(
				size_t index, uint32_t flags) {
	if (index >= mTracks.size()) {
		return NULL;
	}
		
	return mTracks.itemAt(index).mMeta;
}

int MultipleVideoESExtractor::addTracks() {
		Mutex::Autolock autoLock(mLock);
		String8 mStreamPath;
		Video_CodingType_e type = SIGM_VIDEO_CodingUnused;;
		int len = 0 , i = 0, index = 0;
		FILE * fp = NULL;
		char linebuf[MAX_LINE];
		char path_buffer[MAX_LINE];
		char drive[MAX_LINE];
		char dir[MAX_LINE];
		char fname[MAX_LINE];
		char ext[MAX_LINE];
		
		int ret = 0;
		int n = 0;
		
		if(mTracks.size() > 0){
			printf("why are we trying to add tracks when it is already done!!\n");
			return -1;
		}

		printf("%s %d\n",__FUNCTION__,__LINE__);
		mStreamPath = mDataSource->getUri();

		mFileNo = 0;

		if ((fp = ves_open_input(mStreamPath.string(),false)) == NULL){
			printf("Couldn't open file %s\n",mStreamPath.string());
			return -1;
		}
		else{
			while(fgets(linebuf,MAX_LINE,fp) != NULL){
 				len = strlen(linebuf);
 				linebuf[len-1] = '\0';
 				printf("%s\n",linebuf);

				if((ret = ves_find_stream_info(linebuf,(unsigned int *)&type) == 0)){
					if( SIGM_VIDEO_CodingUnused == eType ) eType = type;
					else if(eType != type) {
						printf("why there is different coding type \n");
						printf("skip this\n");
						continue;
					}

					_splitpath( mStreamPath.string(), drive, dir, fname, ext );
					
					mFileName[mFileNo]= drive;
					mFileName[mFileNo] += dir;
					mFileName[mFileNo] += "/";
					mFileName[mFileNo] += linebuf;
					mFileNo++;	
 				}
			}
		}

		ves_close_input(fp);
		fp = NULL;	
		
		srand((int)time(0)); //init the random generate

		n = get_rand(mFileNo);
		
		if(NULL == mFp){	  	
			if((mFp = ves_open_input((const char *)mFileName[n].string(),true)) == NULL)
			{
				/* Couldn't open file*/ 
				printf("open_input failed for file %s!!\n",mFileName[n].string());
				return -1;
			} 
		}
		

		sp<MetaData> meta = new MetaData;
	    switch(type){
			case SIGM_VIDEO_CodingVC1:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_WMV3);
				break;
			case SIGM_VIDEO_CodingAVC:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC);
				break;
			case SIGM_VIDEO_CodingVP8:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_VP8);
				break;
			case SIGM_VIDEO_CodingVP9:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_VP9);
				break;
			case SIGM_VIDEO_CodingHEVC:
				meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_HEVC);
				break;
			default:
				printf("unknown supported video type\n");
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



sp<MetaData> MultipleVideoESExtractor::getMetaData() {
		sp<MetaData> meta = new MetaData;
		meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_ESFILES);
		return meta;
}

//This is valid if we have multiple video tracks
void MultipleVideoESExtractor::setTrackActive(int trackIndex, bool enable) {
}

//Get Next frame of corresponding track..
MediaBuffer * MultipleVideoESExtractor::getNextEncFrame(int trackIndex, int64_t seekTime, int seekMode){
		Mutex::Autolock autoLock(mLock);
		MediaBuffer * frame = NULL;
		uint32_t size = 0;
		int n = 0;

		if(NULL == mFp){
			utils_log(AV_DUMP_ERROR,"No valid video file context FATAL!\n");
			return NULL;
		}

		if(!feof(mFp)) size = fread((void *)buf, 1, DATA_UNIT_SIZE, mFp);

		if(size > 0) {
			MediaBuffer *buffer = new MediaBuffer(size);
			memcpy((void *)buffer->data(),(void *)buf,size);
			buffer->meta_data()->setInt64(kKeyTime, -1ULL);
		
			buffer->set_range(0, size);
			mCurrentOffset += size;
			return buffer;
		}else {
			//switch file
			if(mFp){
			   utils_log(AV_DUMP_ERROR,"closing fp %p\n",mFp);
			   ves_close_input(mFp);
		       mFp = NULL;
			   mCurrentOffset = 0;
		    }
			n = get_rand(mFileNo);
		
			if(NULL == mFp){	  	
				if((mFp = ves_open_input((const char *)mFileName[n].string(),true)) == NULL)
				{
					/* Couldn't open file*/ 
					utils_log(AV_DUMP_ERROR,"open_input failed for file!!\n");
					mEof = true;
					return NULL;
				}

				if(!feof(mFp)) size = fread((void *)buf, 1, DATA_UNIT_SIZE, mFp);

				if(size > 0) {
					MediaBuffer *buffer = new MediaBuffer(size);
					memcpy((void *)buffer->data(),(void *)buf,size);
					buffer->meta_data()->setInt64(kKeyTime, -1ULL);
		
					buffer->set_range(0, size);
					mCurrentOffset += size;
					return buffer;
				}
				else {
					utils_log(AV_DUMP_ERROR,"size <=0 %p\n",mFp);
					mEof = true;
					return NULL;
				}
			}else{
				utils_log(AV_DUMP_ERROR,"why here\n");
				mEof = true;
				return NULL;
			}
		}
}

void  MultipleVideoESExtractor::seekTo(int trackIndex,int64_t time_us){
	bool bIsseekmode = false;
	Mutex::Autolock autoLock(mLock);
	
	return;
}

bool SniffMultipleVideoES(
				const sp<DataSource> &source, String8 *mimeType, float *confidence) {
		String8 streamBuf;
		int ret = 0;
		FILE * fp = NULL;
		Video_CodingType_e eType = SIGM_VIDEO_CodingUnused;
		char linebuf[MAX_LINE];
		int len = 0;
#if 0
		struct stat bufStat;
		int n;	
		DIR * dir;
    	struct dirent * ptr;
    	int i;
		int get = 0;
#endif    	
		mimeType->setTo(MEDIA_MIMETYPE_CONTAINER_ESFILES);
		
		streamBuf = source->getUri();
		printf("MultipleVES sniff %s\n",streamBuf.string());
#if 0
		if(stat(streamBuf.string(), &bufStat))  {
			printf("can't find the file %s\n",streamBuf);
			*confidence = 0;
			return false;
		}

		if(!(bufStat.st_mode|S_IFDIR)) {
			printf("The file %s isn't folder\n",streamBuf);
			*confidence = 0;
			return false;
		}

		dir = opendir(streamBuf.string());
    	while((ptr = readdir(dir)) != NULL)
    	{
        	printf("d_name : %s\n", ptr->d_name);
			for(i = 0; i< sizeof(prefix_list)/sizeof(const char *); i++) {
    			if (strncmp(prefix_list[i], ptr->d_name, strlen(prefix_list[i])) == 0 ) {
					get = 1;
					break;
    			}
			}
		}
    	closedir(dir);
	
		if (get == 0){
			printf("can't find any right files\n");
			*confidence = 0;
			return false;
		}
#endif
		if ((fp = ves_open_input(streamBuf.string(),false)) == NULL)
		{
				/* Couldn't open file*/ 
				printf("ves_open_input failed for file while sniffing ret %x!!\n",fp);
				*confidence = 0;
				return false;
		}
		else{
				*confidence = 0;
				printf("sniff pass with 0 confidence by multiple es\n");
				
				while(fgets(linebuf,MAX_LINE,fp) != NULL){
 					len = strlen(linebuf);
					if(len==0) break;
 					linebuf[len-1] = '\0';
 					printf("%s %d \n",linebuf,len - 1);

					if((ret = ves_find_stream_info(linebuf,(unsigned int *)&eType) == 0))
					{
						*confidence = 0.5f;
						printf("sniff pass with 0.5 confidence by multiple ves\n");
						break;
					}
 				}
		}

		ves_close_input(fp);
		fp = NULL;		

		if(*confidence > 0) return true;			
		return false;
}
