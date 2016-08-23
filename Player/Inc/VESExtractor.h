#ifndef _ES_EXTRACTOR_H_

#define _ES_EXTRACTOR_H_

#include "MediaExtractor.h"
#include "MediaBuffer.h"
#include "MediaSource.h"
#include "SigmaMediaPlayerImpl.h"

#include <Vector.h>
#include <List.h>

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <Threads.h>

#include "SIGM_Types.h"

#define INITIAL_STREAM_BUF_SIZE 512 * 1024
#define DEBUGFILE "./haltest.es"

class String8;

struct VideoESSource;

struct VideoESExtractor : public MediaExtractor {
		VideoESExtractor(const sp<DataSource> &source);

		virtual size_t countTracks();

		virtual sp<MediaSource> getTrack(size_t index);

		virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);

		virtual sp<MetaData> getMetaData();

		virtual void setTrackActive(int trackIndex, bool enable);

		virtual MediaBuffer * getNextEncFrame(int trackIndex,int64_t seekTime, int seekMode);

		virtual void seekTo(int trackIndex,int64_t time_us);
		
		protected:
			virtual ~VideoESExtractor();

		private:
			Mutex mLock;
		
			friend struct VideoESSource;

			bool mEof; // EOF reached? 
			int mCurrentOffset;
			FILE * mFp;

			struct TrackInfo {
					unsigned long mTrackNum; 
					sp<MetaData> mMeta;
				};

			Vector<TrackInfo> mTracks;
			
			sp<DataSource> mDataSource;
			
			int addTracks();

			VideoESExtractor(const VideoESExtractor &);
			VideoESExtractor &operator=(const VideoESExtractor &);
};


struct VideoESSource : public MediaSource {
		VideoESSource(
						const sp<VideoESExtractor> &extractor,  size_t trackindex);

		virtual Error_Type_e start(MetaData *params);
		virtual Error_Type_e stop();

		virtual sp<MetaData> getFormat();

		virtual Error_Type_e read(
						MediaBuffer **buffer, const ReadOptions *options);

		virtual bool threadLoop();

		virtual int  requestExitAndWait();

		virtual Error_Type_e pause();

		virtual Error_Type_e resume();

		virtual Error_Type_e seekTo(uint64_t timeMS);

		private:
		enum Status{
			NONE              = 0,
			PAUSE_PENDING     = 1,
			PAUSED            = 2,    
			RESUME_PENDING    = 4,
			RESUMED           = 8,
			EOS               = 16,
			INVALID
		};
		
		sp<VideoESExtractor> mExtractor;
		size_t mTrackIndex;
		
		MediaBuffer *mBuffer;
		bool bEOS;

		Video_CodingType_e eType;
#ifdef DEBUGFILE
		FILE * mFile;
#endif
		mutable Mutex mLock;
		uint32_t  mStatus;
		Condition mCondition;
	
		MediaSource::ReadOptions mOptions;

		VideoESSource(const VideoESSource &);
		VideoESSource &operator=(const VideoESSource &);
};

bool SniffVideoES(
			const sp<DataSource> &source, String8 *mimeType, float *confidence);


#endif  // VideoES_EXTRACTOR_H_
