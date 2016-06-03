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
typedef struct HALSYS_COMMON_PARAMETER { unsigned int instance; } __attribute((packed)) halsys_common_param;

/** struct HALSYS_COMMON_RETURN
 *  	common return type
 *   '1I'
 */
typedef struct HALSYS_COMMON_RETURN { unsigned int ret; } __attribute((packed)) halsys_common_ret;

/** struct HALSYS_COMMAND_PARAMETER
 *  Init Parameter for HalSys_Initialize
 *  '128s128s128s'
 */
typedef struct HALSYS_INIT_PARAMETER {
    char sound_path[PATHLEN];
    char audio_path[PATHLEN];
    char display_path[PATHLEN];
} __attribute((packed)) halsys_init_param;

/** struct MEDIA_OPEN_PARAMETER
 *  media open command parameter
 *  10B
 */
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
} __attribute((packed)) media_open_param;

/** struct HALSYS_OPEN_RETURN
 *   media open command return value
 *   '2I'
 */
typedef struct HALSYS_OPEN_RETURN {
    unsigned int ret;
    unsigned int instance;
} __attribute((packed)) halsys_open_ret;

/** struct MEDIA_FLUSH_PARAMETER
 *  media flush command parameter
 *  '1I1B'
 */
typedef struct MEDIA_FLUSH_PARAMETER {
    unsigned int instance;
    unsigned char flush_mode;
} __attribute((packed)) media_flush_param;

/** struct MEDIA_PUSH_PARAMETER
 *  media push command parameter
 * header '5I1Q1I'
 */
typedef struct MEDIA_PUSH_PARAMETER {
    unsigned int instance;
    unsigned int size;
    unsigned int alloc_size;
    unsigned int fill_size;
    unsigned int offset;
    unsigned long long pts;
    unsigned int flags;
    unsigned char data[0];
} __attribute((packed)) media_push_param;

/** struct HDMI_OPEN_PARAMETER
 *  	HDMI open command parameter
 *   '1I'
 */
typedef struct HDMI_OPEN_PARAMETER { unsigned int bFrc; } __attribute((packed)) hdmi_open_param;

/** struct HDMI_START_PARAMETER
 *   HDMI start command parameter
 *   '4I'
 */
typedef struct HDMI_START_PARAMETER {
    unsigned int instance;
    unsigned int display_source;
    unsigned int audio_sink;
    unsigned int video_sink;
} __attribute((packed)) hdmi_start_param;

typedef struct HALSYS_RETURN {
    unsigned int size;
    union {
        halsys_common_ret common_ret;
        halsys_open_ret open_ret;
    };
} __attribute((packed)) halsys_ret;

typedef struct {
    unsigned int length;
    char domain_name[HALSYS_NAME_LENGTH];
} halsys_packet_header;

typedef struct {
    char func_name[HALSYS_NAME_LENGTH];
    unsigned int param_len;
    char data[0];
} halsys_packet_data;


//*************************************************************
// Function
//*************************************************************
halsys_ret halsys_initialize(void* arg);

halsys_ret halsys_media_initialize(void* arg);

halsys_ret halsys_media_deinit(void* arg);

halsys_ret halsys_media_open(void* arg);

halsys_ret halsys_media_close(void* arg);

halsys_ret halsys_media_start(void* arg);

halsys_ret halsys_media_stop(void* arg);

halsys_ret halsys_media_pause(void* arg);

halsys_ret halsys_media_resume(void* arg);

halsys_ret halsys_media_flush(void* arg);

halsys_ret halsys_media_pushframe(void* arg);

halsys_ret halsys_hdmi_initialize(void* arg);

halsys_ret halsys_hdmi_deinit(void* arg);

halsys_ret halsys_hdmi_open(void* arg);

halsys_ret halsys_hdmi_close(void* arg);

halsys_ret halsys_hdmi_start(void* arg);

halsys_ret halsys_hdmi_stop(void* arg);

#ifdef __cplusplus
}
#endif

#endif
