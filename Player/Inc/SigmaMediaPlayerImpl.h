#ifndef SIGMA_PLAYER_IMPL_H_
#define SIGMA_PLAYER_IMPL_H_

#include "SIGM_Types.h"

#include "ISigmaPlayer.h"
#include "DataSource.h"
#include "HalSysClient.h"
#include "MetaData.h"
#include <Threads.h>
#include "MediaSource.h"
#include "MediaExtractor.h"
#include "SIGM_Media_API.h"

struct SigmaMediaPlayerImpl {
    SigmaMediaPlayerImpl();
    ~SigmaMediaPlayerImpl();

    void setUID(uid_t uid);

    Error_Type_e setDataSource(const char *uri);

    void reset();

    Error_Type_e prepare();

    Error_Type_e play();
	Error_Type_e stop();
    Error_Type_e pause();
	Error_Type_e resume();

    bool isPlaying() const;

    Error_Type_e setParameter(int key, const void * request);
    Error_Type_e getParameter(int key, void *reply);

    Error_Type_e seekTo(int64_t timeUs);

    // This is a mask of MediaExtractor::Flags.
    uint32_t flags() const;

	void setListener(const wp<ISigmaPlayer> &listener);

	Error_Type_e  notifyListener_l(int msg, int ext1 = 0, int ext2 = 0);

	Error_Type_e  sendEvent(int msg, int ext1=0, int ext2=0);

private:
    enum {
        PLAYING             = 0x01,
        LOOPING             = 0x02,
        FIRST_FRAME         = 0x04,
        PREPARING           = 0x08,
        PREPARED            = 0x10,
        AT_EOS              = 0x20,
        PREPARE_CANCELLED   = 0x40,
        CACHE_UNDERRUN      = 0x80,
        AUDIO_AT_EOS        = 0x0100,
        VIDEO_AT_EOS        = 0x0200,
        AUTO_LOOPING        = 0x0400,

        // We are basically done preparing but are currently buffering
        // sufficient data to begin playback and finish the preparation phase
        // for good.
        PREPARING_CONNECTED = 0x0800,

        // We're triggering a single video event to display the first frame
        // after the seekpoint.
        SEEK_PREVIEW        = 0x1000,

        AUDIO_RUNNING       = 0x2000,
        AUDIOPLAYER_STARTED = 0x4000,

        INCOGNITO           = 0x8000,

        TEXT_RUNNING        = 0x10000,
        TEXTPLAYER_INITIALIZED  = 0x20000,

        SLOW_DECODER_HACK   = 0x40000,
        PAUSED              = 0x80000,

    };

    mutable Mutex mLock;
   
    mutable Mutex mStatsLock;
    
    bool mUIDValid;
    uid_t mUID;
	
    String8 mUri;

	bool haveAudio;
	Video_CodingType_e mVideoFormat;
		
    bool haveVideo;
	Audio_CodingType_e mAudioFormat;
	
    sp<DataSource> mFileSource;

    sp<MediaSource> mVideoTrack;
	sp<MediaSource> mAudioTrack;

	wp<ISigmaPlayer> mListener;
	
    ssize_t mActiveAudioTrackIndex;
	ssize_t mActiveVideoTrackIndex;

    int64_t mDurationUs;

    int32_t mDisplayWidth;
    int32_t mDisplayHeight;

	uint32_t mFlags;
    uint32_t mExtractorFlags;
    
    int64_t mVideoTimeUs;

    enum SeekType {
        NO_SEEK,
        SEEK,
        SEEK_VIDEO_ONLY
    };
    SeekType mSeeking;

    bool mSeekNotificationSent;
    int64_t mSeekTimeUs;

    int64_t mBitrate;  // total bitrate of the file (in bps) or -1 if unknown.

    Error_Type_e play_l();
	Error_Type_e stop_l();

    MediaBuffer *mVideoBuffer;    
    sp<MediaExtractor> mExtractor;

	sp<HalSysClient> mClient;

    Error_Type_e setDataSource_l(const char *uri);
    Error_Type_e setDataSource_l(const sp<DataSource> &dataSource);
    Error_Type_e setDataSource_l(const sp<MediaExtractor> &extractor);

	void setVideoSource(sp<MediaSource> source);
	void setAudioSource(sp<MediaSource> source);


    void reset_l();
    Error_Type_e seekTo_l(int64_t timeMs);
    Error_Type_e pause_l(bool at_eos = false);
	Error_Type_e resume_l();

	Error_Type_e prepare_l();
  
    void onStreamDone();

  //  void notifyListener_l(int msg, int ext1 = 0, int ext2 = 0);

    enum FlagMode {
        SET,
        CLEAR,
        ASSIGN
    };
    void modifyFlags(unsigned value, FlagMode mode);

    struct TrackStat {
        String8 mMIME;
        String8 mDecoderName;
    };

    // protected by mStatsLock
    struct Stats {
        int mFd;
        String8 mURI;
        int64_t mBitrate;

        // FIXME:
        // These two indices are just 0 or 1 for now
        // They are not representing the actual track
        // indices in the stream.
        ssize_t mAudioTrackIndex;
        ssize_t mVideoTrackIndex;

        int64_t mNumVideoFramesDecoded;
        int64_t mNumVideoFramesDropped;
        int32_t mVideoWidth;
        int32_t mVideoHeight;
        uint32_t mFlags;
        Vector<TrackStat> mTracks;
    } mStats;

    Error_Type_e getTrackInfo(void * reply) const;

    // when select is true, the given track is selected.
    // otherwise, the given track is unselected.
    Error_Type_e selectTrack(size_t trackIndex, bool select);

    size_t countTracks() const;

    SigmaMediaPlayerImpl(const SigmaMediaPlayerImpl &);
    SigmaMediaPlayerImpl &operator=(const SigmaMediaPlayerImpl &);
};

#endif
