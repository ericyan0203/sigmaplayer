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


#include "FfmpegExtractor.h"
#include "DataSource.h"
#include "MediaBuffer.h"
#include "MediaSource.h"
#include "MetaData.h"
#include "MediaDefs.h"
#include <String8.h>
#include <string.h>
#include <Utils.h>
extern "C" {
#if 1
#include "ffmpeg/ffmpeg_rv_sc.h"
#include "ffmpeg/ffmpeg_vp68_sc.h"
#include "ffmpeg/ffmpeg_spark_sc.h"
#endif
}
#include <ctype.h>

#include "SIGM_Types.h"
#include "SIGM_Media_API.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

//#define DEBUGFILE "d://ffmpeg.es"

static bool bIsAvRegistered;

#define FUN_START
#define FUN_END
//#define HALSYS 1


//Mutex mHalSysLock; //temperary solution for thread confilict

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

void Ffmpeg_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{ 
   // ALOGW("Enter int log callback\n");
  //  LOG_PRI_VA(ANDROID_LOG_WARN,LOG_TAG,fmt,vl);
}

struct FfmpegSource : public MediaSource {
		FfmpegSource(
						const sp<FfmpegExtractor> &extractor,  size_t trackindex, size_t demuxreftrackindex);

		virtual Error_Type_e start(MetaData *params);
		virtual Error_Type_e stop();

		virtual sp<MetaData> getFormat();

		virtual Error_Type_e read(
						MediaBuffer **buffer, const ReadOptions *options);

		virtual bool threadLoop();

		virtual int  requestExitAndWait();

		private:
		enum Type {
				AVC,
				AAC,
				WMV,
				WMA,
				RV,
				RA,
				VP6,
				VP8,
				VP9,
				HEVC,
				MP1,
				MP2,
				MP3,
				AC3,
				MPEG4,
				OTHER
		};

		sp<FfmpegExtractor> mExtractor;
		size_t mTrackIndex;
		size_t mDemuxRefTrackIndex;
		Type mType;
		bool isVideo;
		sigma_handle_t mHandle;
		MediaBuffer *mBuffer;
		bool bEOS;
#ifdef DEBUGFILE
		FILE * mFile;
#endif
		//virtual ~FfmpegSource(){};
		FfmpegSource(const FfmpegSource &);
		FfmpegSource &operator=(const FfmpegSource &);
};

FfmpegSource::FfmpegSource(
				const sp<FfmpegExtractor> &extractor, size_t trackindex, size_t demuxreftrackindex)
		: mExtractor(extractor),
		mTrackIndex(trackindex),
		mDemuxRefTrackIndex(demuxreftrackindex),
		mType(OTHER),
		isVideo(true),
		mBuffer(NULL),
		bEOS(false){
				const char *mime;
				mExtractor->mTracks.itemAt(trackindex).mMeta->findCString(kKeyMIMEType, &mime);
				if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)) {
						mType = MPEG4;
						isVideo = true;
				}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
						mType = AVC;
						isVideo = true;
				}else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
						mType = AAC;
						isVideo = false;
				}else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_WMA1)) {
			            mType = WMA;
						isVideo = false;
				}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_WMV1)) {
				         mType = WMV;
						 isVideo = true;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_RV)) {
					     mType = RV;
						 isVideo = true;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RA)) {
					     mType = RA;
						 isVideo = false;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP6)) {
					     mType = VP6;
						 isVideo = true;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP8)||!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP8X)) {
					     mType = VP8;
						 isVideo = true;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP9)) { 
				         mType = VP9;
						 isVideo = true;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC)) { 
				         mType = HEVC;
						 isVideo = true;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I)) { 
				         mType = MP1;
						 isVideo = false;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II)) { 
				         mType = MP2;
						 isVideo = false;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) { 
				         mType = MP3;
						 isVideo = false;
				}else if(!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AC3)) { 
				         mType = AC3;
						 isVideo = false;
				}

#ifdef DEBUGFILE
			if(isVideo) mFile = fopen (DEBUGFILE, "wb+");
#endif

		}

Error_Type_e FfmpegSource::start(MetaData *params) {
		mHandle = (sigma_handle_t)-1;
#ifdef HALSYS
		if(mExtractor->mTracks.itemAt(mTrackIndex).mMeta->findInt32(kKeyPlatformPrivate, (int32_t *)&mHandle)){
			run();
			return SIGM_ErrorNone;
		}
		else
			return SIGM_ErrorFailed;
#else
		run();
		return SIGM_ErrorNone;
#endif
}

Error_Type_e FfmpegSource::stop() {
	    requestExitAndWait();
		return SIGM_ErrorNone;
}

sp<MetaData> FfmpegSource::getFormat() {
		return mExtractor->mTracks.itemAt(mTrackIndex).mMeta;
}

Error_Type_e FfmpegSource::read(
				MediaBuffer **out, const ReadOptions *options) {
		*out = NULL;
		int64_t seekTimeUs = -1;//default no seek
		ReadOptions::SeekMode mode;
		//set the current track as active
		mExtractor->setTrackActive(mTrackIndex,true);

		if (options && options->getSeekTo(&seekTimeUs, &mode)) {
	        printf("ffmpeg seek requested for track %d mode %d\n",mTrackIndex,mode);
			//we need to send demux ref track index
			*out = mExtractor->getNextEncFrame(mDemuxRefTrackIndex,seekTimeUs,mode);
			//*out = mExtractor->getNextEncFrame(mTrackIndex,seekTimeUs,mode);
		}else{
		    //we need to send demux ref track index
		     *out = mExtractor->getNextEncFrame(mDemuxRefTrackIndex,-1,mode); 
		     // *out = mExtractor->getNextEncFrame(mTrackIndex,-1,mode);
		 }

#if 0
		//ALOGI("(*out)->data():0x%x offset:0x%x length:%d\n",(*out)->data(),(*out)->range_offset(),(*out)->range_length());
		if(mType == AVC){
				hexdump((const uint8_t *)(*out)->data() + (*out)->range_offset(),
								(*out)->range_length());
		}
#endif
		if(NULL == *out){
				*out = new MediaBuffer(0);
				printf("Ffmpeg Extractor returned EOS\n");
				return SIGM_ErrorNone;
		}
		return SIGM_ErrorNone;
}

bool FfmpegSource::threadLoop()
{
	MediaSource::ReadOptions options;
	Error_Type_e err =  SIGM_ErrorNone;
	int64_t timeUs;
#ifdef HALSYS
	Media_Buffer_t buffer = {0};
#endif
	int32_t size = 0;
	uint32_t flags = 0;

	if( mBuffer == NULL && bEOS) {
		return false;
	} else if(mBuffer == NULL){
		err = read(&mBuffer, &options);
	}
	
	options.clearSeekTo();
	
	size = mBuffer->size();
		
    mBuffer->meta_data()->findInt64(kKeyTime, &timeUs);

	utils_log(AV_DUMP_ERROR,"%s pts %lld  size %d\n",isVideo?"Video":"Audio",timeUs,mBuffer->range_length());
#ifdef DEBUGFILE
	if(isVideo) {
		fwrite((void *)mBuffer->data(),mBuffer->range_length(),1,mFile);
		fflush(mFile);
	}
#endif
	
#ifdef HALSYS
	buffer.nAllocLen = size;
	buffer.nOffset = mBuffer->range_offset();	
	buffer.nSize = mBuffer->range_length();
	buffer.nTimeStamp = (timeUs == -1)? -1:timeUs/1000;
	buffer.nFilledLen =  mBuffer->range_length();
	buffer.pBuffer = (trid_uint8 *)mBuffer->data();
#endif	
	if( size == 0)
	{
		flags |= SIGM_BUFFERFLAG_ENDOFSTREAM;
		bEOS = true;
#ifdef DEBUGFILE
	    if(isVideo) fclose(mFile);
#endif   
	}
	if(isVideo) flags |= SIGM_BUFFERFLAG_VIDEO_BL | SIGM_BUFFERFLAG_BLOCKCALL;
	else flags |= SIGM_BUFFERFLAG_AUDIOFRAME| SIGM_BUFFERFLAG_BLOCKCALL;

#ifdef HALSYS	
	buffer.nFlags = flags;

	//mHalSysLock.lock();	
	err = HalSys_Media_PushFrame(mHandle, &buffer);
	//mHalSysLock.unlock();	
	if(err == SIGM_ErrorNone) {
		mBuffer->release();
    	mBuffer = NULL;
	}
	else{
		utils_log(AV_DUMP_ERROR,"push error ret %x\n",err);
	}
#else
	mBuffer->release();
    mBuffer = NULL;
#endif
	//else  keep the back up the data
	Sleep(1);


	return true;
}

int FfmpegSource::requestExitAndWait() {
	int ret = Thread::requestExitAndWait();
	bEOS = false;
	return ret;
}

////////////////////////////////////////////////////////////////////////////////

FfmpegExtractor::FfmpegExtractor(const sp<DataSource> &source)
		: mDataSource(source),
		mExtractedThumbnails(false),
		mEof(false),
		mCurrentOffset(0),
		mCurrentVideoTrack(-1),
		mCurrentAudioTrack(-1),
		mCurrentSubtitleTrack(-1),
		mCurrentDrmTrack(-1),
		mIsAudioCodecSpecificDataValid(false),
		mIsVideoCodecSpecificDataValid(false),
		mIsAsdTobeSent(true),
		mIsVsdTobeSent(true),
		mVideoTrackLatestPts(0),
		mVideoTrackFrameCountSinceLastPts(0),
		mAudioTrackLatestPts(0),
		mAudioTrackFrameCountSinceLastPts(0),
		mVideoFirstPts(-1),
		mVideoLastPts(-1),
	    mAudioFirstPts(-1),
		mAudioLastPts(-1),
		mFirstFrame(0),
		mVListSize(0),
		mAListSize(0),
		mVideoCodecSpecificDataSize(0),
		pFormatCtx(NULL),
		pFilter(NULL){
				addTracks();
		}

FfmpegExtractor::~FfmpegExtractor() {

		//free all the prefetch lists 
		while(encVideoFrameList.size() > 0){
				TrackEncFrame * out = *encVideoFrameList.begin();
				encVideoFrameList.erase(encVideoFrameList.begin());
				if(NULL!=out && NULL!=out->encFrame){
						out->encFrame->release();
				}
				if(out)
				{
					delete out;
				}
		}
		while(encAudioFrameList.size() > 0){
				TrackEncFrame * out = *encAudioFrameList.begin();
				encAudioFrameList.erase(encAudioFrameList.begin());
				if(NULL!=out && NULL!=out->encFrame){
						out->encFrame->release();
				}
				if(out)
				{
					delete out;
				}

		}

		//close the stream
		printf("av_close_input_file...\n");
		if(pFormatCtx){
				av_close_input_file(pFormatCtx);
				pFormatCtx = NULL;
		}

		if(pFilter){
			 av_bitstream_filter_close(pFilter);
			 pFilter = NULL;
		}
}

size_t FfmpegExtractor::countTracks() {
		return mTracks.size();
}

