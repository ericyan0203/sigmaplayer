#ifndef _HALSYS_SERVER_H
#define _HALSYS_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//***************************************************************
//
// Macro Define
//
//***************************************************************

#define HALSYS_NAME_LENGTH 128

#define PATHLEN 128

//***************************************************************
//
// Data Type Define
//
//***************************************************************

/** struct HALSYS_COMMON_PARAMETER
 *    common parameter
 *    '1I'
 */
#pragma pack(push)  
#pragma pack(1) 
typedef struct HALSYS_COMMON_PARAMETER { 
	unsigned int instance; 
} halsys_common_param;
#pragma pack(pop)

/** struct HALSYS_COMMON_RETURN
 *  	common return type
 *   '1I'
 */
#pragma pack(push)  
#pragma pack(1) 
typedef struct HALSYS_COMMON_RETURN { 
	unsigned int ret; 
}halsys_common_ret;
#pragma pack(pop)

/** struct HALSYS_COMMAND_PARAMETER
 *  Init Parameter for HalSys_Initialize
 *  '128s128s128s'
 */
#pragma pack(push)  
#pragma pack(1) 
typedef struct HALSYS_INIT_PARAMETER {
    char sound_path[PATHLEN];
    char audio_path[PATHLEN];
    char display_path[PATHLEN];
} halsys_init_param;
#pragma pack(pop)

/** struct HALSYS_CALLBACK_PARAMETER
 *  halsys install callback parameter
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct HALSYS_CALLBACK_PARAMETER {
	unsigned int instance;
	unsigned int reason;
	unsigned int subid;
}halsys_cb_param;
#pragma pack(pop)


/** struct MEDIA_OPEN_PARAMETER
 *  media open command parameter
 *  10B
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct MEDIA_OPEN_PARAMETER {
    unsigned char video_format;
    unsigned char video_playback_mode;
    unsigned char video_seamless_mode;
    unsigned char video_stream_mode;
    unsigned char video_sink;
    unsigned char video_mux;
    unsigned char audio_format;
    unsigned char audio_sink;
    unsigned char clock_mode;
    unsigned char lowlatency;
} media_open_param;
#pragma pack(pop)

/** struct HALSYS_OPEN_RETURN
 *   media open command return value
 *   '2I'
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct HALSYS_OPEN_RETURN {
    unsigned int ret;
    unsigned int instance;
} halsys_open_ret;
#pragma pack(pop)

/** struct HALSYS_OPEN_RETURN
 *   media open command return value
 *   '2I'
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct HALSYS_CALLBACK_RETURN {
    unsigned int ret;
    unsigned int subid;
} halsys_cb_ret;
#pragma pack(pop)


/** struct MEDIA_FLUSH_PARAMETER
 *  media flush command parameter
 *  '1I1B'
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct MEDIA_FLUSH_PARAMETER {
    unsigned int instance;
    unsigned int flush_mode;
	unsigned long long timestamp;
} media_flush_param;
#pragma pack(pop)

/** struct MEDIA_PUSH_PARAMETER
 *  media push command parameter
 * header '5I1Q1I'
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct MEDIA_PUSH_PARAMETER {
    unsigned int instance;
    unsigned int size;
    unsigned int alloc_size;
    unsigned int fill_size;
    unsigned int offset;
    unsigned long long pts;
    unsigned int flags;
    unsigned char data[0];
} media_push_param;
#pragma pack(pop)

/** struct HDMI_OPEN_PARAMETER
 *  	HDMI open command parameter
 *   '1I'
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct HDMI_OPEN_PARAMETER { 
	unsigned int bFrc; 
}hdmi_open_param;
#pragma pack(pop)

/** struct HDMI_START_PARAMETER
 *   HDMI start command parameter
 *   '4I'
 */
