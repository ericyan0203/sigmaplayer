#include  "SigmaPlayerDLL.h"
#include  "SigmaMediaPlayer.h"
#include  <StrongPointer.h>
#include <String8.h>
#include  <Utils.h>
#include  <string.h>
#include  <stdio.h>
#include  <utils_socket.h>
#include "SIGM_Utils.h"
#include "SIGM_API.h"
#include "SIGM_Media_API.h"
#include "SIGM_DigitalTV_API.h"
#include "SIGM_HDMI_API.h"
#include  "ISigmaPlayer.h"

#define SOUND_BIN_PATH "/opt/Misc/sound_preset.bin"
#define AUDIO_PATH_XML "/opt/Misc/audiopath_cfg.xml"
#define DISPLAY_CONFIG_PATH "/opt/"

#if 0
#define FILE_PATH 	"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116
#endif

sp<SigmaMediaPlayer> player = NULL;

int mPort = 52116;
char mIP[256]= {0};

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


int halsys_player_init(const char * ip, const int port){
	utils_init("./sigma.log");
	Utils_InitInfoArray();
	
	strncpy(mIP,ip,10);
	mPort = port;

	socket_connect(ip,port, 3000);
	socket_setListener((Observer)Utils_InvokeCallback);
	client_server_start();
	utils_log(AV_DUMP_ERROR,"halsys_player_init\n");
	return 0;
}

int halsys_player_deinit() {
	utils_deinit();
	Utils_DeinitInfoArray();
	socket_disconnect();
	client_server_stop();
	//utils_log(AV_DUMP_ERROR,"halsys_player_deinit\n");
	return 0;
}


int  halsys_media_player_create(const char * url,void** phandle){
	 player = new SigmaMediaPlayer();
	 player->setDataSource(url);

	 *phandle = (void *)player.get();
	 utils_log(AV_DUMP_ERROR,"halsys_media_player_create\n");
	 return 0;
}

int halsys_media_player_start(void * phandle) {
	if(phandle == player.get()) {
	 	player->start();
	}else {
		utils_log(AV_DUMP_ERROR,"handle pointer isn't correct\n");
		return -1;
	}

	utils_log(AV_DUMP_ERROR,"halsys_media_player_start\n");
	return 0;
}

int halsys_media_player_stop(void * phandle) {
	if(phandle == player.get()) {
	 	player->stop();
	}else {
		utils_log(AV_DUMP_ERROR,"handle pointer isn't correct\n");
		return -1;
	}
	utils_log(AV_DUMP_ERROR,"halsys_media_player_stop\n");
	return 0;
}

int  halsys_media_player_destroy(void * phandle) {
	if(phandle == player.get()) {
	 	player.clear();
	}else {
		utils_log(AV_DUMP_ERROR,"handle pointer isn't correct\n");
		return -1;
	}
	utils_log(AV_DUMP_ERROR,"halsys_media_player_destroy\n");

	 return 0;
}