sp<MediaSource> FfmpegExtractor::getTrack(size_t index) {
		if (index >= mTracks.size()) {
				return NULL;
		}
		return new FfmpegSource(this, index,(size_t)mTracks.itemAt(index).mDemuxRefTracknum);
}

sp<MetaData> FfmpegExtractor::getTrackMetaData(
				size_t index, uint32_t flags) {
		if (index >= mTracks.size()) {
				return NULL;
		}
		//TODO: how do we get thumbnail times from ffmpeg??
		//or just set some random time in first 1 min
		if ((flags & kIncludeExtensiveMetaData) && !mExtractedThumbnails) {
				//findThumbnails();
				//mExtractedThumbnails = true;
		}

		return mTracks.itemAt(index).mMeta;
}
//========================H.264 codec specific data==========================
#if 1 
const uint8_t startcode[] = {0x00,0x00,0x00,0x01};
static int construct_h264_header(AVFormatContext *s,int indx, uint8_t * _data, int *length)
{
		//H264Frame *pH264Frm = &(s->H264Data);

		int dest_index = 0,
			src_index = 0;
		unsigned int Offset;
		AVCodecContext *pCodec   = s->streams[indx]->codec;
		
		if(pCodec->extradata_size <= 6)
		{
				printf("H.264 Extra Data Size %d is Invalid \n",pCodec->extradata_size);
				*length  = 0;
				return -1;
		}

		if(pCodec->extradata == NULL)
		{
				printf("No Extra Data for H.264 stream\n");
				*length  = 0;
				return -1;
		}

		memcpy(_data+dest_index,&startcode[0],4);
		dest_index += 4;

		/* First Six Bytes doesn't have SPS Info. */
		src_index  += 6;
		/* Get the SPS Size */
		Offset = 0;
		Offset  = pCodec->extradata[src_index++] << 8;
		Offset |= pCodec->extradata[src_index++];

		/* Copy the SPS */
		memcpy(_data+dest_index,&pCodec->extradata[src_index],Offset);
		src_index  += Offset;
		dest_index += Offset;

		/* Ignore the No. of PPS Byte */
		src_index++;

		/* Add the Start Code for PPS */
		memcpy(_data+dest_index,&startcode[0],4);
		dest_index += 4;

		/* Get the PPS Size */
		Offset = 0;
		Offset  = pCodec->extradata[src_index++] << 8;
		Offset |= pCodec->extradata[src_index++];

		/* Copy the PPS */
		 memcpy(_data+dest_index,&pCodec->extradata[src_index],Offset);
		src_index  += Offset;
		dest_index += Offset;

        *length  = dest_index;
		printf("H264 codec specific data constructed length %d\n",*length);

		return 0;

}
#endif
//========================H.264 codec specific data end========================================================
//================DIVX Codec Specific Data=====================================================================
const uint8_t startcode_prefix[] = {0x00, 0x00, 0x01};
const uint8_t seq_startcode   = 0x31;
const uint8_t frame_startcode = 0x32;
const uint8_t slice_startcode = 0x33;
typedef enum DIVXCode{
		DIVX_CODE_SEQHDR,
		DIVX_CODE_FRAME,
		DIVX_CODE_SLICE,
		DIVX_CODE_ENDOFSEQ
}DIVXCode;

static int construct_divx_payload_header(
				AVFormatContext *s, 
				AVStream *st,
				DIVXCode code,
				uint8_t *csp, int *csp_length, int fmlength)
{
		AVCodecContext *avctx   = st->codec;
		int            length = 0;
		uint8_t   byteoffset = 0;
		uint8_t  *payload_header = NULL;

		switch(code)
		{
			case DIVX_CODE_SEQHDR:

			memcpy(csp,startcode_prefix,3);/*copy startcode prefix */
			*(csp+3) = seq_startcode;
			payload_header = csp;
			length = 4;
			byteoffset += 4;
			// *byteoffset = 0;
			// 4+ 12 bytes are copied	
			*csp_length  = 16; 	
			break;

		case DIVX_CODE_FRAME:
			memcpy(csp,startcode_prefix,3);/*copy startcode prefix */
			*(csp+3) = frame_startcode;
			payload_header = csp;
			length = fmlength + 12;
			byteoffset += 4;
			// *byteoffset = 0;
			// 4+ 12 bytes are copied
			*csp_length  = 16;
			break;

		default:
			 printf("Invalid code %d \n",code);
			 return -1;
		}

		payload_header[byteoffset++] = (length & 0x00FF0000)>>16; /* payload length bits (23:16)*/
		payload_header[byteoffset++] = (length & 0x0000FF00)>>8; /* payload length bits (8:15)*/
		payload_header[byteoffset++] = 0x4e;  /*N*/
		payload_header[byteoffset++] = (length & 0x000000FF);/* payload length bits (0:7) 12 byte payload*/
		payload_header[byteoffset++] = 0x38; /* codec id */
		payload_header[byteoffset++] = 0x1; /* version id */
		payload_header[byteoffset++] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
		payload_header[byteoffset++] = (st->codec->width &0x00FF); /* width 0:7 */
		payload_header[byteoffset++] =  0x58; /* X*/
		payload_header[byteoffset++] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
		payload_header[byteoffset++]=  (st->codec->height &0x00FF); /* height 0:7 */
		payload_header[byteoffset++] =  0x50; /* P */
		/* End of payload header insertion */      
		return 0;
}
//================DIVX Codec Specific Data End=====================================================================

//============================= WMV codec specific data ====================================================

#ifndef AV_RB32
#   define AV_RB32(x)                           \
((((const uint8_t*)(x))[0] << 24) |         \
     (((const uint8_t*)(x))[1] << 16) |         \
     (((const uint8_t*)(x))[2] <<  8) |         \
      ((const uint8_t*)(x))[3])
#endif
static uint8_t *gWMVseqHdr[100];
static int gWMVseqhdrLength;
typedef enum VC1Code{
	VC1_CODE_RES0       = 0x00000100,
	VC1_CODE_ENDOFSEQ   = 0x0000010A,
	VC1_CODE_SLICE,
	VC1_CODE_FIELD,
	VC1_CODE_FRAME,
	VC1_CODE_ENTRYPOINT,
	VC1_CODE_SEQHDR,
}VC1Code;

/** Available Profiles */
//@{
enum Profile {
	PROFILE_SIMPLE,
	PROFILE_MAIN,
	PROFILE_COMPLEX, ///< TODO: WMV9 specific
	PROFILE_ADVANCED
};

#define IS_MARKER(x) (((x) & ~0xFF) == VC1_CODE_RES0)

const uint8_t adv_seq_startcode   = 0x0F;
const uint8_t adv_frame_startcode = 0x0D;
const uint8_t adv_entry_startcode = 0x0E;
/** Find VC-1 marker in buffer
* @return position where next marker starts or end of buffer if no marker found
*/
static av_always_inline const uint8_t* find_next_marker(const uint8_t *src, const uint8_t *end)
{
	uint32_t mrk = 0xFFFFFFFF;
	FUN_START
	if(end-src < 4) return end;
	while(src < end){
		mrk = (mrk << 8) | *src++;
		if(IS_MARKER(mrk))
		{
		    FUN_END
		    return src-4;
		}
	}
	FUN_END
	return end;
}

static av_always_inline int vc1_unescape_buffer(const uint8_t *src, int size, uint8_t *dst)
{
	int dsize = 0, i;
	FUN_START
	if(size < 4){
		for(dsize = 0; dsize < size; dsize++) *dst++ = *src++;
		FUN_END
		return size;
	}
	for(i = 0; i < size; i++, src++) {
		if(src[0] == 3 && i >= 2 && !src[-1] && !src[-2] && i < size-1 && src[1] < 4) {
		    dst[dsize++] = src[1];
		    src++;
		    i++;
		} else
		    dst[dsize++] = *src;
	}
	FUN_END
	return dsize;
}
static int construct_vc1_codec_specific_sequence_header(
       AVFormatContext *s, 
       AVStream *st,
       uint8_t *extradata,
       uint32_t extradata_size)
{
	FUN_START
	AVCodecContext *avctx = st->codec;
	VC1Frame  *pvc1Frm =  &(s->VC1Data);
	int  byteoffset =0;
	unsigned int  framerate = 0;
	unsigned long num = 0;
	pvc1Frm->seq_header_size =0;
	pvc1Frm->profile = (extradata[0] & 0xC0)>>6; /* first 2 bit of extradata is profile id */

	if(pvc1Frm->profile == PROFILE_ADVANCED){
		FUN_END
		return 0;
	}

	/*start of codec specific sequence header  */
	pvc1Frm->seq_header[byteoffset++] = 0xff; /*TODO ?? 16:23 Num of frames */
	pvc1Frm->seq_header[byteoffset++] = 0xff; /*TODO ?? 8:15  Num of frames */
	pvc1Frm->seq_header[byteoffset++] = 0xff; /*TODO ?? 0:17  Num of frames */
	pvc1Frm->version = pvc1Frm->seq_header[byteoffset++] = 0xC5;/*version id */

	/* VC1 SequnceHeader_C length 20-23 */
	pvc1Frm->seq_header[byteoffset++] = 0x4;// (extradata_size & 0xFF);
	pvc1Frm->seq_header[byteoffset++] = 0;//(extradata_size & 0xFF00)>>8; 
	pvc1Frm->seq_header[byteoffset++] = 0;//(extradata_size & 0xFF0000)>>16; 
	pvc1Frm->seq_header[byteoffset++] = 0;//(extradata_size & 0xFF000000)>>24; 

	/* start of VC1 SequnceHeader_C  4 bytes*/
	memcpy(&pvc1Frm->seq_header[byteoffset],extradata,0x4);
	if(pvc1Frm->profile == PROFILE_SIMPLE){
		pvc1Frm->seq_header[byteoffset] = (pvc1Frm->seq_header[byteoffset] & 0x0F); /* 0 for simple*/
	}
	else if (pvc1Frm->profile == PROFILE_MAIN) {
	 	pvc1Frm->seq_header[byteoffset] = ((pvc1Frm->seq_header[byteoffset] & 0x0F)| 0x40); /*4 for main */
	}
	else
	{
		printf("Invalid profile id profile %d \n",pvc1Frm->profile);
		FUN_END
		return -1;
	}

	byteoffset+=4;
	/* start of VC1 SequnceHeader_A  8 bytes*/
	memcpy(&pvc1Frm->seq_header[byteoffset],&st->codec->height,4); /*vertical size */
	byteoffset+=4;
	memcpy(&pvc1Frm->seq_header[byteoffset],&st->codec->width,4); /*horizontal size */
	byteoffset+=4;

	/*0x0000000C */
	pvc1Frm->seq_header[byteoffset++] = 0x0C; 
	pvc1Frm->seq_header[byteoffset++] = 0x00; 
	pvc1Frm->seq_header[byteoffset++] = 0x00; 
	pvc1Frm->seq_header[byteoffset++] = 0x00; 

	/* start of VC1 SequnceHeader_B  12 bytes value not knowen so filling with zero*/
	if(pvc1Frm->version ==0xC5)
	{ 
		memset(&pvc1Frm->seq_header[byteoffset],0,8);
		byteoffset+=8;

		if (st->r_frame_rate.den)
		{
		 num = (unsigned long)(st->r_frame_rate.num * 1000);
		 framerate = (int)(num/st->r_frame_rate.den);
		}
		else
		{
		 framerate = 0;
		}
		memcpy(&pvc1Frm->seq_header[byteoffset], &framerate, 4);
		byteoffset+=4;
	}
	else
	{
		memset(&pvc1Frm->seq_header[byteoffset],0,1);
		byteoffset+=1;
	}
	pvc1Frm->seq_header_size = byteoffset;
	FUN_END
	return 0;
}


