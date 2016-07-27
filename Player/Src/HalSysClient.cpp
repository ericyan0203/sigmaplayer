#include "HalSysClient.h"
#include "SIGM_API.h"
#include "SIGM_Media_API.h"

static bool bGlbInit = false;  //we should use singleton here
static uint32_t bRef = 0;

#define SOUND_BIN_PATH "/opt/Misc/sound_preset.bin"
#define AUDIO_PATH_XML "/opt/Misc/audiopath_cfg.xml"
#define DISPLAY_CONFIG_PATH "/opt/"

static Error_Type_e sigma_test_init(void) {
    BoardConfig_t tBoardConfig;
    Error_Type_e ret = SIGM_ErrorNone;

    memset(&tBoardConfig, 0, sizeof(BoardConfig_t));

    tBoardConfig.pSoundBin = SOUND_BIN_PATH;
    tBoardConfig.pAudioPathXML = AUDIO_PATH_XML;
    tBoardConfig.pDisplayFilePath = DISPLAY_CONFIG_PATH;
    ret = HalSys_Initialize(&tBoardConfig);
    return ret;
}

static Error_Type_e sigma_test_deinit(void) { 
		return HalSys_Deinit(); 
}

HalSysClient::HalSysClient(HalSysType eType)
		:mType(eType){

		mHandle = (sigma_handle_t)-1;
}

HalSysClient::~HalSysClient() {

}

Error_Type_e HalSysClient::connect(){

	Mutex::Autolock autoLock(mLock);
	
	if(bGlbInit == true){
		bRef++;
		return SIGM_ErrorNone;
	}

    return sigma_test_init();
}

Error_Type_e HalSysClient::disconnect() {
	Mutex::Autolock autoLock(mLock);
	
	if(bGlbInit == false) 
		return SIGM_ErrorInvalidState;

	bRef--;

	if(bRef == 0){
		sigma_test_deinit();
	}

	return SIGM_ErrorNone;
}

Error_Type_e HalSysClient::init() {
	Mutex::Autolock autoLock(mLock);
	return HalSys_Media_Initialize();
}

Error_Type_e HalSysClient::open(Video_CodingType_e video_format, Audio_CodingType_e audio_format) {
	Media_Config_t tMediaConfig;
	Mutex::Autolock autoLock(mLock);

	
	mVideoFormat = video_format;
	mAudioFormat = audio_format;

	tMediaConfig.bLowLatency = trid_false;
    tMediaConfig.eClockMode = CLOCK_MODE_TIMER;

    tMediaConfig.tAudioConfig.eAudioFormat = mAudioFormat;
    tMediaConfig.tAudioConfig.eSoundSink = (audio_format == SIGM_AUDIO_CodingUnused)?0:(SIGM_SOUND_SINK_HEADPHONE | SIGM_SOUND_SINK_SPDIF_PCM | SIGM_SOUND_SINK_SPEAKER);

    tMediaConfig.tVideoConfig.eVideoDisplayMode = SIGM_SEAMLESS_NONE;
    tMediaConfig.tVideoConfig.eVideoFormat = mVideoFormat ;
    tMediaConfig.tVideoConfig.eVideoPlayMode = SIGM_PLAYMODE_NORMAL;
    tMediaConfig.tVideoConfig.eVideoStreamMode = SIGM_STREAMMODE_NORMAL;
    tMediaConfig.tVideoConfig.eVideoSink = (Video_Sink_e)((video_format == SIGM_VIDEO_CodingUnused)?0:SIGM_VIDEO_SINK_MP);
	tMediaConfig.tVideoConfig.eMuxType = SIGM_MUX_ES;

	return  HalSys_Media_Open(&tMediaConfig, &mHandle);
}

Error_Type_e HalSysClient::close() {
	Error_Type_e ret = SIGM_ErrorFailed;
	Mutex::Autolock autoLock(mLock);

	if((sigma_handle_t)-1 != mHandle) {
		ret = HalSys_Media_Close(mHandle);
		mHandle = (sigma_handle_t)-1;
	}
	return ret;
}

Error_Type_e HalSysClient::start(){
	Error_Type_e ret = SIGM_ErrorFailed;
	Mutex::Autolock autoLock(mLock);

	if((sigma_handle_t)-1 != mHandle) {
		ret = HalSys_Media_Start(mHandle);
	}
	return ret;
}

Error_Type_e HalSysClient::stop() {
	Error_Type_e ret = SIGM_ErrorFailed;
	Mutex::Autolock autoLock(mLock);

	if( (sigma_handle_t)-1 != mHandle) {
		ret = HalSys_Media_Stop(mHandle);
	}
	return ret;
}

Error_Type_e HalSysClient::pause() {
	Error_Type_e ret = SIGM_ErrorFailed;
	Mutex::Autolock autoLock(mLock);

	if( (sigma_handle_t)-1 != mHandle) {
		ret = HalSys_Media_Pause(mHandle);
	}
	return ret;
}

Error_Type_e HalSysClient::resume() {
	Error_Type_e ret = SIGM_ErrorFailed;
	Mutex::Autolock autoLock(mLock);

	if( (sigma_handle_t)-1 != mHandle) {
		ret = HalSys_Media_Resume(mHandle);
	}
	return ret;
}

Error_Type_e HalSysClient::handleBuffer(Media_Buffer_t *buffer){	
	Error_Type_e ret = SIGM_ErrorFailed;
	Mutex::Autolock autoLock(mLock);
	if( (sigma_handle_t)-1 != mHandle) {
		ret = HalSys_Media_PushFrame(mHandle, buffer);
	}
	return ret;
}

