#ifndef MEDIA_SOURCE_H_

#define MEDIA_SOURCE_H_

#include <sys/types.h>

#include <RefBase.h>
#include <Vector.h>

#include "SIGM_Types.h"
#include "MetaData.h"
#include "StrongPointer.h"
#include "MediaBuffer.h"
#include "Threads.h"
#include "Listener.h"

struct MediaSource : public virtual RefBase ,public virtual Thread {
    MediaSource();

    // To be called before any other methods on this object, except
    // getFormat().
    virtual Error_Type_e start(MetaData *params = NULL) = 0;

    // Any blocking read call returns immediately with a result of NO_INIT.
    // It is an error to call any methods other than start after this call
    // returns. Any buffers the object may be holding onto at the time of
    // the stop() call are released.
    // Also, it is imperative that any buffers output by this object and
    // held onto by callers be released before a call to stop() !!!
    virtual Error_Type_e stop() = 0;

    // Returns the format of the data output by this media source.
    virtual sp<MetaData> getFormat() = 0;

    struct ReadOptions;

    // Returns a new buffer of data. Call blocks until a
    // buffer is available, an error is encountered of the end of the stream
    // is reached.
    // End of stream is signalled by a result of ERROR_END_OF_STREAM.
    // A result of INFO_FORMAT_CHANGED indicates that the format of this
    // MediaSource has changed mid-stream, the client can continue reading
    // but should be prepared for buffers of the new configuration.
    virtual Error_Type_e read(
            MediaBuffer **buffer, const ReadOptions *options = NULL) = 0;

    // Options that modify read() behaviour. The default is to
    // a) not request a seek
    // b) not be late, i.e. lateness_us = 0
    struct ReadOptions {
        enum SeekMode {
            SEEK_PREVIOUS_SYNC,
            SEEK_NEXT_SYNC,
            SEEK_CLOSEST_SYNC,
            SEEK_CLOSEST,
        };

        ReadOptions();

        // Reset everything back to defaults.
        void reset();

        void setSeekTo(int64_t time_us, SeekMode mode = SEEK_CLOSEST_SYNC);
        void clearSeekTo();
        bool getSeekTo(int64_t *time_us, SeekMode *mode) const;

    private:
        enum Options {
            kSeekTo_Option      = 1,
        };

        uint32_t mOptions;
        int64_t mSeekTimeUs;
        SeekMode mSeekMode;
    };

	virtual void setListener(const wp<Listener> &listener) {
		mListener = listener;
	}

    // Causes this source to suspend pulling data from its upstream source
    // until a subsequent read-with-seek. Currently only supported by
    // OMXCodec.
    virtual Error_Type_e pause() {
        return SIGM_ErrorNotSupported;
    }

	virtual Error_Type_e resume() {
        return SIGM_ErrorNotSupported;
    }

	virtual Error_Type_e  seekTo(uint64_t timeMS) {
        return SIGM_ErrorNotSupported;
    }

    // The consumer of this media source requests that the given buffers
    // are to be returned exclusively in response to read calls.
    // This will be called after a successful start() and before the
    // first read() call.
    // Callee assumes ownership of the buffers if no error is returned.
    virtual status_t setBuffers(const Vector<MediaBuffer *> & /* buffers */) {
        return SIGM_ErrorNotSupported;
    }

protected:
    virtual ~MediaSource();
	wp<Listener> mListener;

private:
    MediaSource(const MediaSource &);
    MediaSource &operator=(const MediaSource &);
	
};


#endif  // MEDIA_SOURCE_H_