static int construct_vc1_sequence_header(AVFormatContext *s, AVStream *st)
{
	FUN_START
	AVCodecContext *avctx = st->codec;
	const uint8_t *start = avctx->extradata;
	uint8_t *end = avctx->extradata + avctx->extradata_size;
	const uint8_t *next;
	int size, buf2_size;
	uint8_t *buf2 ;
	VC1Frame  *pvc1Frm =  &(s->VC1Data);
	pvc1Frm->seq_header_size =0;

	if (st->codec->codec_id  == AV_CODEC_ID_VC1){

		if(avctx->extradata_size < 16) {
		 printf("Extradata size too small: %i\n", avctx->extradata_size);
		 FUN_END
		 return -1;
		}

		buf2 =(uint8_t*) av_mallocz((unsigned int)(avctx->extradata_size +FF_INPUT_BUFFER_PADDING_SIZE));
		if(start[0]) start++; /* in WVC1 extradata first byte is its size */
		next = start;
		for(; next < end; start = next){
			  next = find_next_marker(start + 4, end);
			  size = next - start - 4;
			  if(size <= 0) continue;                             
			  switch(AV_RB32(start))
			  {                              
			     case VC1_CODE_SEQHDR:
			        buf2_size = vc1_unescape_buffer(start + 4, size, buf2);
			        memcpy(&pvc1Frm->seq_header, buf2, buf2_size) ;                              
			        pvc1Frm->seq_header_size = buf2_size;
			        pvc1Frm->profile = (buf2[0] & 0xC0)>>6; /* first 2 bit of extradata is profile id */
			        break;
			     case VC1_CODE_ENTRYPOINT:
			        buf2_size = vc1_unescape_buffer(start + 4, size, buf2);
			        memcpy(&pvc1Frm->entry_point_header, buf2, buf2_size);
			        pvc1Frm->entry_point_header_size  = buf2_size;
			        break;
			     default:
			      break;
			 }
		}
		av_free(buf2);
	}/*wmv contains only simple and main profile */
	else if (st->codec->codec_id  == AV_CODEC_ID_WMV3){
		if(construct_vc1_codec_specific_sequence_header(s,st,avctx->extradata,avctx->extradata_size)<0){
			printf(" ********************************** construct_vc1_codec_specific_sequence_header  failed \n");
			FUN_END
			return -1;
		}
	}      
	FUN_END
	return 0;
}


static int construct_vc1_payload_header(
		AVFormatContext *s, 
		AVStream *st,
		VC1Code code,
		AVPacket *pkt)
{
	FUN_START
	AVCodecContext *avctx = st->codec;
	VC1Frame  *pvc1Frm = &(s->VC1Data);
	int length =0;
	uint8_t   *payload_header = NULL;
	uint8_t   *byteoffset;

	if(code == VC1_CODE_SEQHDR)
	{
		payload_header = pvc1Frm->seq_payload_header;
		/* payload length 48 = 12 byte payload header + 36 bytes fo sequence header */
		length = pvc1Frm->seq_header_size +12 ;
		byteoffset = &pvc1Frm->seq_payload_header_size;
		*byteoffset = 0;
	 
	} 
	else if(code == VC1_CODE_FRAME) {
		payload_header = pvc1Frm->frame_payload_header;
		/* payload length = 12 byte payload header + pkt size + size of picture/frame header */
		length = pvc1Frm->frame_header_size +  pkt->size +12;
		byteoffset = &pvc1Frm->frame_payload_header_size;
		*byteoffset = 0;
	}
	else
	{
		printf( "Invalid code %d \n",code);
		FUN_END
		return -1;
	}
	pvc1Frm->profile = (avctx->extradata[0] & 0xC0)>>6; /* first 2 bit of extradata is profile id */
	if(pvc1Frm->profile == PROFILE_ADVANCED)
	{
		FUN_END
		return 0;
	}

	payload_header[(*byteoffset)++] = (length & 0x00FF0000)>>16; /* payload length bits (23:16)*/
	payload_header[(*byteoffset)++] = (length & 0x0000FF00)>>8; /* payload length bits (8:15)*/
	payload_header[(*byteoffset)++] = 0x4e;  /*N*/
	payload_header[(*byteoffset)++] = (length & 0x000000FF);/* payload length bits (0:7) 12 byte payload*/
	if(pvc1Frm->profile == PROFILE_SIMPLE){
	  payload_header[(*byteoffset)++] = 0x10; /* codec id */
	}
	else if (pvc1Frm->profile == PROFILE_MAIN) {
	  payload_header[(*byteoffset)++] = 0x11; /* codec id */
	}
	else
	{
		 printf("Invalid profile id profile %d \n",pvc1Frm->profile);
		 FUN_END
		 return -1;
	}

	payload_header[(*byteoffset)++] = 0x1; /* version id */
	payload_header[(*byteoffset)++] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
	payload_header[(*byteoffset)++] = (st->codec->width &0x00FF); /* width 0:7 */
	payload_header[(*byteoffset)++] =  0x58; /* X*/
	payload_header[(*byteoffset)++] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
	payload_header[(*byteoffset)++]=  (st->codec->height &0x00FF); /* height 0:7 */
	payload_header[(*byteoffset)++] =  0x50; /* height 0:7 */
	/* End of payload header insertion */
	FUN_END
	return 0;
}

static int construct_startcode_header(AVFormatContext *s,AVStream *st, VC1Code code)
{
	FUN_START
	VC1Frame  *pvc1Frm =  &(s->VC1Data);
	switch(code)
	{
		case VC1_CODE_SEQHDR:
		    memcpy(&pvc1Frm->seq_startcode_header[0],startcode_prefix,3);/*copy startcode prefix */
		    if(pvc1Frm->profile == PROFILE_ADVANCED)
		    {
		        pvc1Frm->seq_startcode_header[3] = adv_seq_startcode;
		    }
		    else
		    {
		        pvc1Frm->seq_startcode_header[3] = seq_startcode;
		    }
		    pvc1Frm->seq_startcode_header_size = 4;
			break;
		case VC1_CODE_FRAME:
			memcpy(&pvc1Frm->frame_startcode_header[0],startcode_prefix,3);/*copy startcode prefix */
			if(pvc1Frm->profile == PROFILE_ADVANCED)
			{
			  pvc1Frm->frame_startcode_header[3] = adv_frame_startcode;
			}
			else
			{
			  pvc1Frm->frame_startcode_header[3] = frame_startcode;
			}
			pvc1Frm->frame_startcode_header_size = 4;
			break;
		case VC1_CODE_ENTRYPOINT:
			if(pvc1Frm->profile == PROFILE_ADVANCED)
			{
			   memcpy(&pvc1Frm->entry_point_startcode_header[0],startcode_prefix,3);/*copy startcode prefix */
			   pvc1Frm->entry_point_startcode_header[3] = adv_entry_startcode;
			   pvc1Frm->entry_point_startcode_header_size = 4;
			}
			break;       
		default :
			FUN_END
			return -1;
	}
	FUN_END
	return 0;
}

//============================= WMV codec specific data END ====================================================
static int construct_codec_specific_picture_header(AVFormatContext *s,  AVStream *st,AVPacket *pkt){
	FUN_START
	VC1Frame  *pvc1Frm =  &(s->VC1Data);
	/* for advance profile payload not required */
	memset(pvc1Frm->frame_header,0,8);
	if(pvc1Frm->profile<PROFILE_ADVANCED){
		memcpy( &pvc1Frm->frame_header[0],&pkt->size,3);/*size of frame 3bytes*/
		if(pkt->flags & AV_PKT_FLAG_KEY)
		 pvc1Frm->frame_header[3]=0x80;/*bit 31 key frame 24:30 res */
	}

	if(pvc1Frm->version == 0xC5)
		pvc1Frm->frame_header_size = 8;/*4 bytes of timestamp which currently set to zero */
	else
		pvc1Frm->frame_header_size = 4;/* which currently set to zero no time stamp*/ 
	FUN_END
	return 0;
}



int64_t FfmpegExtractor::computeTrackDuration(int track){
	AVPacket encFrame  ={0};
	int Ret = 0;
	int seek_ret = 0;
	int64_t pts = -1;

	if(NULL == pFormatCtx){
		return 0;
	}
	//we try to seek to last 1MB of file and find the track frames pts/dts till we reach EOF
	//then we restore back the seek position to zero bytes 
	//Using AVSEEK_FLAG_BYTE flag
	printf("file_size:%lld bytes\n",avio_size(pFormatCtx->pb));
	int64_t seek_target = avio_size(pFormatCtx->pb)-(1024 * 1024);
	if(seek_target < 0){
	//file size is < 1MB
	seek_target = 0;
	}

	{
		Mutex::Autolock autoLock(mLock);
		seek_ret = av_seek_frame(pFormatCtx, track, seek_target, AVSEEK_FLAG_BYTE);
		if(seek_ret < 0){
		 printf("AVSEEK_FLAG_BYTE to 1MB before EOF failed\n");
		 return pts;
		}
	}

	//read the frames till we reach EOF
	while(Ret >= 0)
	{
		{
			Mutex::Autolock autoLock(mLock);
			Ret = av_read_frame(pFormatCtx, &encFrame);
		}
		if(Ret <0){
		 //EOF reached 
		 	printf("EOF reached while searching for last pts of track %d\n",track);
			break;
		}
		if(track == encFrame.stream_index){
			if(encFrame.pts != 0x8000000000000000LL){
			    pts = encFrame.pts;
			}
			else if(encFrame.dts != 0x8000000000000000LL){
			    pts = encFrame.dts;
			}
		}
	}

	//seek back to BOF
	{
		Mutex::Autolock autoLock(mLock);
		seek_ret = av_seek_frame(pFormatCtx, track, 0, AVSEEK_FLAG_BYTE);
		if(seek_ret < 0){
			printf("AVSEEK_FLAG_BYTE to BOF failed\n");
			return pts;
		}	   
	}

	return pts;

}

