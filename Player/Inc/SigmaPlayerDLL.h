#ifndef _SIGMA_PLAYER_DLL_H
#define _SIGMA_PLAYER_DLL_H

typedef void (*callback_t)(void* cookie, int msg, int ext1, int ext2, unsigned int *obj);

typedef enum video_format {
	VIDEO_CodingMPEG12 = 1,
    VIDEO_CodingDIX3,
    VIDEO_CodingMPEG4,
    VIDEO_CodingVC1,
    VIDEO_CodingRV,
    VIDEO_CodingAVC,
    VIDEO_CodingMVC,
    VIDEO_CodingAVS,
    VIDEO_CodingJPEG,
    VIDEO_CodingVP6,
    VIDEO_CodingVP8,  /**< Google VP8, formerly known as On2 VP8 */
    VIDEO_CodingVP9,  /**< Google VP9 */
    VIDEO_CodingHEVC, /**< HEVC */
}video_format_t;

typedef enum audio_format {
	AUDIO_CodingAC3 = 1,
    AUDIO_CodingMPEG,
    AUDIO_CodingAAC,
    AUDIO_CodingMP3,
    AUDIO_CodingAC3Plus,
    AUDIO_CodingPCM,
    AUDIO_CodingWMA,
    AUDIO_CodingWAV,
    AUDIO_CodingAUX,
    AUDIO_CodingVORBIS,
    AUDIO_CodingAIFC,
    AUDIO_CodingRACOOK,
    AUDIO_CodingDTS,
    AUDIO_CodingALAC,
    AUDIO_CodingRealAudio,
    AUDIO_CodingDRA,
    AUDIO_CodingAMRNB,
    AUDIO_CodingAMRWB,
    AUDIO_CodingSILK,
    AUDIO_CodingHEAAC,
    AUDIO_CodingFLAC,
    AUDIO_CodingHEAAC_V2,
    AUDIO_CodingLPCM,
    AUDIO_CodingWMAPRO,
    AUDIO_CodingOPUS,
    AUDIO_ENCODER_CodingMP3,
    AUDIO_ENCODER_CodingSILK,
    AUDIO_ENCODER_CodingSBC,
    AUDIO_HDMI_CodingAC3,
    AUDIO_HDMI_CodingPCM,
}audio_format_t;

typedef enum display_port {
	DISPLAY_SOURCE_HDMI1 = 2,
    DISPLAY_SOURCE_HDMI2 = 3,
    DISPLAY_SOURCE_HDMI3 = 4,
    DISPLAY_SOURCE_HDMI4 = 5,
} display_port_t;

typedef struct channel_config {
   unsigned int video_pid;
   unsigned int video_format;
   unsigned int audio_pid;
   unsigned int audio_format;
   unsigned int pcr_pid;
}channel_config_t;

typedef enum tunner_type {
    DEMOD_TYPE_INVALID,
    DEMOD_TYPE_ATSC,
    DEMOD_TYPE_DVBC,
    DEMOD_TYPE_DVBT,
    DEMOD_TYPE_MAX
}tuner_type_t;

extern "C"  int __declspec(dllexport)halsys_player_init(const char * ip, const int port);

extern "C"  int __declspec(dllexport)halsys_player_deinit();

extern "C"  int  __declspec(dllexport)halsys_media_player_create(const char * url ,void** phandle);

extern "C"  int  __declspec(dllexport)halsys_media_player_start(void * phandle);

extern "C"  int  __declspec(dllexport)halsys_media_player_stop(void * phandle);

extern "C"  int  __declspec(dllexport)halsys_media_player_destroy(void * phandle);

extern "C"  int  __declspec(dllexport)halsys_media_player_pause(void * phandle);

extern "C"  int  __declspec(dllexport)halsys_media_player_resume(void * phandle);

extern "C"  int  __declspec(dllexport)halsys_media_player_seek(void * phandle,unsigned long long ms);

extern "C"  int  __declspec(dllexport)halsys_media_player_getduration(void * phandle,unsigned long long * duration);

extern "C"  int  __declspec(dllexport) halsys_media_player_getcurrtime(void * phandle,unsigned long long * pCurrTime);

extern "C" int  __declspec(dllexport)halsys_media_player_installcb(void * phandle,callback_t cb);

extern "C" int __declspec(dllexport)halsys_hdmi_player_create(void** phandle);
	
extern "C" int __declspec(dllexport)halsys_hdmi_player_start(void * phandle,display_port_t source,void ** port);

extern "C" int __declspec(dllexport)halsys_hdmi_player_stop(void * port);

extern "C" int __declspec(dllexport)halsys_hdmi_player_destroy(void * phandle);

extern "C" int __declspec(dllexport)halsys_dtv_player_create(void** phandle);

extern "C" int __declspec(dllexport)halsys_dtv_player_start(void * phandle, channel_config_t * config, void ** channel); 

extern "C" int __declspec(dllexport)halsys_dtv_player_stop(void * channel);

extern "C" int __declspec(dllexport)halsys_dtv_player_destroy(void * phandle);

extern "C" int __declspec(dllexport)halsys_tuner_lock(tuner_type_t stackyype, int bandwidth, int symbolrate, int freqkhz);

#endif
