#include <stdio.h>

#include "Metadata.h"
#include "MediaExtractor.h"
#include "SigmaMediaPlayer.h"

SigmaMediaPlayer::SigmaMediaPlayer()
    : mPlayer(new SigmaMediaPlayerImpl) {
     mPlayer->setListener(this);
}


SigmaMediaPlayer::SigmaMediaPlayer(const char * ip, uint32_t port) 
	:mPlayer(new SigmaMediaPlayerImpl(ip,port)){
	
}	

SigmaMediaPlayer::~SigmaMediaPlayer() {
    
    reset();

    delete mPlayer;
    mPlayer = NULL;
}

#if 0
Error_Type_e SigmaMediaPlayer::setUID(uid_t uid) {
    mPlayer->setUID(uid);

    return SIGM_ErrorNone;
}
#endif
Error_Type_e SigmaMediaPlayer::setDataSource(const char *url) {
    return mPlayer->setDataSource(url);
}

Error_Type_e SigmaMediaPlayer::prepare() {
    return mPlayer->prepare();
}

Error_Type_e SigmaMediaPlayer::start() {
    return mPlayer->play();
}

Error_Type_e SigmaMediaPlayer::stop() {
    return mPlayer->stop();; 
}

Error_Type_e SigmaMediaPlayer::pause() {
    return mPlayer->pause();
}

Error_Type_e SigmaMediaPlayer::resume() {
	return mPlayer->resume();
}

bool SigmaMediaPlayer::isPlaying() {
    return mPlayer->isPlaying();
}

Error_Type_e SigmaMediaPlayer::seekTo(int msec) {
    Error_Type_e err = mPlayer->seekTo((int64_t)msec * 1000);
    return err;
}

Error_Type_e SigmaMediaPlayer::reset() {
    mPlayer->reset();
	return SIGM_ErrorNone;
}


Error_Type_e SigmaMediaPlayer::setParameter(int key, const void * request) {
    printf("setParameter(key=%d)\n", key);
    return mPlayer->setParameter(key, request);
}

Error_Type_e SigmaMediaPlayer::getParameter(int key, void *reply) {
    printf("getParameter\n");
    return mPlayer->getParameter(key, reply);
}

Error_Type_e SigmaMediaPlayer::sendEvent(int msg, int ext1, int ext2) {
	Mutex::Autolock autoLock(mNotifyLock);
	Error_Type_e ret = SIGM_ErrorNone;
	switch(msg)
	{
		case MEDIA_BUFFERING_UPDATE:
			ret = mPlayer->sendEvent(msg,ext1,ext2);
			break;
	}
	return ret;
}