int FfmpegExtractor::addTracks() {
		printf("%s %d\n",__FUNCTION__,__LINE__);

		//=================================================FFMPEG======================================================
		unsigned long   uAvgDataRate = 0,uCount =0;
		int streamid = 0;
		String8 mStreamBuf;
		AVRational      *pAVFrameRate;
		pInputFileFormat = NULL;
		bool bContextAlreadyExists = false;

		if(mTracks.size() > 0){
				printf("why are we trying to add tracks when it is already done!!\n");
				return -1;
		}

		if(!mIsAvRegistered && !bIsAvRegistered){
				av_register_all();
				av_log_set_level(AV_LOG_WARNING);
				av_log_set_callback(Ffmpeg_log_callback);
				mIsAvRegistered = true;
		}
		printf("%s %d\n",__FUNCTION__,__LINE__);
		mStreamBuf = mDataSource->getUri();
		pFormatCtx = NULL;

		if(NULL == pFormatCtx){	  	
				if (avformat_open_input(&pFormatCtx, (const char *)mStreamBuf.string(),NULL, NULL) < 0)
				{
						/* Couldn't open file*/ 
						printf("av_open_input failed for file!!\n");
						return -1;
				} 
		}

		//ALOGI("%s %d\n",__FUNCTION__,__LINE__);	   

		if(!bContextAlreadyExists){
				pFormatCtx->flags |= AVFMT_FLAG_GENPTS;
				pFormatCtx->debug |= FF_FDEBUG_TS;
				/* Retrieve stream information */
				if(avformat_find_stream_info(pFormatCtx,NULL)<0)
				{
						/* Couldn't find stream information */
						printf("av_find_stream_info failed %d %s",__LINE__,__FUNCTION__);
						return -1;
				}
		}
		//ALOGI("%s %d\n",__FUNCTION__,__LINE__);	  
	 
		printf("%s %d track count:%d\n",__FUNCTION__,__LINE__,pFormatCtx->nb_streams);

		/* Add tracks */
		for(uCount=0; uCount<pFormatCtx->nb_streams; uCount++)
		{
				sp<MetaData> meta = new MetaData;
				printf("trackindx:%d extradata size:%d\n",uCount,pFormatCtx->streams[uCount]->codec->extradata_size);
				if(pFormatCtx->streams[uCount]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
				{
                        bool bVideoCodecSupported = false;
						int  num,framerate;
						
						if(pFormatCtx->streams[uCount]->r_frame_rate.den)
						{
						    num = (unsigned long)(pFormatCtx->streams[uCount]->r_frame_rate.num*1000);
							framerate = (int)(num/pFormatCtx->streams[uCount]->r_frame_rate.den);
						}
						else
						{
						    framerate = 0;
						}

					    meta->setInt32(kKeyFrameRate, framerate);
		                printf("Ffmpeg set frame rate %x\n",framerate); 
						
						switch(pFormatCtx->streams[uCount]->codec->codec_id)
						{
								case	AV_CODEC_ID_MPEG2VIDEO: 
								case	AV_CODEC_ID_MPEG1VIDEO: 
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MPEG2);
										//meta->setData(kKeyAVCC, 0, codecPrivate, codecPrivateSize);
										//*pVideoFormat = CNXT_VIDEO_FORMAT_MPEG2;
										printf("FFMPEG VIDEO MPEG1/2\n");
										printf("codecPrivateSize video= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										bVideoCodecSupported = true;
										break;

								case   AV_CODEC_ID_MSMPEG4V2:
								case   AV_CODEC_ID_MSMPEG4V1:
								case   AV_CODEC_ID_MSMPEG4V3:
								case   AV_CODEC_ID_MPEG4:
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MPEG4);

										//meta->setData(kKeyAVCC, 0, codecPrivate, codecPrivateSize);
										//*pVideoFormat = CNXT_VIDEO_FORMAT_DIVX;
										printf("FFMPEG VIDEO MSMPEG4/V1/2/3\n");
										printf("codecPrivateSize video= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										
										if(pFormatCtx->streams[uCount]->codec->extradata_invalid == 0 && 
											pFormatCtx->streams[uCount]->codec->extradata_size > 0)
										{
											mVideoCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;
											if(mVideoCodecSpecificDataSize > 0){
												memcpy(&mVideoCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsVideoCodecSpecificDataValid = true;
											}
										}
										
										bVideoCodecSupported = true;

										break;

								case	AV_CODEC_ID_H263:			  
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_H263);
										//meta->setData(kKeyAVCC, 0, codecPrivate, codecPrivateSize);			  	
										//*pVideoFormat = CNXT_VIDEO_FORMAT_MPEG4PART2;
										break;

								case	AV_CODEC_ID_H264 :
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_AVC);
										//meta->setData(kKeyAVCC, kTypeAVCC, pFormatCtx->streams[uCount]->codec->extradata, 
										//	            pFormatCtx->streams[uCount]->codec->extradata_size); 			  
										printf("FFMPEG VIDEO H264 size %x invalid %x\n",pFormatCtx->streams[uCount]->codec->extradata_size,pFormatCtx->streams[uCount]->codec->extradata_invalid);
//avi extra data is useless. asf/wmv extra data is sps+pps.  mkv/mp4 is sps/pps
										if(pFormatCtx->streams[uCount]->codec->extradata_invalid == 0 && 
											pFormatCtx->streams[uCount]->codec->extradata_size >0 &&
											strncasecmp(pFormatCtx->iformat->long_name,"raw H.264 video",15))
											mIsVideoCodecSpecificDataValid = true;

										printf("codecPrivateSize video= %d\n",mVideoCodecSpecificDataSize);
										bVideoCodecSupported = true;
										break;
#if 0
								case	AV_CODEC_ID_XVID:
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_MPEG4);
										//meta->setData(kKeyAVCC, kTypeAVCC, pFormatCtx->streams[uCount]->codec->extradata, 
										//				pFormatCtx->streams[uCount]->codec->extradata_size);			  
										//*pVideoFormat = CNXT_VIDEO_FORMAT_MPEG4;
										//construct_h264_header(pFormatCtx,pFormatCtx->streams[uCount]->codec);
										ALOGI("FFMPEG VIDEO XVID\n");
										ALOGI("codecPrivateSize video= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										bVideoCodecSupported = true;
										mVideoCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;
										if(mVideoCodecSpecificDataSize > 0){
												memcpy(&mVideoCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsVideoCodecSpecificDataValid = true;
										}

#endif												break;

								case	AV_CODEC_ID_GIF :
										//*pVideoFormat = CNXT_VIDEO_FORMAT_GIF;
										break;

								case	AV_CODEC_ID_PNG:
										//*pVideoFormat = CNXT_VIDEO_FORMAT_PNG;
										break;

								case	AV_CODEC_ID_WMV2:
								case	AV_CODEC_ID_WMV1:
								case	AV_CODEC_ID_WMV3:
								case	AV_CODEC_ID_VC1:
 		                                printf("FFMPEG VIDEO CODEC_ID_WMV\n");
									    pFormatCtx->VC1Data.firstFrame  = 1;
									    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_WMV1);		
									    /* Indicate that we need to insert startcode headers */
									    printf("codecPrivateSize video= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
									    bVideoCodecSupported = true;
									    mIsVideoCodecSpecificDataValid  = false;

										break;

								case    AV_CODEC_ID_RV10:
								case    AV_CODEC_ID_RV20:
								case    AV_CODEC_ID_SVQ1: //Sorenson Vector Quantizer 1
								case    AV_CODEC_ID_SVQ3: //Sorenson Vector Quantizer 3	
								case    AV_CODEC_ID_VP3:
								case    AV_CODEC_ID_VP5:
									    break;
										
								case    AV_CODEC_ID_RV30:
								case    AV_CODEC_ID_RV40:
									    printf("FFMPEG VIDEO CODEC_ID_RV\n");
									    mFirstFrame  = 1;
									    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_RV);		
									    /* Indicate that we need to insert startcode headers */
									    printf("codecPrivateSize video= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
									    bVideoCodecSupported = true;
									    mIsVideoCodecSpecificDataValid  = false;
										break;

								case    AV_CODEC_ID_FLV1:
									    printf("FFMPEG VIDEO CODEC_ID_H253\n");
									    mFirstFrame  = 1;
									    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_H263);
									    bVideoCodecSupported = true;
									    mIsVideoCodecSpecificDataValid  = false;
									    break;
								case    AV_CODEC_ID_VP6:
							    case    AV_CODEC_ID_VP6A: //I don't know in which case we will meet this case
								case    AV_CODEC_ID_VP6F:	 
								        printf("FFMPEG VIDEO CODEC_ID_VP6\n");
									    mFirstFrame  = 1;
									    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_VP6);		
									    /* Indicate that we need to insert startcode headers */
									    printf("codecPrivateSize video= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
									    bVideoCodecSupported = true;
									    mIsVideoCodecSpecificDataValid  = false;	
								break;
								case    AV_CODEC_ID_VP8:
								        printf("FFMPEG VIDEO CODEC_ID_VP8\n");
									    mFirstFrame  = 1;
									    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_VIDEO_VP8X);		
									    /* Indicate that we need to insert startcode headers */
									    printf("codecPrivateSize video= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
									    bVideoCodecSupported = true;
									    mIsVideoCodecSpecificDataValid  = false;	
										break;
								case    AV_CODEC_ID_MJPEG:
								case    AV_CODEC_ID_MJPEGB:
								default :
										printf("Unknown Video Format %s %d\n",__FILE__,__LINE__);
										break;

						}
						if(!bVideoCodecSupported){
								continue;
						}
						meta->setInt32(kKeyWidth, pFormatCtx->streams[uCount]->codec->width);
						meta->setInt32(kKeyHeight,pFormatCtx->streams[uCount]->codec->height);
						if(-1 == mCurrentVideoTrack){
								mCurrentVideoTrack = uCount;//Default to first video track
								printf("mCurrentVideoTrack:%d\n",mCurrentVideoTrack);
						}
				}
				else if(pFormatCtx->streams[uCount]->codec->codec_type == AVMEDIA_TYPE_AUDIO){   
						bool bAudioCodecSupported = false;
						uint8_t       *pEncodeOption = NULL;
						unsigned short       uEncoderOptions;
						unsigned long uChannelMask =0;
                        
						printf("Found Audio Format %s %d  codecid:: %d \n",__FILE__,__LINE__,(int)pFormatCtx->streams[uCount]->codec->codec_id);
						switch(pFormatCtx->streams[uCount]->codec->codec_id)
						{         
								case  	AV_CODEC_ID_MP1:
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_I);
										bAudioCodecSupported = true;
										printf("FFMPEG AUDIO MP1\n");
										printf("codecPrivateSize audio= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;				   
										if(mAudioCodecSpecificDataSize > 0){
												memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsAudioCodecSpecificDataValid = true;
										}
										break;
								case    AV_CODEC_ID_MP2:
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_MPEG_LAYER_II);
										bAudioCodecSupported = true;
										printf("FFMPEG AUDIO MP2\n");
										printf("codecPrivateSize audio= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;				   
										if(mAudioCodecSpecificDataSize > 0){
												memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsAudioCodecSpecificDataValid = true;
										}
										break;
								case    AV_CODEC_ID_MP3:    
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_MPEG);
										bAudioCodecSupported = true;
										printf("FFMPEG AUDIO MP3\n");
										printf("codecPrivateSize audio= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;				   
										if(mAudioCodecSpecificDataSize > 0){
												memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsAudioCodecSpecificDataValid = true;
										}
										break;

								case    AV_CODEC_ID_AC3:
									    meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AC3);
										bAudioCodecSupported = true;
										printf("FFMPEG AUDIO AC3\n");
										printf("codecPrivateSize audio= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;
										if(mAudioCodecSpecificDataSize > 0){
												memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsAudioCodecSpecificDataValid = true;
										}

										break;
#if 0
								case    AV_CODEC_ID_AAC :
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_AAC);
										bAudioCodecSupported = true;
										printf("FFMPEG AUDIO AAC\n");
										printf("codecPrivateSize audio= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;
										if(mAudioCodecSpecificDataSize > 0){
												memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsAudioCodecSpecificDataValid = true;
										}

										//*pAudioFormat  = CNXT_AUDIO_FMT_AAC;
										break;   
								case    AV_CODEC_ID_WMAV1:
								case    AV_CODEC_ID_WMAV2:   
								        //*pAudioFormat  = CNXT_AUDIO_FMT_WMA;
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_WMA1);
										bAudioCodecSupported = true;
										mIsAudioCodecSpecificDataValid = false;
										printf("FFMPEG AUDIO WMAWMA 11111111111111111111111111111111111111111 \n");
										printf("codecPrivateSize audio= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
										//   LOGI("FFMPEG AUDIO WMAWMA    FRMAES ::  %d    \n",pFormatCtx->nb_packets);
										mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;
										/*if(mAudioCodecSpecificDataSize > 0){
										memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
										mIsAudioCodecSpecificDataValid = true;
										}  */
										/*eric
										if(mAudioCodecSpecificDataSize > 0){
										    meta->setData(kKeyWmaCodecData, 0,pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size );
										 }
										*/
										if(AV_CODEC_ID_WMAV1 == pFormatCtx->streams[uCount]->codec->codec_id)
										{
												meta->setInt32(kKeyWMAVersion, 1);				   			
										}
										else
										{
												meta->setInt32(kKeyWMAVersion, 2);				   			
										}
										meta->setInt32(kKeyBitRate,pFormatCtx->streams[uCount]->codec->bit_rate);
										meta->setInt32(kKeyBlockAllign,pFormatCtx->streams[uCount]->codec->block_align);
										meta->setInt32(kKeyPcktSize,pFormatCtx->packet_size);
										meta->setInt32(kKeyNumPackets,pFormatCtx->nb_packets);
										meta->setInt32(kKeyValidbps,pFormatCtx->streams[uCount]->codec->bits_per_coded_sample);

										switch(pFormatCtx->streams[uCount]->codec->channels)
                                        {
                                            case 1: 
												uChannelMask = 0x4;
												break;
											case 2:
												uChannelMask = 0x3;
											    break;
											case 6:
												uChannelMask = 0x3f;
												break;
											default:
												printf("error channel =%d\n",pFormatCtx->streams[uCount]->codec->channels);
												break;


										}
										meta->setInt32(kKeyChnlMask,(int)uChannelMask);

										if(mAudioCodecSpecificDataSize > 8){	
											    int i = 0;
											    unsigned short uValidBitsPerSample = 0, uEncoderOptions = 0 ;
												pEncodeOption =  pFormatCtx->streams[uCount]->codec->extradata + 4;
												memcpy((void *)&uEncoderOptions, (void*)pEncodeOption, sizeof(unsigned short)); 	
											    
												meta->setInt32(kKeyEncOpt,(int)uEncoderOptions);							
										}else{
												meta->setInt32(kKeyEncOpt,0);											
										}
										break;
										case AV_CODEC_ID_WMAPRO:
										// need to uncomment below code for supporting wmapro after audio team completes full support
										//*pAudioFormat  = CNXT_AUDIO_FMT_WMA_PRO;
											meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_WMA3);
											bAudioCodecSupported = true;
											mIsAudioCodecSpecificDataValid = false;
											printf("FFMPEG AUDIO WMAPRO WMAPRO 11111111111111111111111111111111111111111 \n");
											printf("codecPrivateSize audio= %d\n",pFormatCtx->streams[uCount]->codec->extradata_size);
											mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;
											if(mAudioCodecSpecificDataSize > 0){
												memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												mIsAudioCodecSpecificDataValid = true;
											} 
											meta->setInt32(kKeyWMAVersion, 3);
											meta->setInt32(kKeyBitRate,pFormatCtx->streams[uCount]->codec->bit_rate);
											meta->setInt32(kKeyBlockAllign,pFormatCtx->streams[uCount]->codec->block_align);
											meta->setInt32(kKeyPcktSize,pFormatCtx->packet_size);
											meta->setInt32(kKeyNumPackets,pFormatCtx->nb_packets);				
											if(mAudioCodecSpecificDataSize >=18){
												unsigned short uValidBitsPerSample = 0, uEncoderOptions = 0, uAdvEncOption =0,uFormatTag = 0 ;
												unsigned long uChannelMask =0, uAdvEncOption2 =0;
												unsigned char *codecPrivateData = pFormatCtx->streams[uCount]->codec->extradata;

									        	printf("FOUND extradata in wma pro \n");

												uValidBitsPerSample = *(unsigned short *)(codecPrivateData);
												uChannelMask        = *(unsigned long *)(codecPrivateData + 2);
												uAdvEncOption2      = *(unsigned long *)(codecPrivateData + 10);
												uEncoderOptions   = *(unsigned short *)(codecPrivateData + 14);
												uAdvEncOption      = *(unsigned short *)(codecPrivateData + 16);
												uFormatTag         = pFormatCtx->streams[uCount]->codec->codec_tag;

												meta->setInt32(kKeyValidbps,(int)uValidBitsPerSample);
												meta->setInt32(kKeyChnlMask,(int)uChannelMask);
												meta->setInt32(kKeyAdvEncOp2,(int)uAdvEncOption2);
												meta->setInt32(kKeyEncOpt,(int)uEncoderOptions);												
												meta->setInt32(kKeyAdvEncOp,(int)uAdvEncOption);
												meta->setInt32(kKeyFmtTag,(int)uFormatTag);	
												printf("Trid:FFMPEG:%s,  		uSampleRate =%d	\n",__FUNCTION__, pFormatCtx->streams[uCount]->codec->sample_rate);
												printf("Trid:FFMPEG:%s,  	uStreamRate =%d,   uChannelCount=%d	\n",__FUNCTION__,pFormatCtx->streams[uCount]->codec->bit_rate, (int) pFormatCtx->streams[uCount]->codec->channels);
												printf("Trid:FFMPEG:%s,  	uPacketSize =%d, 	uBlockAlignment =%d 	\n",__FUNCTION__,(int)pFormatCtx->packet_size,(int) pFormatCtx->streams[uCount]->codec->block_align);
												printf("Trid:FFMPEG:%s,  	uNumOfPackets =%d , uValidBitsPerSample=%d	\n",__FUNCTION__,(int)pFormatCtx->nb_packets ,(int)uValidBitsPerSample);
												printf("Trid:FFMPEG:%s,  	uChannelMask =%d, 	uAdvEncOpt =%d 	\n",__FUNCTION__,(int)uChannelMask,(int) uAdvEncOption);
												printf("Trid:FFMPEG:%s,  	uAdvEncOpt2 =%d , uFormatTag=%d	\n",__FUNCTION__,(int)uAdvEncOption2,(int)uFormatTag); 
												mAudioCodecSpecificDataSize = 0; //Naresh @ for testing
											}else{
												printf(" \n @@@@@@@@@@@@@@@@@@  ERROR ERROR ERROR no extradata in wma pro  @@@@@@@@@@ \n");
												bAudioCodecSupported = false;
												mIsAudioCodecSpecificDataValid = false;	
											}
										 
										break;



						//case    AV_CODEC_ID_DTS:

								//*pAudioFormat  = CNXT_AUDIO_FMT_DTS;

						//		break;

								case    AV_CODEC_ID_MLP:
										//*pAudioFormat  = CNXT_AUDIO_FMT_MLP;
										break;

								case    AV_CODEC_ID_COOK:
									    bAudioCodecSupported = true;
										mIsAudioCodecSpecificDataValid = false;
										meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_RA);
										mAudioCodecSpecificDataSize = pFormatCtx->streams[uCount]->codec->extradata_size;
										printf("RA Specific data=%d flavor=%d Block Align=%d\n",mAudioCodecSpecificDataSize,pFormatCtx->streams[uCount]->codec->flavor,pFormatCtx->streams[uCount]->codec->block_align);
                                        meta->setInt32(kKeyflavor,(int)pFormatCtx->streams[uCount]->codec->flavor);	
										if(mAudioCodecSpecificDataSize > 0){
												//memcpy(&mAudioCodecSpecificData[0],pFormatCtx->streams[uCount]->codec->extradata,pFormatCtx->streams[uCount]->codec->extradata_size);
												//mIsAudioCodecSpecificDataValid = true;
										}												
										//*pAudioFormat = CNXT_AUDIO_FMT_REALAUDIO;
										break;
#endif
								default :
										printf("Unsupported audio Format %s %d codecid %x\n",__FILE__,__LINE__,pFormatCtx->streams[uCount]->codec->codec_id);
										break;         
						}   
						if(!bAudioCodecSupported){
								continue;
						}

						meta->setInt32(kKeySampleRate,pFormatCtx->streams[uCount]->codec->sample_rate);
						meta->setInt32(kKeyChannelCount, pFormatCtx->streams[uCount]->codec->channels);
						if(-1 == mCurrentAudioTrack){
								mCurrentAudioTrack = uCount; //Default to first audio track
								printf("mCurrentAudioTrack:%d\n",mCurrentAudioTrack);
						}
				}
				else {
						//For now ignore subtitle and other tracks
						printf("Track %d ignored\n",uCount);
						continue;
				}

				int64_t duration;
				int     num, den;
				if(pFormatCtx->streams[uCount]->duration != 0x8000000000000000LL){
						duration = pFormatCtx->streams[uCount]->duration; 
				}
				else{
				      //compute the track duration the hard way-pts way 
					  printf("Track %d duration is unknown trying for pts diff\n",uCount);

                      duration =  computeTrackDuration(uCount);

					  if(duration < 0){
					      duration = 0;
					  }
				}
				num = pFormatCtx->streams[uCount]->time_base.num;
				den = pFormatCtx->streams[uCount]->time_base.den;
				duration = (int64_t)(((double)duration *((double)num/(double)den)) * (double)(1000000));        
				meta->setInt64(kKeyDuration, duration);

				mTracks.push();
				TrackInfo *trackInfo = &mTracks.editItemAt(mTracks.size() - 1);
				trackInfo->mTrackNum = pFormatCtx->streams[uCount]->index;
				trackInfo->mDemuxRefTracknum = uCount;
				trackInfo->mMeta = meta;
				printf("Track %d successfully added\n",uCount);
		} /* for(uCount=0; uCount<pFormatCtx->nb_streams; uCount++) */	 
		printf("%s %d context:0x%x\n",__FUNCTION__,__LINE__,pFormatCtx);
		//========================================================================================
		return 0;
}