#pragma pack(push)  
#pragma pack(1)
typedef struct HDMI_START_PARAMETER {
    unsigned int instance;
    unsigned int display_source;
    unsigned int audio_sink;
    unsigned int video_sink;
}hdmi_start_param;
#pragma pack(pop)

#pragma pack(push)  
#pragma pack(1)
typedef struct DTV_OPEN_PARAMETER { 
	unsigned int demux_input;
	unsigned int demux_cipath;
}dtv_open_param;
#pragma pack(pop)

#pragma pack(push)  
#pragma pack(1)
typedef struct DTV_START_PARAMETER {
    unsigned int instance;
    unsigned int video_pid;
    unsigned int video_format;
    unsigned int video_sink;
    unsigned int video_display_mode;
    unsigned int video_stream_mode;
    unsigned int audio_pid;
    unsigned int audio_format;
    unsigned int audio_sink;
    unsigned int subtitle_pid;
    unsigned int teletext_pid;
    unsigned int pcr_pid;
    unsigned int low_latency;
}dtv_start_param;
#pragma pack(pop)

#pragma pack(push)  
#pragma pack(1)
typedef struct HALSYS_RETURN {
    unsigned int size;
    union {
		halsys_common_ret common_ret;
		halsys_open_ret open_ret;
		halsys_cb_ret  	cb_ret;	
    };
} halsys_ret;
#pragma pack(pop)

#pragma pack(push)  
#pragma pack(1)
typedef struct {
    unsigned int length;
    char domain_name[HALSYS_NAME_LENGTH];
} halsys_packet_header;
#pragma pack(pop)

#pragma pack(push)  
#pragma pack(1)
typedef struct {
    char func_name[HALSYS_NAME_LENGTH];
    unsigned int param_len;
    char data[0];
} halsys_packet_data;
#pragma pack(pop)

#pragma pack(push)  
#pragma pack(1) 
typedef struct HALMISC_TUNER_PARAMETER { 
	unsigned int demod_type;
	unsigned int bandwidth;
	unsigned int symbolrate;
	unsigned int frequence;
} halmisc_tuner_param;
#pragma pack(pop)

//*************************************************************
// Function
//*************************************************************
halsys_ret halsys_initialize(halsys_init_param * arg);

halsys_ret halsys_deinit(void* arg);

halsys_ret halsys_media_initialize(void);

halsys_ret halsys_media_deinit(void);

halsys_ret halsys_media_open(media_open_param* arg);

halsys_ret halsys_media_close(halsys_common_param* arg);

halsys_ret halsys_media_start(halsys_common_param* arg);

halsys_ret halsys_media_stop(halsys_common_param* arg);

halsys_ret halsys_media_pause(halsys_common_param* arg);

halsys_ret halsys_media_resume(halsys_common_param* arg);

halsys_ret halsys_media_flush(media_flush_param* arg);

halsys_ret halsys_media_pushframe(media_push_param* arg);

halsys_ret halsys_media_installcb(halsys_cb_param * arg);

halsys_ret halsys_media_uninstallcb(halsys_cb_param * arg);

halsys_ret halsys_hdmi_initialize(void);

halsys_ret halsys_hdmi_deinit(void);

halsys_ret halsys_hdmi_open(hdmi_open_param* arg);

halsys_ret halsys_hdmi_close(halsys_common_param* arg);

halsys_ret halsys_hdmi_start(hdmi_start_param* arg);

halsys_ret halsys_hdmi_stop(halsys_common_param* arg);

halsys_ret halsys_dtv_initialize(void);

halsys_ret halsys_dtv_deinit(void);

halsys_ret halsys_dtv_open(dtv_open_param* arg);

halsys_ret halsys_dtv_close(halsys_common_param* arg);

halsys_ret halsys_dtv_start(dtv_start_param* arg);

halsys_ret halsys_dtv_stop(halsys_common_param* arg);

halsys_ret halmisc_tuner_lock(halmisc_tuner_param * arg);

#ifdef __cplusplus
}
#endif

#endif
