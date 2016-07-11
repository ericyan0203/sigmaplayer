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
#include <stdint.h>
#include <utils.h>
#include <stdio.h>

#include "SigmaMediaPlayerImpl.h"
#include "DataSource.h"
#include "FileSource.h"
#include "MediaBuffer.h"
#include "MediaDefs.h"
#include "MediaExtractor.h"
#include "MediaSource.h"
#include "MetaData.h"

#define DEFAULT_IP  	"127.0.0.1"
#define DEFAULT_PORT 	0

#define HALSYS  1

static int64_t kLowWaterMarkUs = 2000000ll;  // 2secs
static int64_t kHighWaterMarkUs = 5000000ll;  // 5secs
static const size_t kLowWaterMarkBytes = 40000;
static const size_t kHighWaterMarkBytes = 200000;

// maximum time in paused state when offloading audio decompression. When elapsed, the AudioPlayer
// is destroyed to allow the audio DSP to power down.
static int64_t kOffloadPauseMaxUs = 60000000ll;

static int32_t convertToSigmaFormat(const char * mime) {
	int32_t ret = 0;
	if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)||!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
		ret = (int32_t)SIGM_VIDEO_CodingAVC;
	} else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC)) {
		ret = (int32_t)SIGM_AUDIO_CodingAAC;
	}else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_WMA1)) {
		ret = (int32_t)SIGM_AUDIO_CodingWMA;
	}else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_WMV1)) {
		ret = (int32_t)SIGM_VIDEO_CodingVC1;
	}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_RV)) {
		ret = (int32_t)SIGM_VIDEO_CodingRV;
	}else if(!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_RA)) {
		ret = (int32_t)SIGM_VIDEO_CodingRV;
	}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP6)) {
		ret = (int32_t)SIGM_VIDEO_CodingVP6;
	}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP8)||!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP8X)) {
		ret = (int32_t)SIGM_VIDEO_CodingVP8;
	}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_VP9)){ 
		ret = (int32_t)SIGM_VIDEO_CodingVP9;
	}else if(!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_HEVC)){ 
		ret = (int32_t)SIGM_VIDEO_CodingHEVC;
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////
SigmaMediaPlayerImpl::SigmaMediaPlayerImpl()
    : mUIDValid(false),
      mDisplayWidth(0),
      mDisplayHeight(0),
      mFlags(0),
      mExtractorFlags(0),
      mIP(DEFAULT_IP),
      mPort(DEFAULT_PORT),
      haveAudio(0),
      haveVideo(0),
      mVideoFormat(SIGM_VIDEO_CodingUnused),
      mAudioFormat(SIGM_AUDIO_CodingUnused),
      mHandle((void *)-1){

	//Init Platform
#ifdef HALSYS
	mClient.connect(mIP,mPort);
#endif	
    DataSource::RegisterDefaultSniffers();

    reset();
}

SigmaMediaPlayerImpl::SigmaMediaPlayerImpl(const char * ip, int32_t port)
    : mUIDValid(false),
      mDisplayWidth(0),
      mDisplayHeight(0),
      mFlags(0),
      mExtractorFlags(0),
      mIP(ip),
      mPort(port),
      haveAudio(0),
      haveVideo(0),
      mVideoFormat(SIGM_VIDEO_CodingUnused),
      mAudioFormat(SIGM_AUDIO_CodingUnused){
	
	mHandle = (sigma_handle_t)-1;
	//Init Platform
#ifdef HALSYS
	mClient.connect(mIP,mPort);
#endif	
    DataSource::RegisterDefaultSniffers();

    reset();
}

SigmaMediaPlayerImpl::~SigmaMediaPlayerImpl() {
    reset();

	//disconnect halsys platform
#ifdef HALSYS
    mClient.disconnect();
#endif

}

void SigmaMediaPlayerImpl::setUID(uid_t uid) {
    printf("MediaPlayer running on behalf of uid %d\n", uid);

    mUID = uid;
    mUIDValid = true;
}

Error_Type_e SigmaMediaPlayerImpl::setDataSource(const char *uri) {
    Mutex::Autolock autoLock(mLock);
    return setDataSource_l(uri);
}

Error_Type_e SigmaMediaPlayerImpl::setDataSource_l(const char *uri) {
    reset_l();

    mUri = uri;
 
    printf("setDataSource_l(%s)\n", mUri.string());

    // The actual work will be done during preparation in the call to
    // ::finishSetDataSource_l to avoid blocking the calling thread in
    // setDataSource for any significant time.

    {
        Mutex::Autolock autoLock(mStatsLock);
        mStats.mFd = -1;
        mStats.mURI = mUri;
    }

    sp<DataSource> dataSource = new FileSource(uri);

    Error_Type_e err = dataSource->initCheck();

    if (err != SIGM_ErrorNone) {
        return err;
    }

    mFileSource = dataSource;

    return setDataSource_l(dataSource);
}


Error_Type_e SigmaMediaPlayerImpl::setDataSource_l(const sp<DataSource> &dataSource) {
    sp<MediaExtractor> extractor = MediaExtractor::Create(dataSource);

    if (extractor == NULL) {
		printf("Can't create extractor\n");
        return SIGM_ErrorUndefined;
    }

    return setDataSource_l(extractor);
}


Error_Type_e SigmaMediaPlayerImpl::setDataSource_l(const sp<MediaExtractor> &extractor) {
    // Attempt to approximate overall stream bitrate by summing all
    // tracks' individual bitrates, if not all of them advertise bitrate,
    // we have to fail.

    int64_t totalBitRate = 0;
	Error_Type_e ret = SIGM_ErrorNone;

    mExtractor = extractor;
    for (size_t i = 0; i < extractor->countTracks(); ++i) {
        sp<MetaData> meta = extractor->getTrackMetaData(i);

        int32_t bitrate;
        if (!meta->findInt32(kKeyBitRate, &bitrate)) {
            const char *mime;
            meta->findCString(kKeyMIMEType, &mime);
            printf("track of type '%s' does not publish bitrate\n", mime);

            totalBitRate = -1;
            break;
        }

        totalBitRate += bitrate;
    }
    sp<MetaData> fileMeta = mExtractor->getMetaData();
    if (fileMeta != NULL) {
        int64_t duration;
        if (fileMeta->findInt64(kKeyDuration, &duration)) {
            mDurationUs = duration;
        }
    }

    mBitrate = totalBitRate;

    printf("mBitrate = %lld bits/sec\n", mBitrate);

    {
        Mutex::Autolock autoLock(mStatsLock);
        mStats.mBitrate = mBitrate;
        mStats.mTracks.clear();
        mStats.mAudioTrackIndex = -1;
        mStats.mVideoTrackIndex = -1;
    }
	
    for (size_t i = 0; i < extractor->countTracks(); ++i) {
        sp<MetaData> meta = extractor->getTrackMetaData(i);

        const char *_mime;

		meta->findCString(kKeyMIMEType, &_mime);

        String8 mime = String8(_mime);

        if (!haveVideo && !strncasecmp(mime.string(), "video/", 6)) {
            setVideoSource(extractor->getTrack(i));
            haveVideo = true;

            // Set the presentation/display size
            int32_t displayWidth, displayHeight;
            bool success = meta->findInt32(kKeyDisplayWidth, &displayWidth);
            if (success) {
                success = meta->findInt32(kKeyDisplayHeight, &displayHeight);
            }
            if (success) {
                mDisplayWidth = displayWidth;
                mDisplayHeight = displayHeight;
            }

            {
                Mutex::Autolock autoLock(mStatsLock);
                mStats.mVideoTrackIndex = mStats.mTracks.size();
                mStats.mTracks.push();
                TrackStat *stat =
                    &mStats.mTracks.editItemAt(mStats.mVideoTrackIndex);
                stat->mMIME = mime.string();
            }

			mVideoFormat = (Video_CodingType_e)convertToSigmaFormat(mime.string());
        } else if (!haveAudio && !strncasecmp(mime.string(), "audio/", 6)) {
            setAudioSource(extractor->getTrack(i));
            haveAudio = true;
            mActiveAudioTrackIndex = i;

            {
                Mutex::Autolock autoLock(mStatsLock);
                mStats.mAudioTrackIndex = mStats.mTracks.size();
                mStats.mTracks.push();
                TrackStat *stat =
                    &mStats.mTracks.editItemAt(mStats.mAudioTrackIndex);
                stat->mMIME = mime.string();
            }

			mAudioFormat = (Audio_CodingType_e)convertToSigmaFormat(mime.string());
   	 	}
    }
   
    if (!haveAudio && !haveVideo) {
         return SIGM_ErrorFailed;
    }

    mExtractorFlags = extractor->flags();
	
#ifdef HALSYS
	ret = HalSys_Media_Initialize();
#endif
    return ret;
}

void SigmaMediaPlayerImpl::reset() {
    Mutex::Autolock autoLock(mLock);
    reset_l();
}

void SigmaMediaPlayerImpl::reset_l() {
 
    mActiveAudioTrackIndex = -1;
    mDisplayWidth = 0;
    mDisplayHeight = 0;

	printf("reset_l \n");

    mAudioTrack.clear();
    mVideoTrack.clear();
    mExtractor.clear();

    mDurationUs = -1;
    modifyFlags(0, ASSIGN);
    mExtractorFlags = 0;
    mVideoTimeUs = 0;

    mSeeking = NO_SEEK;
    mSeekNotificationSent = true;
    mSeekTimeUs = 0;

    mUri.setTo("");

    mFileSource.clear();

    mBitrate = -1;
 //   mLastVideoTimeUs = -1;

    {
        Mutex::Autolock autoLock(mStatsLock);
        mStats.mFd = -1;
        mStats.mURI = String8();
        mStats.mBitrate = -1;
        mStats.mAudioTrackIndex = -1;
        mStats.mVideoTrackIndex = -1;
        mStats.mNumVideoFramesDecoded = 0;
        mStats.mNumVideoFramesDropped = 0;
        mStats.mVideoWidth = -1;
        mStats.mVideoHeight = -1;
        mStats.mFlags = 0;
        mStats.mTracks.clear();
    }

}

void SigmaMediaPlayerImpl::onStreamDone() {
    Mutex::Autolock autoLock(mLock);
   
    const bool allDone = true;

    if (!allDone) {
        return;
    }

    if (allDone) {
        // Don't AUTO_LOOP if we're being recorded, since that cannot be
        // turned off and recording would go on indefinitely.
        seekTo_l(0);

    } else {
       printf("MEDIA_PLAYBACK_COMPLETE\n");
		
        pause_l(true /* at eos */);

        modifyFlags(AT_EOS, SET);
    }
}

Error_Type_e SigmaMediaPlayerImpl::play() {

    Mutex::Autolock autoLock(mLock);

    modifyFlags(CACHE_UNDERRUN, CLEAR);

    return play_l();
}

Error_Type_e SigmaMediaPlayerImpl::play_l() {
	Error_Type_e ret = SIGM_ErrorNone;
	
    if (mFlags & PLAYING) {
        return SIGM_ErrorNone;
    }

 
    if (!(mFlags & PREPARED)) {
        Error_Type_e err = prepare_l();

        if (err != SIGM_ErrorNone) {
            return err;
        }
    }

    modifyFlags(PLAYING, SET);
    modifyFlags(FIRST_FRAME, SET);

    if (mFlags & AT_EOS) {
        // Legacy behaviour, if a stream finishes playing and then
        // is started again, we play from the start...
        seekTo_l(0);
    }

#ifdef HALSYS
	if(mHandle != (sigma_handle_t)-1) 
		ret = HalSys_Media_Start(mHandle);	
#endif


	if(ret != SIGM_ErrorNone){
		 modifyFlags((PLAYING | FIRST_FRAME), CLEAR);
	}

	if(haveVideo) mVideoTrack->start(NULL);
	if(haveAudio) mAudioTrack->start(NULL);
	
	return ret;
}

Error_Type_e SigmaMediaPlayerImpl::stop() {
	printf("stop\n");
	return stop_l();
}

Error_Type_e SigmaMediaPlayerImpl::stop_l(){
	Error_Type_e ret = SIGM_ErrorNone;
	
#if HALSYS
	if(mHandle != (sigma_handle_t)(-1) && (mFlags|PLAYING)){
		ret = HalSys_Media_Stop(mHandle);	
	}
#endif
	modifyFlags(PLAYING, CLEAR);

#ifdef HALSYS
	if(ret == SIGM_ErrorNone && (mFlags|PREPARED)) {
		ret = HalSys_Media_Close(mHandle);
	}
#endif

	modifyFlags(PREPARED, CLEAR); //should move to antohre api?
 	
	return ret;
}

Error_Type_e SigmaMediaPlayerImpl::pause() {
    Mutex::Autolock autoLock(mLock);

    modifyFlags(CACHE_UNDERRUN, CLEAR);

    return pause_l();
}

Error_Type_e SigmaMediaPlayerImpl::pause_l(bool at_eos) {
	Error_Type_e ret = SIGM_ErrorNone;
	
    if (!(mFlags & PLAYING)) {    
        return ret;
    }

    modifyFlags(PLAYING, CLEAR);

    modifyFlags(PAUSED, SET);

#ifdef HALSYS
	if(mHandle == (sigma_handle_t)-1) {
		ret = HalSys_Media_Pause(mHandle);
	}
#endif

    return ret;
}

bool SigmaMediaPlayerImpl::isPlaying() const {
    return (mFlags & PLAYING) || (mFlags & CACHE_UNDERRUN);
}


Error_Type_e SigmaMediaPlayerImpl::seekTo(int64_t timeUs) {
	
    if (mExtractorFlags & MediaExtractor::CAN_SEEK) {
        Mutex::Autolock autoLock(mLock);
        return seekTo_l(timeUs);
    }

    return SIGM_ErrorNone;
}

Error_Type_e SigmaMediaPlayerImpl::seekTo_l(int64_t timeUs) {
    mSeeking = SEEK;
    mSeekNotificationSent = false;
    mSeekTimeUs = timeUs;
    modifyFlags((AT_EOS | AUDIO_AT_EOS | VIDEO_AT_EOS), CLEAR);

    return SIGM_ErrorNone;
}

Error_Type_e SigmaMediaPlayerImpl::prepare() {
  
    Mutex::Autolock autoLock(mLock);
    return prepare_l();
}

Error_Type_e SigmaMediaPlayerImpl::prepare_l() {
	Error_Type_e ret = SIGM_ErrorNone;
	Media_Config_t tMediaConfig;
	
    if (mFlags & PREPARED) {
        return SIGM_ErrorNone;
    }

    if (mFlags & PREPARING) {
        return SIGM_ErrorIncorrectStateTransition;
    }

	modifyFlags(PREPARING, SET);

#ifdef HALSYS
	 
    tMediaConfig.bLowLatency = trid_false;
    tMediaConfig.eClockMode = CLOCK_MODE_TIMER;

    tMediaConfig.tAudioConfig.eAudioFormat = mAudioFormat;
    tMediaConfig.tAudioConfig.eSoundSink = (mAudioFormat == SIGM_AUDIO_CodingUnused)?0:(SIGM_SOUND_SINK_HEADPHONE | SIGM_SOUND_SINK_SPDIF_PCM | SIGM_SOUND_SINK_SPEAKER);

    tMediaConfig.tVideoConfig.eVideoDisplayMode = SIGM_SEAMLESS_NONE;
    tMediaConfig.tVideoConfig.eVideoFormat = mVideoFormat ;
    tMediaConfig.tVideoConfig.eVideoPlayMode = SIGM_PLAYMODE_NORMAL;
    tMediaConfig.tVideoConfig.eVideoStreamMode = SIGM_STREAMMODE_NORMAL;
    tMediaConfig.tVideoConfig.eVideoSink = (Video_Sink_e)((mVideoFormat== SIGM_VIDEO_CodingUnused)?0:SIGM_VIDEO_SINK_MP);
	tMediaConfig.tVideoConfig.eMuxType = SIGM_MUX_ES;
    ret = HalSys_Media_Open(&tMediaConfig, &mHandle);

	for (size_t i = 0; i < mExtractor->countTracks(); ++i) {
        sp<MetaData> meta = mExtractor->getTrackMetaData(i);
		meta->setInt32(kKeyPlatformPrivate, (int32_t)mHandle);
	}
#endif

   if(ret == SIGM_ErrorNone) {
		modifyFlags((PREPARING|PREPARE_CANCELLED|PREPARING_CONNECTED), CLEAR);
		modifyFlags(PREPARED, SET);
   	}

	return SIGM_ErrorNone;
}


uint32_t SigmaMediaPlayerImpl::flags() const {
    return mExtractorFlags;
}

Error_Type_e SigmaMediaPlayerImpl::setParameter(int key, const void *request) {
   return SIGM_ErrorNone;
}

Error_Type_e SigmaMediaPlayerImpl::getParameter(int key, void *reply) {
    return SIGM_ErrorNone;
}

Error_Type_e SigmaMediaPlayerImpl::getTrackInfo(void *reply) const {
    Mutex::Autolock autoLock(mLock);
#if 0
    size_t trackCount = mExtractor->countTracks();
#endif
    return SIGM_ErrorNone;
}

Error_Type_e SigmaMediaPlayerImpl::selectTrack(size_t trackIndex, bool select) {
 
    printf("selectTrack: trackIndex = %zu and select=%d\n", trackIndex, select);
    Mutex::Autolock autoLock(mLock);

	#if 0
    size_t trackCount = mExtractor->countTracks();
    
    if (trackIndex >= trackCount) {
        ALOGE("Track index (%zu) is out of range [0, %zu)", trackIndex, trackCount);
        return ERROR_OUT_OF_RANGE;
    }

    bool isAudioTrack = false;
    if (trackIndex < mExtractor->countTracks()) {
        sp<MetaData> meta = mExtractor->getTrackMetaData(trackIndex);
        const char *mime;
        CHECK(meta->findCString(kKeyMIMEType, &mime));
        isAudioTrack = !strncasecmp(mime, "audio/", 6);

        if (!isAudioTrack && strcasecmp(mime, MEDIA_MIMETYPE_TEXT_3GPP) != 0) {
            ALOGE("Track %zu is not either audio or timed text", trackIndex);
            return ERROR_UNSUPPORTED;
        }
    }

    if (isAudioTrack) {
        if (!select) {
            ALOGE("Deselect an audio track (%zu) is not supported", trackIndex);
            return ERROR_UNSUPPORTED;
        }
        return selectAudioTrack_l(mExtractor->getTrack(trackIndex), trackIndex);
    }

    // Timed text track handling
    if (mTextDriver == NULL) {
        return INVALID_OPERATION;
    }

    status_t err = OK;
    if (select) {
        err = mTextDriver->selectTrack(trackIndex);
        if (err == OK) {
            modifyFlags(TEXTPLAYER_INITIALIZED, SET);
            if (mFlags & PLAYING && !(mFlags & TEXT_RUNNING)) {
                mTextDriver->start();
                modifyFlags(TEXT_RUNNING, SET);
            }
        }
    } else {
        err = mTextDriver->unselectTrack(trackIndex);
        if (err == OK) {
            modifyFlags(TEXTPLAYER_INITIALIZED, CLEAR);
            modifyFlags(TEXT_RUNNING, CLEAR);
        }
    }
#endif
    return SIGM_ErrorNone;
}

size_t SigmaMediaPlayerImpl::countTracks() const {
	return mExtractor->countTracks();
}

void SigmaMediaPlayerImpl::modifyFlags(unsigned value, FlagMode mode) {
    switch (mode) {
        case SET:
            mFlags |= value;
            break;
        case CLEAR:
            if ((value & CACHE_UNDERRUN) && (mFlags & CACHE_UNDERRUN)) {
             //   notifyListener_l(MEDIA_INFO, MEDIA_INFO_BUFFERING_END);
            }
            mFlags &= ~value;
            break;
        case ASSIGN:
            mFlags = value;
            break;
        default:
            break;
    }

    {
        Mutex::Autolock autoLock(mStatsLock);
        mStats.mFlags = mFlags;
    }
}

void SigmaMediaPlayerImpl::setVideoSource(sp<MediaSource> source) {
    mVideoTrack = source;
}

void SigmaMediaPlayerImpl::setAudioSource(sp<MediaSource> source) {
    mAudioTrack = source;
}

void SigmaMediaPlayerImpl::setListener(const wp<ISigmaPlayer> &listener) {
    Mutex::Autolock autoLock(mLock);
    mListener = listener;
}

Error_Type_e SigmaMediaPlayerImpl::notifyListener_l(int msg, int ext1, int ext2) {
    if (mListener != NULL) {
        sp<ISigmaPlayer> listener = mListener.promote();

        if (listener != NULL) {
            return listener->sendEvent(msg, ext1, ext2);
        }
    }
	return SIGM_ErrorNone;
}

Error_Type_e SigmaMediaPlayerImpl::sendEvent(int msg, int ext1, int ext2) {
	return SIGM_ErrorNone;
}