void FfmpegExtractor::findThumbnails() {
		for (size_t i = 0; i < mTracks.size(); ++i) {
				TrackInfo *info = &mTracks.editItemAt(i);

				const char *mime;
				info->mMeta->findCString(kKeyMIMEType, &mime);

				if (strncasecmp(mime, "video/", 6)) {
						continue;
				}		
				info->mMeta->setInt64(kKeyThumbnailTime, 30000);//30 secs
		}
}

sp<MetaData> FfmpegExtractor::getMetaData() {
		sp<MetaData> meta = new MetaData;
		meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_WMV);
		return meta;
}
//This is valid if we have multiple video/audio/subtitle tracks
void FfmpegExtractor::setTrackActive(int trackIndex, bool enable) {

		//find out the track type? A/V/Sub  

		//Set it to mCurrentXXXXXTrack 
		//Acquire a lock and flush the encFrameList for previous index and start
		//afresh for new index

}
//Get Next frame of corresponding track..
MediaBuffer * FfmpegExtractor::getNextEncFrame(int trackIndex, int64_t seekTime, int seekMode){
		AVPacket		 encFrame  ={0};
		MediaBuffer * frame = NULL;
		bool bFrameAvailable = false,bIsseekmode = false;
		int Ret = 0;
		int entrypointSize   = 0;
		Mutex::Autolock autoLock(mLock);

		const int VIDEO_MAXQ_SIZE=10*1024*1024; //Do not allow caching of more than this size
		const int AUDIO_MAXQ_SIZE=10*1024*1024;

		if(NULL == pFormatCtx){
				printf("No valid AV context FATAL!\n");
				return NULL;
		}

		//flush all tracks prefetched lists if seek requested for any
		// one track..this may cause multiple ffmpeg seeks..
		if(seekTime >= 0){ 
				printf("ffmpeg Seek mode\n");
				bIsseekmode = true;
				//do the ffmpeg seek
				int64_t seek_target = seekTime;
				int seek_flags = -1;
				switch(seekMode){
						case   0://SEEK_PREVIOUS_SYNC:
						case   1://SEEK_NEXT_SYNC:
						case   2://SEEK_CLOSEST_SYNC:
						case   3://SEEK_CLOSEST:
								seek_flags = 0;
								break;

						default :
								printf("Invalid Seek mode\n");
								break;

				}
				if(trackIndex >=0){
						/*seek_target= av_rescale_q(seek_target, AV_TIME_BASE_Q,
						  pFormatCtx->streams[trackIndex]->time_base); */
						//For wmv, vc1
						seek_target = (seekTime/(double)1000000) *(double) pFormatCtx->streams[trackIndex]->time_base.den/(double)pFormatCtx->streams[trackIndex]->time_base.num;
				}

				int seek_ret = 0;
				{
					//	Mutex::Autolock autoLock(mLock);
						seek_ret = av_seek_frame(pFormatCtx, trackIndex, seek_target, AVSEEK_FLAG_ANY);
				}
				if(seek_ret < 0) {
						printf("ffmpeg error while seeking track %d\n",trackIndex);
				}
				else{ //ffmpeg seek is successful flush prefetched pkts
						printf("seek successful\n");
						while(encVideoFrameList.size() > 0){
								TrackEncFrame * out = *encVideoFrameList.begin();
								encVideoFrameList.erase(encVideoFrameList.begin());
								if(NULL!=out && NULL!=out->encFrame){

										mVListSize-=out->encFrame->size();
										//ALOGE("vlist size at pop (%d)\n", mVListSize);

										out->encFrame->release();
								}
								if(out)
								{
									delete out;
								}

						}
						while(encAudioFrameList.size() > 0){
							    printf("audio frame size %x\n",encAudioFrameList.size());
								TrackEncFrame * out = *encAudioFrameList.begin();
								encAudioFrameList.erase(encAudioFrameList.begin());
								if(NULL!=out && NULL!=out->encFrame){

										mAListSize-=out->encFrame->size();
										//ALOGE("alist size at pop (%d)\n", mAListSize);

										out->encFrame->release();
								}
								if(out)
								{
									delete out;
								}

						}

				}

		}
	//	ALOGW(" TRACK Index :: %d  %d\n",(int) trackIndex , (int) mCurrentVideoTrack);

		//try to get the frames from prefetched lists
		if(trackIndex == mCurrentVideoTrack){
				//Lock the video list while retrieving data
				Mutex::Autolock autoLock(mVListLock);
				if(encVideoFrameList.size() <= 0){
						if(mIsVsdTobeSent && mIsVideoCodecSpecificDataValid){
								if((AV_CODEC_ID_H264 != pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id )/* ||
									!strncasecmp(pFormatCtx->iformat->long_name,"raw H.264 video",15)*/) {
									MediaBuffer *buffer = new MediaBuffer(mVideoCodecSpecificDataSize);
								
									if(mVideoCodecSpecificDataSize < 400){
											memcpy((uint8_t *)buffer->data(),mVideoCodecSpecificData,mVideoCodecSpecificDataSize);	 
											buffer->set_range(0, mVideoCodecSpecificDataSize);
									}else {
										memcpy((uint8_t *)buffer->data(),mVideoCodecSpecificData,400);	
										buffer->set_range(0, 400);
									}
									mIsVsdTobeSent = false;
									//set the frame time to 0
									buffer->meta_data()->setInt64(kKeyTime, 0);

									return buffer;
								}else {
								//	Mutex::Autolock autoLock(mLock);
									pFilter = av_bitstream_filter_init("h264_mp4toannexb");
									printf("Init h264_mp4toannexb filter\n");
								}
								mIsVsdTobeSent = false;
						}
						//printf("Video track prefetch list is empty\n");
				}else{
						TrackEncFrame * out = *encVideoFrameList.begin();
						encVideoFrameList.erase(encVideoFrameList.begin());	   

						if(NULL != out){
								frame = out->encFrame;
								bFrameAvailable = true;
								delete out;
								mVListSize-=frame->size();
						}
				}
		}else if(trackIndex == mCurrentAudioTrack){
				//Lock the audio list while retrieving data
				Mutex::Autolock autoLock(mAListLock);

				if(encAudioFrameList.size() <= 0){
						if(mIsAsdTobeSent && mIsAudioCodecSpecificDataValid){
								MediaBuffer *buffer = new MediaBuffer(mAudioCodecSpecificDataSize);
								memcpy((uint8_t *)buffer->data(),mAudioCodecSpecificData,mAudioCodecSpecificDataSize);	 
								buffer->set_range(0, mAudioCodecSpecificDataSize);
								mIsAsdTobeSent = false;
								//set the frame time to 0
								buffer->meta_data()->setInt64(kKeyTime, 0);
								printf("specific data\n");
								return buffer;		 
						}
						//printf("Audio track prefetch list is empty\n");
				}else{
						TrackEncFrame * out = *encAudioFrameList.begin();
						encAudioFrameList.erase(encAudioFrameList.begin());
						if(NULL != out)
						{
								frame = out->encFrame;
								bFrameAvailable = true;
								delete out;
								mAListSize-=frame->size();
						}
				}
		}else {
				printf("why are we reading something which is not A/V?\n");
				//TMOD@ not a video Or audio index should not call  read frame
				return NULL;
		}
		while(!bFrameAvailable && !mEof){	
				/* Read frame */
				{
					//	Mutex::Autolock autoLock(mLock);
						Ret = av_read_frame(pFormatCtx, &encFrame);
				//		ALOGI("after av_read_frame\n");
						if(Ret < 0){
								mEof = true;
								printf("EOF reached %s %d %x\n",__FILE__,__LINE__,Ret);	 
								return NULL;
						}

						if(mCurrentVideoTrack != encFrame.stream_index && mCurrentAudioTrack != encFrame.stream_index)
						{
						    printf("ignore the id %x \n",encFrame.stream_index);
						    av_free_packet(&encFrame);
							continue;
						}
						
				}
	
				//Add the frame to track encoded frame list	
				MediaBuffer *buffer = new MediaBuffer(encFrame.size+8192); //add 8192 bytes 

				if(mCurrentVideoTrack == encFrame.stream_index) printf("videotrack pts %lld size %d\n",encFrame.pts,encFrame.size);
				else if(mCurrentAudioTrack == encFrame.stream_index) printf("audiotrack %lld size %d\n",encFrame.pts,encFrame.size);
				//try for pts
				if((encFrame.pts != 0x8000000000000000LL) && (encFrame.flags & AV_PKT_FLAG_KEY)){
						int64_t pts;
						int     num, den;
						pts = encFrame.pts;
						num = pFormatCtx->streams[encFrame.stream_index]->time_base.num;
						den = pFormatCtx->streams[encFrame.stream_index]->time_base.den;
						pts = (int64_t)(((double)pts *((double)num/(double)den)) * (double)(1000000));

						buffer->meta_data()->setInt64(kKeyTime, pts);

						if(mCurrentVideoTrack == encFrame.stream_index) {
								mVideoTrackLatestPts = pts;
								mVideoTrackFrameCountSinceLastPts = 0;	
								if(-1 == mVideoFirstPts){
								    mVideoFirstPts = pts;
								}    
							//	ALOGI("Sync video pts :%lld\n",pts);
						}else if(mCurrentAudioTrack == encFrame.stream_index){
								mAudioTrackLatestPts = pts;
								mAudioTrackFrameCountSinceLastPts = 0;
								 if(-1 == mAudioFirstPts){
								     mAudioFirstPts = pts;
								 }
								//ALOGI("Sync audio pts :%lld\n",pts);
						}
				}
				else if(encFrame.dts != 0x8000000000000000LL){
						//try dts
						int64_t pts;
						int 	num, den;
						pts = encFrame.dts;
						num = pFormatCtx->streams[encFrame.stream_index]->time_base.num;
						den = pFormatCtx->streams[encFrame.stream_index]->time_base.den;
						pts = (int64_t)(((double)pts *((double)num/(double)den)) * (double)(1000000));
						//ALOGI("dts :%lld\n",pts);

						buffer->meta_data()->setInt64(kKeyTime, pts);


						if(mCurrentVideoTrack == encFrame.stream_index) {	
								mVideoTrackFrameCountSinceLastPts++;
								if(-1 == mVideoFirstPts){
								    mVideoFirstPts = pts;
								}
						}else if(mCurrentAudioTrack == encFrame.stream_index){
								mAudioTrackFrameCountSinceLastPts++;
								 if(-1 == mAudioFirstPts){
								     mAudioFirstPts = pts;
								 }   

						}
				}
				else{
						//no pts and no dts try interpolating from last available pts
						//assume file is not proper if last pts does not exist
						int64_t pts;
						int num, den;
						if(mCurrentVideoTrack == encFrame.stream_index) {
								//printf("num %x den %x last %lld\n",pFormatCtx->streams[encFrame.stream_index]->time_base.num,pFormatCtx->streams[encFrame.stream_index]->time_base.den, mVideoTrackFrameCountSinceLastPts);
								pts = mVideoTrackFrameCountSinceLastPts+1;
								num = pFormatCtx->streams[encFrame.stream_index]->time_base.num;
								den = pFormatCtx->streams[encFrame.stream_index]->time_base.den;
								//seems not right in es playback
								pts = -1;//(int64_t)(((double)pts *((double)num/(double)den)) * (double)(1000000));
								buffer->meta_data()->setInt64(kKeyTime,mVideoTrackLatestPts+pts);

								mVideoTrackFrameCountSinceLastPts++;
						}else if(mCurrentAudioTrack == encFrame.stream_index){
								pts = mAudioTrackFrameCountSinceLastPts+1;
								num = pFormatCtx->streams[encFrame.stream_index]->time_base.num;
								den = pFormatCtx->streams[encFrame.stream_index]->time_base.den;
								//Fix wrong PTS calculate, we just set it as 0
								pts = -1;//(int64_t)(((double)pts *((double)num/(double)den)) * (double)(1000000));
								buffer->meta_data()->setInt64(kKeyTime,mAudioTrackLatestPts+pts);
								mAudioTrackFrameCountSinceLastPts++;
						}

				}
				if(encFrame.flags & AV_PKT_FLAG_KEY){
						buffer->meta_data()->setInt32(kKeyIsSyncFrame, 1);
				}
				buffer->set_range(0, encFrame.size);
				if (mCurrentVideoTrack == encFrame.stream_index) {
						uint8_t *data = (uint8_t *)buffer->data();
						if(AV_CODEC_ID_H264 == pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ){
							if(pFilter != NULL){
								AVPacket fpkt = encFrame;
							//  Mutex::Autolock autoLock(mLock);
                				int a = av_bitstream_filter_filter(pFilter,pFormatCtx->streams[mCurrentVideoTrack]->codec,
											NULL, &fpkt.data, &fpkt.size,encFrame.data, encFrame.size, encFrame.flags & AV_PKT_FLAG_KEY);
                				encFrame.data = fpkt.data;
                				encFrame.size = fpkt.size;
							}
							memcpy(data,encFrame.data,encFrame.size);
							buffer->set_range(0, encFrame.size);
						}
						#if 1
						else if((AV_CODEC_ID_MSMPEG4V3 == pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id) ||
										(AV_CODEC_ID_MSMPEG4V2 == pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id)||
										(AV_CODEC_ID_MSMPEG4V1 == pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id)/*||
										(AV_CODEC_ID_XVID == pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id)*/){
							 int divx_payload_length = 0;
#if 0
							if (AV_CODEC_ID_MSMPEG4V3 != pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id) {
								construct_divx_payload_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],DIVX_CODE_FRAME,data,&divx_payload_length,encFrame.size);	
							}
#else
							 construct_divx_payload_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],DIVX_CODE_FRAME,data,&divx_payload_length,encFrame.size);
