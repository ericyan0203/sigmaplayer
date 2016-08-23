#ifndef SIGMA_IPLAYER_H
#define SIGMA_IPLAYER_H

#include <RefBase.h>
#include <KeyedVector.h>
#include <Mutex.h>
#include <Listener.h>

#include "SIGM_Types.h"

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

enum media_parameter_key {
	MEDIA_PARAMETER_NOP = 0,
	MEDIA_DURATION,
	MEDIA_CURRENTTIME,
};

class ISigmaPlayer: /*public VirtualLightRefBase*/public virtual RefBase {
public:
    ISigmaPlayer(){};                                                     \
    virtual ~ISigmaPlayer(){}; 

    virtual Error_Type_e        setDataSource(const char *url) = 0;

    virtual Error_Type_e        prepare() = 0;
    virtual Error_Type_e        start() = 0;
    virtual Error_Type_e        stop() = 0;
    virtual Error_Type_e        pause() = 0;
	virtual Error_Type_e        resume() = 0;
	virtual bool        isPlaying() = 0;
    virtual Error_Type_e        seekTo(uint64_t msec) = 0;
	
	virtual Error_Type_e        reset() = 0;

    virtual Error_Type_e        setParameter(int key, const void * request) = 0;
    virtual Error_Type_e        getParameter(int key, void * reply) = 0;
};

#endif