int  halsys_media_player_pause(void * phandle) {
	if(phandle == player.get()) {
	 	player->pause();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}

int  halsys_media_player_resume(void * phandle) {
	if(phandle == player.get()) {
	 	player->resume();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}


int halsys_media_player_seek(void * phandle,unsigned long long ms) {
	if(phandle == player.get()) {
	 	player->seekTo(ms);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}

int halsys_media_player_getduration(void * phandle,unsigned long long * duration) {
	if(phandle == player.get()) {
	 	player->getParameter(MEDIA_DURATION,(void *)duration);
		utils_log(AV_DUMP_ERROR,"Duration %lld\n",*duration);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}
	 
	 return 0;
}

int halsys_media_player_getcurrtime(void * phandle,unsigned long long * pCurrTime) {
	if(phandle == player.get()) {
	 	player->getParameter(MEDIA_CURRENTTIME,(void *)pCurrTime);
		utils_log(AV_DUMP_ERROR,"curtime %lld\n",*pCurrTime);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}
	 
	 return 0;
	
}
int halsys_media_player_installcb(void * phandle,callback_t cb)
{
	if(phandle == player.get()) {
	 	player->setNotifyCallback(phandle,(notify_callback_t)cb);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}
	 
	 return 0;
}

int halsys_hdmi_player_create(void** phandle){
	Error_Type_e ret = SIGM_ErrorNone;
    HDMI_Config_t tConfig;
	tConfig.bFrc = trid_false;
	sigma_handle_t pInstance;

    sigma_test_init(); //don't check the return value

	ret = HalSys_HDMI_Open(&tConfig, &pInstance);

	if(SIGM_ErrorNone == ret) {
		*phandle = (void *)pInstance;
	}else {
		*phandle = (void *)-1;
	}

	return ret;
}

int halsys_hdmi_player_start(void * phandle,display_port_t source,void ** port) {
	HDMI_PortConfig_t tConfig;
	sigma_handle_t handle;
	Error_Type_e ret = SIGM_ErrorNone;

	tConfig.eHDMIInput = (Display_Source_e)source;
    tConfig.eVideoSink = SIGM_VIDEO_SINK_MP;
    tConfig.nAudioSink = SIGM_SOUND_SINK_HEADPHONE | SIGM_SOUND_SINK_SPDIF_PCM | SIGM_SOUND_SINK_SPEAKER;
    ret = HalSys_HDMI_Start(phandle, &tConfig, &handle);

	if(SIGM_ErrorNone == ret) {
		*port = (void *)handle;
	}else {
		*port = (void *)-1;
	}

	return ret;
}

int halsys_hdmi_player_stop(void * port) {
	return HalSys_HDMI_Stop(port); 
}

int  halsys_hdmi_player_destroy(void * phandle) {
	 return HalSys_HDMI_Close(phandle);
}

int  halsys_dtv_player_create(void** phandle){
	Error_Type_e ret = SIGM_ErrorNone;
    Demux_Config_t tConfig;
	sigma_handle_t pInstance;

    sigma_test_init(); //don't check the return value

	tConfig.eDemuxCIPath = SIGM_CI_PATH_BYPASS;
	tConfig.eDemuxInput = SIGM_INPUT_INTERNAL_SERIAL_DEMOD_0;
	ret = HalSys_DigitalTV_Open(&tConfig, &pInstance);

	if(SIGM_ErrorNone == ret) {
		*phandle = (void *)pInstance;
	}else {
		*phandle = (void *)-1;
	}

	return ret;
}

int halsys_dtv_player_start(void * phandle, channel_config_t * config, void ** channel) {
	Error_Type_e ret = SIGM_ErrorNone;
    Channel_Config_t tConfig;
	sigma_handle_t pChannel;
	
	tConfig.tVideoConfig.nVideoPID = config->video_pid;
	tConfig.tVideoConfig.eVideoFormat = (Video_CodingType_e)config->video_format;
	tConfig.tVideoConfig.eVideoSink = SIGM_VIDEO_SINK_MP;
	tConfig.tVideoConfig.eVideoDisplayMode = SIGM_SEAMLESS_NONE;
	tConfig.tVideoConfig.eVideoStreamMode = SIGM_STREAMMODE_NORMAL;

	tConfig.tAudioConfig.nAudioPID = config->audio_pid;
	tConfig.tAudioConfig.eAudioFormat = (Audio_CodingType_e)config->audio_format;
	tConfig.tAudioConfig.eSoundSink = SIGM_SOUND_SINK_HEADPHONE | SIGM_SOUND_SINK_SPDIF_PCM | SIGM_SOUND_SINK_SPEAKER;

	tConfig.bLowlatency = false;
	tConfig.nPcrPID = config->pcr_pid;
	tConfig.tSubtitleConfig.nPID = -1;
	tConfig.tTeletextConfig.nPID = -1;
	
	ret = HalSys_DigitalTV_Start(phandle,&tConfig,&pChannel);

	if(SIGM_ErrorNone == ret) {
		*channel = (void *)pChannel;
	}else {
		*channel = (void *)-1;
	}

	return ret;
}

int halsys_dtv_player_stop(void * channel) {
	return HalSys_DigitalTV_Stop(channel);
}

int halsys_dtv_player_destroy(void * phandle) {
	return HalSys_DigitalTV_Close(phandle);
}

int halsys_tuner_lock(int stacktype, int bandwidth, int symbolrate, int freqkhz)
{
	return 0;
}