#endif
							 memcpy(data+divx_payload_length ,encFrame.data,encFrame.size);        
							 buffer->set_range(0, divx_payload_length +encFrame.size);
                             printf("DIVX 3/4/5/6 frame detected\n");
						}
                        /* For video packet we need to add vc1 header at the begining of the buffer this is the required by WMV  */
						else  if(( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_WMV3)){
						    VC1Frame  *pvc1Frm = &(pFormatCtx->VC1Data);
						    AVPacket *pkt = &encFrame; //for backward compatability
						    unsigned int  tempsize =encFrame.size, headersize =0; 
						    pkt->size = encFrame.size;		 

							/* For video packet we need to add vc1 header at the begining of the buffer this is the required by WMV  */
						    if (pvc1Frm->firstFrame == 1) {
						        memset(&gWMVseqHdr[0], 0x0, 100);	 
						        gWMVseqhdrLength = 0;
						        //  LOGI ("\n ********************** construct_vc1_sequence_header 111111111111 \n");
                                if(construct_vc1_sequence_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack])<0){
						            printf("\n ********************** construct_vc1_sequence_header failed \n");
						            return NULL;
						    }
							// LOGI ("\n ********************** construct_vc1_payload_header 111111111111 \n");
			
							if( construct_vc1_payload_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],VC1_CODE_SEQHDR,pkt)<0){
									printf("\n ********************** construct_vc1_payload_header failed \n");
									return NULL;
							}
							// LOGI ("\n ********************** construct_startcode_header 111111111111 \n");

							if(construct_startcode_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],VC1_CODE_SEQHDR)<0){
									printf("\n ********************** construct_startcode_header failed \n");
									return NULL;
							}
							tempsize += pvc1Frm->seq_startcode_header_size +pvc1Frm->seq_header_size +\
										pvc1Frm->seq_payload_header_size ;

							//Naresh @ copy header data to buffer
							if(headersize < 400)
							{
									memcpy(data,pvc1Frm->seq_startcode_header,pvc1Frm->seq_startcode_header_size);
									data += pvc1Frm->seq_startcode_header_size;
									headersize += pvc1Frm->seq_startcode_header_size;
									//copy seq hdr
									memcpy(gWMVseqHdr,pvc1Frm->seq_startcode_header,pvc1Frm->seq_startcode_header_size);
									gWMVseqhdrLength  = pvc1Frm->seq_startcode_header_size;

							}else {
									printf("\n ********************** seq_startcode_header_size copy failed \n");
									return NULL;
							}
							if(headersize < 400)
							{
									memcpy(data,pvc1Frm->seq_payload_header,pvc1Frm->seq_payload_header_size);
									data += pvc1Frm->seq_payload_header_size;
									headersize += pvc1Frm->seq_payload_header_size;	
									//copy seq hdr
									memcpy(gWMVseqHdr,pvc1Frm->seq_payload_header,pvc1Frm->seq_payload_header_size);
									gWMVseqhdrLength  += pvc1Frm->seq_payload_header_size;			   
							}else {
									printf("\n ********************** seq_payload_header_size copy failed \n");
									return NULL;
							}	
							if(headersize < 400)
							{		   
									memcpy(data,pvc1Frm->seq_header,pvc1Frm->seq_header_size);
									data += pvc1Frm->seq_header_size;
									headersize += pvc1Frm->seq_header_size;	
									//copy seq hdr
									memcpy(gWMVseqHdr,pvc1Frm->seq_header,pvc1Frm->seq_header_size);
									gWMVseqhdrLength  += pvc1Frm->seq_header_size;			   
							}else {
									printf("\n ********************** seq_header_size copy failed \n");
									return NULL;
							}						
							pvc1Frm->firstFrame = 0xf;	
							}
							else if(bIsseekmode)
							{
									//copy start seq data as decoder is resetting after seek
									/*     memcpy(data,gWMVseqHdr,gWMVseqhdrLength);
										   data += gWMVseqhdrLength;
										   headersize += gWMVseqhdrLength;		 */		
							}
							/* For video packet we need to add frame header at the begining of the buffer this is the required by WMV  */

							if(construct_codec_specific_picture_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],pkt)<0){
									printf("\n ********************** construct_vc1_sequence_header failed \n");
									return NULL;
							}
							if( construct_vc1_payload_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],VC1_CODE_FRAME,pkt)<0){
									printf("\n ********************** construct_vc1_payload_header failed \n");
									return NULL;
							}
							if(construct_startcode_header(pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],VC1_CODE_FRAME)<0){
									printf("\n ********************** construct_startcode_header failed \n");
									return NULL;
							}
							if(headersize < 400)
							{	
									memcpy(data,pvc1Frm->frame_startcode_header,pvc1Frm->frame_startcode_header_size);
									data += pvc1Frm->frame_startcode_header_size;
									headersize += pvc1Frm->frame_startcode_header_size;									 
							}else {
									printf("\n ********************** frame_startcode_header_size copy failed \n");
									return NULL;
							}	
							if(headersize < 400)
							{	             
									memcpy(data,pvc1Frm->frame_payload_header,pvc1Frm->frame_payload_header_size);
									data += pvc1Frm->frame_payload_header_size;
									headersize += pvc1Frm->seq_payload_header_size;									 
							}else {
									printf("\n ********************** seq_payload_header_size copy failed \n");
									return NULL;
							}	
							if(headersize < 400)
							{	
									memcpy(data,pvc1Frm->frame_header,pvc1Frm->frame_header_size);
									data+= pvc1Frm->frame_header_size;			 
									headersize += pvc1Frm->frame_header_size;					
							}else {
									printf("\n ********************** frame_header_size copy failed \n");
									return NULL;
							}	

							tempsize += pvc1Frm->frame_startcode_header_size + pvc1Frm->frame_payload_header_size +\
										pvc1Frm->frame_header_size;
							memcpy(data,encFrame.data,encFrame.size);		
							buffer->set_range(0, tempsize);

						}
						else if(( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_VC1)) {
								int i;
								AVCodecContext *codec_data = pFormatCtx->streams[mCurrentVideoTrack]->codec;		
								VC1Frame  *pvc1Frm = &(pFormatCtx->VC1Data);			 
								AVPacket *pkt = &encFrame; //for backward compatability
								unsigned int  tempsize =encFrame.size, headersize =0; ; 
								int offset        = -1;

								//LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  1111111111111 \n");

								pkt->size = encFrame.size;
								if (pvc1Frm->firstFrame == 1)
								{
										// LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  222222222222 \n");

										if((codec_data->extradata_size <= 5)||(codec_data->extradata == NULL))
										{
												printf("\n **********************  Error: No extra data for VC-1 Stream\n");
												return NULL;
										}

										memcpy(pvc1Frm->seq_header,&codec_data->extradata[1],codec_data->extradata_size - 1);
										pvc1Frm->seq_header_size = codec_data->extradata_size - 1;
										tempsize +=  pvc1Frm->seq_header_size;
										pvc1Frm->firstFrame = 0xf;
										//LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  44444444444 \n");

										if(headersize < 400)
										{
												memcpy(data,pvc1Frm->seq_header,pvc1Frm->seq_header_size);
												data += pvc1Frm->seq_header_size;
												headersize += pvc1Frm->seq_header_size;					
										}
										else {
												printf("\n ********************** seq_header_size copy failed \n");
												return NULL;
										}	   
										//LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  33333333333333 \n");

								}
								if(pFormatCtx->has_startcode != 1)
								{
										uint8_t seqcode[] = {0x00,0x00,0x01,0x0D};
										if((pkt->flags &AV_PKT_FLAG_KEY)&&(pvc1Frm->firstFrame != 1))
										{
												for(i = codec_data->extradata_size; i >= 5; i--)
												{
														/* Find the Entry Point Start Code Offset */
														if(*(unsigned int *)(&codec_data->extradata[i - 4]) == 0x0E010000)
														{
																offset = i - 4;
																break;
														}
												}
												if(offset == -1)
												{
														printf("\n ********************** Failed to Find Entry  Point Header in VC-1 AP Extra data\n");
														return NULL;
												}

												/* Size = Entry Point Start Code + Entry Point Header + Frame Start Code */
												entrypointSize = (codec_data->extradata_size) - (offset);
												tempsize +=  entrypointSize + 4;
												memcpy(data,&data[offset],entrypointSize);
												data += entrypointSize;
												//LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  bbbbbbbbbb \n");

										}
										else
										{
												entrypointSize = 0;
												tempsize += 4;
										}
										// LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  cccccccccccc \n");
										memcpy(data,&seqcode[0],4);
										data += 4;

										/*		 
												 buffer[0] = 0x00;
												 buffer[1] = 0x00;
												 buffer[2] = 0x01;
												 buffer[3] = 0x0D; */			 
								}
								//copy frame data 	
								// 	              LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  dddddddddddddd \n");

								memcpy(data,encFrame.data,encFrame.size);			
								buffer->set_range(0, tempsize);
								// LOGI ("\n **********************CODEC_ID_VC1CODEC_ID_VC1  eeeeeeeeeeeeeee \n"); 
						}
						else if(( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_RV40) || (( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_RV30))) {
                            VSliceInfo info;
							int ioffset = 0;
							int offset = 0;
							uint8_t* datain = encFrame.data; 
							
							ioffset = ffmpeg_rv_parse_slice_info(datain,&info);
							
							if(mFirstFrame == 1)
							{
                               offset += ffmepg_rv_append_sequence_header(data,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack]);
							   mFirstFrame = 0;
							}

							offset += ffmpeg_rv_append_picture_header(data+offset,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],&info,encFrame.size);
							offset += ffmpeg_rv_append_slice(data+offset,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],&info,(uint8_t *)(encFrame.data+ioffset),encFrame.size);

							buffer->set_range(0, offset );
                            
						}
						else if(( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_VP6) || 
							    (( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_VP8)) ||
							    (( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_VP6F))) {
							int offset = 0; 
							uint8_t* datain = encFrame.data; 

							if(mFirstFrame == 1)
							{
                               offset += ffmepg_vp68_append_sequence_header(data,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack]);
							   mFirstFrame = 0;
							}

							offset += ffmpeg_vp68_append_picture_header(data+offset,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],encFrame.size);
							offset += ffmpeg_vp68_append_slice(data+offset,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],(uint8_t *)(encFrame.data),encFrame.size);

							buffer->set_range(0, offset);
							
						}
						else if(( pFormatCtx->streams[mCurrentVideoTrack]->codec->codec_id ==  AV_CODEC_ID_FLV1)){
							int offset = 0; 
							uint8_t* datain = encFrame.data; 

							if(mFirstFrame == 1)
							{
                               offset += ffmepg_spark_append_sequence_header(data,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack]);
							   mFirstFrame = 0;
							}

							offset += ffmpeg_spark_append_picture_header(data+offset,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],encFrame.size);
							offset += ffmpeg_spark_append_slice(data+offset,pFormatCtx,pFormatCtx->streams[mCurrentVideoTrack],(uint8_t *)(encFrame.data),encFrame.size);

							buffer->set_range(0, offset);
							
						}
						#endif
						else{
								//Nothing special needed
								memcpy(data,encFrame.data,encFrame.size);
								buffer->set_range(0, encFrame.size);         
						}

						if(trackIndex == encFrame.stream_index){
								bFrameAvailable = true;
								frame = buffer;
						}else{
								TrackEncFrame *t = new TrackEncFrame();
								t->encFrame = buffer;

								{
									//lock the list while pushing
									Mutex::Autolock autoLock(mVListLock);
									encVideoFrameList.push_back(t);
									mVListSize+=buffer->size();
									//ALOGE("vlist size at push (%d)\n", mVListSize);
								}
/* This could be used to prevent the list to eat too much memory
								while(  (mVListSize > VIDEO_MAXQ_SIZE) && (!mEof) )
								{
									ALOGE("throttle audio, video size limit (%d) reached", mVListSize);
									usleep(15000);
								}
*/
						}
					//	ALOGI("video frame found\n");
				} else if (mCurrentAudioTrack == encFrame.stream_index) {
                         uint8_t *data = (uint8_t *)buffer->data();
						unsigned int    temp32 =0,extradata = 0;
						if((pFormatCtx->streams[mCurrentAudioTrack]->codec->codec_id ==  AV_CODEC_ID_WMAV1) ||
										(pFormatCtx->streams[mCurrentAudioTrack]->codec->codec_id ==  AV_CODEC_ID_WMAV2) ||
										(pFormatCtx->streams[mCurrentAudioTrack]->codec->codec_id ==  AV_CODEC_ID_WMAPRO) ||
										(pFormatCtx->streams[mCurrentAudioTrack]->codec->codec_id ==  AV_CODEC_ID_AC3))
						{
						/*

								temp32 = 0xFFFFFFFF;
								extradata = 4;
								// copy size at the begining
								memcpy(data,&temp32,4);      
								data = data+4; */
						}
						memcpy(data,encFrame.data,encFrame.size);	  
						buffer->set_range(0, encFrame.size+extradata);
						if(trackIndex == encFrame.stream_index){
								bFrameAvailable = true;
								frame = buffer;
						}
						else{

								TrackEncFrame* t = new TrackEncFrame();
								t->encFrame = buffer;
								//lock the list while pushing
								{	
									Mutex::Autolock autoLock(mAListLock);
									encAudioFrameList.push_back(t);
									mAListSize+=buffer->size();
									//ALOGE("alist size at push (%d)\n", mAListSize);
								}
/* This could be used to prevent the list to eat too much memory
								while( (mAListSize>AUDIO_MAXQ_SIZE) && (!mEof))
								{
									ALOGE("throttle video, audio size limit (%d) reached", mAListSize);
									usleep(15000);
								}
*/

						}
//								ALOGI("audio frame found size=%d\n",encFrame.size);
				}
				else{
						//For now ignore subtitles/drm etc
						buffer->release();
				}

				//Free the frame
				{
					//Mutex::Autolock autoLock(mLock);
					av_free_packet(&encFrame);
				}

		}//while(bFrameAvailable)

