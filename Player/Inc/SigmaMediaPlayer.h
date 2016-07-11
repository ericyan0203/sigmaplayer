#ifndef SIGMA_MEDIA_PLAYER_H
#define SIGMA_MEDIA_PLAYER_H

#include "ISigmaPlayer.h"
#include "SigmaMediaPlayerImpl.h"

enum media_event_type {
    MEDIA_NOP               = 0, // interface test message
    MEDIA_PLAYBACK_COMPLETE = 1,
    MEDIA_BUFFERING_UPDATE  = 2,
    MEDIA_SEEK_COMPLETE     = 4,
    MEDIA_STARTED           = 6,
    MEDIA_PAUSED            = 7,
    MEDIA_STOPPED           = 8,
    MEDIA_ERROR             = 100,
    MEDIA_INFO              = 200,
    MEDIA_SUBTITLE_DATA     = 201
};

struct SigmaMediaPlayer : public ISigmaPlayer {
    SigmaMediaPlayer();
	SigmaMediaPlayer(const char * ip, uint32_t port);
	virtual ~SigmaMediaPlayer();
#if 0
    virtual Error_Type_e setUID(uid_t uid);
#endif

    virtual Error_Type_e setDataSource(const char *url);

    virtual Error_Type_e prepare();
    virtual Error_Type_e start();
    virtual Error_Type_e stop();
    virtual Error_Type_e pause();
    virtual bool isPlaying();
    virtual Error_Type_e seekTo(int msec);
    virtual Error_Type_e reset();
    virtual Error_Type_e setParameter(int key, const void * request);
    virtual Error_Type_e getParameter(int key, void *reply);
	virtual Error_Type_e sendEvent(int msg, int ext1=0, int ext2=0);
	
private:
    SigmaMediaPlayerImpl *mPlayer;

	Mutex               mNotifyLock;
	
    SigmaMediaPlayer(const SigmaMediaPlayer &); 
    SigmaMediaPlayer &operator=(const SigmaMediaPlayer &);

};

#endif

