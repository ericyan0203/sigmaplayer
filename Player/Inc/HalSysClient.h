#ifndef _HALSYS_CLIENT_
#define _HALSYS_CLIENT_

#include <Refbase.h>
#include <String8.h>
#include <Mutex.h>
#include <Listener.h>

#include "SIGM_Types.h"
#include "SIGM_Media_API.h"

typedef enum {
	MEDIA = 0,
	DTV,
	HDMI,
}HalSysType;

class HalSysClient: public virtual RefBase {
public:
   	HalSysClient(HalSysType eType);
    virtual ~HalSysClient();

	Error_Type_e connect();
	Error_Type_e disconnect();
    Error_Type_e init();
    Error_Type_e deinit();
	Error_Type_e open(Video_CodingType_e video_format, Audio_CodingType_e audio_format);
	Error_Type_e close();
	Error_Type_e start();
	Error_Type_e stop();
	Error_Type_e pause();
	Error_Type_e resume();
	Error_Type_e flush(int64_t timeMs);

	Error_Type_e handleBuffer(Media_Buffer_t *buffer);

	void HalSysClient::setListener(const wp<Listener> &listener);
	void HalSysClient::notifyListener_l(int msg, int ext1, int ext2 ,unsigned int * obj);
	
private:
    HalSysClient(const HalSysClient &);
    HalSysClient &operator=(const HalSysClient &);

	HalSysType mType;
	Video_CodingType_e mVideoFormat;
	Audio_CodingType_e mAudioFormat;

	mutable Mutex mLock;
	mutable Mutex mCallbackLock;
    
	wp<Listener> mListener;
	sigma_handle_t mHandle;

	CallBackFuncsInfo_t  tEOSCbInfo;
	
};
#endif