/* //Comment this one out to print info about the list count
		{
			Mutex::Autolock autoLock(mAListLock);
			Mutex::Autolock autoLock1(mVListLock);
			ALOGE("VS=count(%d)/bytes(%d) AS=count(%d)/bytes(%d) \n", encVideoFrameList.size(),  mVListSize, encAudioFrameList.size(), mAListSize );
		}
*/

		return frame;
}
bool FfmpegExtractor::mIsAvRegistered = false;
Vector<FfmpegExtractor::avcontext> FfmpegExtractor::mContexts;

bool SniffFfmpeg(
				const sp<DataSource> &source, String8 *mimeType, float *confidence) {
		//char streamBuf[400];
		String8 streamBuf;
		AVFormatContext *pFormatCtx = NULL;
		int ret = 0;

		mimeType->setTo(MEDIA_MIMETYPE_CONTAINER_FFMPEG);
		if(!bIsAvRegistered){
			    printf("register all\n");
				av_register_all();
				av_log_set_level(AV_LOG_WARNING);
				//av_log_set_callback(Ffmpeg_log_callback);
				bIsAvRegistered = false;
		}
		streamBuf = source->getUri();
		printf("FFMPEG sniff %s\n",streamBuf.string());
		//try opening file 
		if ((ret = (avformat_open_input(&pFormatCtx, (const char *)streamBuf.string(),NULL, NULL))) < 0)
		{
				/* Couldn't open file*/ 
				printf("av_open_input failed for file while sniffing ret %x!!\n",ret);
				*confidence = 0;
				return false;
		}
		else{
				*confidence = 0.0001;
				printf("sniff pass with 0.0001 confidence by ffmpeg\n");
				if(pFormatCtx){

						bool  IsSupportedVideoFormat = false, IsSupportedAudioFormat= false;
						//  to change confidence level to bypass mp3 extractor in case mpeg audio with supported video format found
						/* Retrieve stream information */
						if((ret = avformat_find_stream_info(pFormatCtx,NULL))<0)
						{
								/* Couldn't find stream information */
								printf("av_find_stream_info failed %d %s ret %x",__LINE__,__FUNCTION__,ret);
								return -1;
						}
						/* Add tracks */
						for(  unsigned int uCount=0; uCount<pFormatCtx->nb_streams; uCount++)
						{
								printf("trackindx:%d extradata size:%d\n",uCount,pFormatCtx->streams[uCount]->codec->extradata_size);
								if(pFormatCtx->streams[uCount]->codec->codec_type == AVMEDIA_TYPE_VIDEO) 
								{
										switch(pFormatCtx->streams[uCount]->codec->codec_id)
										{			
												case	AV_CODEC_ID_MPEG2VIDEO: 
												case	AV_CODEC_ID_MPEG1VIDEO: 
												case    AV_CODEC_ID_MSMPEG4V2:
												case    AV_CODEC_ID_MSMPEG4V1:
												case	AV_CODEC_ID_MSMPEG4V3:
												case	AV_CODEC_ID_MPEG4:
												case	AV_CODEC_ID_H264 :
												//case	AV_CODEC_ID_XVID:
												case	AV_CODEC_ID_WMV2:
												case	AV_CODEC_ID_WMV1:
												case	AV_CODEC_ID_WMV3:
												case	AV_CODEC_ID_VC1: 
												case    AV_CODEC_ID_RV30:
												case    AV_CODEC_ID_RV40:
												case    AV_CODEC_ID_VP6:
												case    AV_CODEC_ID_VP6F:
												case    AV_CODEC_ID_VP8:
												case    AV_CODEC_ID_FLV1:
												case    AV_CODEC_ID_VP9:
												case    AV_CODEC_ID_H265:
														printf("SniffFfmpeg FOUND supported video format %x \n",pFormatCtx->streams[uCount]->codec->codec_id);
														IsSupportedVideoFormat = true;
														break;

												default:
														printf("SniffFfmpeg unsupported video format %x\n",pFormatCtx->streams[uCount]->codec->codec_id);
														break;				  
										}
								}
								else if(pFormatCtx->streams[uCount]->codec->codec_type == AVMEDIA_TYPE_AUDIO) 
								{
										switch(pFormatCtx->streams[uCount]->codec->codec_id)
										{
												case  AV_CODEC_ID_MP1 :
												case  AV_CODEC_ID_MP2 :
												case  AV_CODEC_ID_MP3 : 
												case  AV_CODEC_ID_AAC :
												case  AV_CODEC_ID_AC3 :
												case  AV_CODEC_ID_DTS :
												case  AV_CODEC_ID_WMAV1:
												case  AV_CODEC_ID_WMAV2:
												case  AV_CODEC_ID_WMAPRO:
												case  AV_CODEC_ID_WMALOSSLESS:
												case  AV_CODEC_ID_EAC3:	
												case  AV_CODEC_ID_COOK:	
														printf("SniffFfmpeg FOUND MPEG AUdio format %x \n",pFormatCtx->streams[uCount]->codec->codec_id);						 	
														IsSupportedAudioFormat = true;
														break;

												default:
														printf("SniffFfmpeg unsupported audio format %x\n",pFormatCtx->streams[uCount]->codec->codec_id);
														break;
										}

								}
						}			
						if(IsSupportedAudioFormat || IsSupportedVideoFormat)
						{
								printf("SniffFfmpeg Increasing FFMPEG Confidence level video supported %d audio supported %d\n",IsSupportedAudioFormat,IsSupportedVideoFormat);						 	

								*confidence = 0.3f;	     		
						}
						avformat_close_input(&pFormatCtx);
				}
		}
		return true;
}
