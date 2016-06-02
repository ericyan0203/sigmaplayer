#ifndef SIGM_TYPES_H
#define SIGM_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include "trid_datatype.h"

typedef void* datatype_ptr;

typedef const void* sigma_handle_t;

typedef char* datatype_string;

typedef unsigned char datatype_byte;

typedef enum ERROR_TYPE {
    SIGM_ErrorNone = 0,

    /** There were insufficient resources to perform the requested operation */
    SIGM_ErrorInsufficientResources = (trid_sint32)0x80001000,

    /** common error */
    SIGM_ErrorFailed,

    /** There was an error, but the cause of the error could not be determined */
    SIGM_ErrorUndefined,

    /** One or more parameters were not valid */
    SIGM_ErrorBadParameter,

    /** The requested function is not implemented */
    SIGM_ErrorNotImplemented,

    /** The index is out fo range */
    SIGM_ErrorOutOfIndex,

    /** The buffer was emptied before the next buffer was ready */
    SIGM_ErrorUnderflow,

    /** The buffer was not available when it was needed */
    SIGM_ErrorOverflow,

    /** The hardware failed to respond as expected */
    SIGM_ErrorHardware,

    /** The component is in the state StateInvalid */
    SIGM_ErrorInvalidState,

    /** The component is not ready */
    SIGM_ErrorNotReady,

    /** There was a timeout that occurred */
    SIGM_ErrorTimeout,

    /** This error occurs when trying to transition into the state you are already
       in */
    SIGM_ErrorSameState,

    /** Attempting a state transtion that is not allowed */
    SIGM_ErrorIncorrectStateTransition,

    SIGM_ErrorNotSupported,

    SIGM_ErrorExtensions = (trid_sint32)0x8F000000, /**< Reserved region for
                                                       introducing Extensions */

    SIGM_ErrorMax = 0x7FFFFFFF

} Error_Type_e;

typedef enum SIGM_STATE {
    STATE_UNINIT,
    STATE_IDLE,
    STATE_LOADING,
    STATE_LOADED,
    STATE_RUNNING,  // in multimedia  case
    STATE_PAUSED,
    STATE_FLUSH_IN_PROGRESS,  // in multimedia  case
    STATE_STOPPED,            // in multimedia  case
    STATE_UNLOADED,
    STATE_MAX
} State_e;

typedef enum CLOCK_MODE { CLOCK_MODE_TIMER, CLOCK_MODE_STC, CLOCK_MODE_AUDIO, CLOCK_MODE_MAX } Clock_Mode_e;

typedef enum VIDEO_CODINGTYPE {
    SIGM_VIDEO_CodingUnused, /**< Value when coding is N/A */
    SIGM_VIDEO_CodingMPEG12,
    SIGM_VIDEO_CodingDIX3,
    SIGM_VIDEO_CodingMPEG4,
    SIGM_VIDEO_CodingVC1,
    SIGM_VIDEO_CodingRV,
    SIGM_VIDEO_CodingAVC,
    SIGM_VIDEO_CodingMVC,
    SIGM_VIDEO_CodingAVS,
    SIGM_VIDEO_CodingJPEG,
    SIGM_VIDEO_CodingVP6,
    SIGM_VIDEO_CodingVP8,  /**< Google VP8, formerly known as On2 VP8 */
    SIGM_VIDEO_CodingVP9,  /**< Google VP9 */
    SIGM_VIDEO_CodingHEVC, /**< HEVC */
    SIGM_VIDEO_CodingMax = 0xFF
} Video_CodingType_e;

typedef enum AUDIO_CODINGTYPE {
    SIGM_AUDIO_CodingUnused,
    SIGM_AUDIO_CodingAC3,
    SIGM_AUDIO_CodingMPEG,
    SIGM_AUDIO_CodingAAC,
    SIGM_AUDIO_CodingMP3,
    SIGM_AUDIO_CodingAC3Plus,
    SIGM_AUDIO_CodingPCM,
    SIGM_AUDIO_CodingWMA,
    SIGM_AUDIO_CodingWAV,
    SIGM_AUDIO_CodingAUX,
    SIGM_AUDIO_CodingVORBIS,
    SIGM_AUDIO_CodingAIFC,
    SIGM_AUDIO_CodingRACOOK,
    SIGM_AUDIO_CodingDTS,
    SIGM_AUDIO_CodingALAC,
    SIGM_AUDIO_CodingRealAudio,
    SIGM_AUDIO_CodingDRA,
    SIGM_AUDIO_CodingAMRNB,
    SIGM_AUDIO_CodingAMRWB,
    SIGM_AUDIO_CodingSILK,
    SIGM_AUDIO_CodingHEAAC,
    SIGM_AUDIO_CodingFLAC,
    SIGM_AUDIO_CodingHEAAC_V2,
    SIGM_AUDIO_CodingLPCM,
    SIGM_AUDIO_CodingWMAPRO,
    SIGM_AUDIO_CodingOPUS,
    SIGM_AUDIO_ENCODER_CodingMP3,
    SIGM_AUDIO_ENCODER_CodingSILK,
    SIGM_AUDIO_ENCODER_CodingSBC,
    SIGM_AUDIO_HDMI_CodingAC3,
    SIGM_AUDIO_HDMI_CodingPCM,
    SIGM_AUDIO_CodingMax = 0xFF,
} Audio_CodingType_e;

typedef enum VIDEO_PLAY_MODE {
    SIGM_PLAYMODE_NORMAL,
    SIGM_PLAYMODE_IFRAME,
    SIGM_PLAYMODE_STEPFRAME,
    SIGM_PLAYMODE_UNDEFINED
} Video_PlayMode_e;

typedef enum VIDEO_MUX_TYPE { SIGM_MUX_INVALID, SIGM_MUX_ES, SIGM_MUX_PES } Video_MuxType_e;

typedef enum VIDEO_DISPLAY_MODE { SIGM_SEAMLESS_NONE, SIGM_SEAMLESS_FHD, SIGM_SEAMLESS_UD, SIGM_SEAMLESS_UNDEFINED } Video_DisplayMode_e;

typedef enum SIGM_VIDEO_SINK {
    SIGM_VIDEO_SINK_INVALID = 0,
    SIGM_VIDEO_SINK_MP = 1,
    SIGM_VIDEO_SINK_PIP = 2,
    SIGM_VIDEO_SINK_BOTH = 3,
    SIGM_VIDEO_SINK_MAX
} Video_Sink_e;

typedef enum SIGM_SOUND_SOURCE {
    SIGM_SOUND_SOURCE_INVALID = 0,
    SIGM_SOUND_SOURCE_HDMI_AC3 = 0x1,
    SIGM_SOUND_SOURCE_HDMI_PCM = 0x2,
    SIGM_SOUND_SOURCE_DTV = 0x4,
    SIGM_SOUND_SOURCE_MAX
} Sound_Source_e;

typedef enum SIGM_SOUND_SINK {
    SIGM_SOUND_SINK_INVALID = 0,
    SIGM_SOUND_SINK_SPDIF_PCM = 0x01,
    SIGM_SOUND_SINK_SPDIF_BITSTRAM = 0x02,
    SIGM_SOUND_SINK_SPEAKER = 0x04,
    SIGM_SOUND_SINK_HEADPHONE = 0x08,
    SIGM_SOUND_SINK_SCART1 = 0x10,
    SIGM_SOUND_SINK_SCART2 = 0x20,
    SGIM_SOUND_SINK_MAX
} Sound_Sink_e;

typedef enum SIGM_DEMUX_SINK { SIGM_DEMUX_SINK_INVALID = -1, SIGM_DEMUX_SINK_AV, SIGM_DEMUX_SINK_HOST, SIGM_DEMUX_SINK_MAX } Demux_Sink_e;

typedef enum SIGM_WINDOW_STYLE {
    SIGM_STYLE_169_SUPERWIDE = 0, /**< MP window style, 16:9 super wide*/
    SIGM_STYLE_169_WIDE,          /**< MP window style, 16:9 wide*/
    SIGM_STYLE_169_PANORAMA,      /**< MP window style, 16:9 panorama*/
    SIGM_STYLE_169_SUBZOOM,       /**< MP window style, 16:9 sub zoom*/
    SIGM_STYLE_169_ZOOM,          /**< MP window style, 16:9 zoom*/
    SIGM_STYLE_169_149_ZOOM,      /**< MP window style, 16:9 14:9 zoom*/
    SIGM_STYLE_169_43,            /**< MP window style, 16:9 4:3*/
    SIGM_STYLE_43_NORMAL,         /**< MP window style, 4:3 normal*/
    SIGM_STYLE_43_169_ZOOM,       /**< MP window style, 4:3 16:9 zoom*/
    SIGM_STYLE_TTX_PAT,           /**< MP window style, TTX PAT*/
    SIGM_STYLE_AUTO,              /**< MP window style, Auto*/
    SIGM_STYLE_PANORAMA,          /**< MP window style, Panorama*/
    SIGM_STYLE_FULL,              /**< MP window style, Full*/
    SIGM_STYLE_EPG,               /**< MP window style, EPG*/
    SIGM_STYLE_43_149_ZOOM,       /**< MP window style, 4:3 14:9 zoom*/
    SIGM_STYLE_169_149,           /**< MP window style, 16:9 14:9*/
    SIGM_STYLE_DotByDot,          /**< MP window style, Dot by dot*/
    SIGM_STYLE_MAX
} Window_Style_e;

typedef enum SIGM_AUDIO_OUTPUT_MODE {
    SIGM_AUDIO_OUT_PCM = 0, /**<PCM audio out*/
    SIGM_AUDIO_OUT_DDPLUS,  /**<Dolby audio out*/
    SIGM_AUDIO_OUT_DTS,     /**<Dolby & DTS audio out*/
    SIGM_AUDIO_OUT_MAX
} Audio_OutputMode_e;

typedef enum SIGM_DISPLAY_SOURCE {
    SIGM_DISPLAY_SOURCE_NONE = 0,
    SIGM_DISPLAY_SOURCE_DVI,
    SIGM_DISPLAY_SOURCE_HDMI1,
    SIGM_DISPLAY_SOURCE_HDMI2,
    SIGM_DISPLAY_SOURCE_HDMI3,
    SIGM_DISPLAY_SOURCE_HDMI4,
    SIGM_DISPLAY_SOURCE_DV_SCART,
    SIGM_DISPLAY_SOURCE_EXT_VGA,
    SIGM_DISPLAY_SOURCE_EXT_YPBPR,
    SIGM_DISPLAY_SOURCE_HIDTV1,
    SIGM_DISPLAY_SOURCE_HIDTV2,
    SIGM_DISPLAY_SOURCE_INVALID
} Display_Source_e;

typedef enum VIDEO_STREAM_MODE {
    SIGM_STREAMMODE_NORMAL,
    SIGM_STREAMMODE_EDR,
    SIGM_STREAMMODE_HDR,
    SIGM_STREAMMODE_UNDEFINED
} Video_StreamMode_e;

typedef enum SIGM_PARAMETER_INDEX {
    SIGM_PARAMETER_GET_INX_BASE = 0x10,
    SIGM_PARAMETER_GET_BACKEND_STATE,
    SIGM_PARAMETER_GET_STREAM_DURATION,
    SIGM_PARAMETER_GET_STREAM_POSITON,
    SIGM_PARAMETER_GET_VIDEO_INPUTBUFFER_STATUS,
    SIGM_PARAMETER_GET_AUDIO_INPUTBUFFER_STATUS,
    SIGM_PARAMETER_GET_VIDEO_STATISTICS,
    SIGM_PARAMETER_GET_AUDIO_STATISTICS,
    SIGM_PARAMETER_GET_AUDIO_FORMAT,
    SIGM_PARAMETER_GET_SIGNAL_FORMAT,

    SIGM_PARAMETER_GET_DISPLAY_EXTENSION_BASE = 0x20, /**< display parameter get base index */
    SIGM_PARAMETER_GET_DISPLAY_WINDOW_STYLE,          /**< display get window style */
    SIGM_PARAMETER_GET_DISPLAY_WINDOW,                /**< display get window positon */
    SIGM_PARAMETER_GET_DISPLAY_SATURATION,            /**< display get saturation */
    SIGM_PARAMETER_GET_DISPLAY_HUE,                   /**< display get hue */
    SIGM_PARAMETER_GET_DISPLAY_BRIGHTNESS,            /**< display get brightness */
    SIGM_PARAMETER_GET_DISPLAY_CONTRAST,              /**< display get contrast */
    SIGM_PARAMETER_GET_DISPLAY_SHARPNESS,             /**< display get sharpness */
    SIGM_PARAMETER_GET_DISPLAY_FREEZE,                /**< display get freeze frame */
    SIGM_PARAMETER_GET_DISPLAY_WHITEBALANCE,          /**< display get white balance */
    SIGM_PARAMETER_GET_HDMI_PORT_STATUS,
    SIGM_PARAMETER_GET_HDMI_CURRENT_PORT_STATUS,
    SIGM_PARAMETER_GET_DISPLAY_MAX,

    SIGM_PARAMETER_GET_SOUND_EXTENSION_BASE = 0x40,
    SIGM_PARAMETER_GET_SOUND_VOLUME,
    SIGM_PARAMETER_GET_SOUND_MUTE,
    SIGM_PARAMETER_GET_SOUND_DELAY,
    SIGM_PARAMETER_GET_SOUND_MAX,

    // SIGM_PARAMETER_GET_ALL_CHANNEL_INFO,
    // SIGM_PARAMETER_GET_CUR_CHANNEL_INFO,

    SIGM_PARAMETER_SET_INX_BASE = 0x80,
    SIGM_PARAMETER_SET_CLOCK_POSITION,
    SIGM_PARAMETER_SET_AUDIO_MUTE,
    SIGM_PARAMETER_SET_VIDEO_MUTE,
    SIGM_PARAMETER_SET_VIDEO_STEP,

    SIGM_PARAMETER_SET_DISPLAY_EXTENSION_BASE = 0xA0, /**< display parameter set base index */
    SIGM_PARAMETER_SET_DISPLAY_WINDOW_STYLE,          /**< display set window style */
    SIGM_PARAMETER_SET_DISPLAY_WINDOW,                /**< display set window  position */
    SIGM_PARAMETER_SET_DISPLAY_SATURATION,            /**< display set saturation */
    SIGM_PARAMETER_SET_DISPLAY_HUE,                   /**< display set hue */
    SIGM_PARAMETER_SET_DISPLAY_BRIGHTNESS,            /**< display set brightness */
    SIGM_PARAMETER_SET_DISPLAY_CONTRAST,              /**< display set contrast */
    SIGM_PARAMETER_SET_DISPLAY_SHARPNESS,             /**< display set sharpness */
    SIGM_PARAMETER_SET_DISPLAY_FREEZE,                /**< display set frame freeze */
    SIGM_PARAMETER_SET_DISPLAY_WHITEBALANCE,          /**< display set white balance */
    SIGM_PARAMETER_SET_DISPLAY_MIRROR,                /**< display set mirror */
    SIGM_PARAMETER_SET_DISPLAY_SCREENCOLOR,           /**< display set screen color */
    SIGM_PARAMETER_SET_DISPLAY_MAX,

    SIGM_PARAMETER_SET_SOUND_EXTENSION_BASE = 0xC0,
    SIGM_PARAMETER_SET_SOUND_VOLUME,
    SIGM_PARAMETER_SET_SOUND_MUTE,
    SIGM_PARAMETER_SET_SOUND_DELAY,
    SIGM_PARAMETER_SET_SOUND_MAX,
    // SIGM_PARAMETER_SET_PVR_CONFIG,

    SIGM_PARAMETER_INX_MAX = 0xFF
} SIGM_PARAMETER_INDEX;

typedef enum SIGM_EVENT_INDEX {
    SIGM_EVENT_INX_BASE = 0x10,
    SIGM_EVENT_DISP_BLACKSCREEN_ON,
    SIGM_EVENT_DISP_BLACKSCREEN_OFF,
    SIGM_EVENT_VIDEO_FORMAT,
    SIGM_EVENT_VIDEO_NOTSUPPORTED,
    SIGM_EVENT_VIDEO_FRAME_DECODED,
    SIGM_EVENT_AUDIO_NOTSUPPORTED,
    SIGM_EVENT_AUDIO_FORMAT,
    SIGM_EVENT_STREAM_END,
    SIGM_EVENT_DEMUX_SECTIONDATA,
    SIGM_EVENT_DEMUX_PESDATA,
    SIGM_EVENT_BROADCAST_UDATA,
    SIGM_EVENT_DISPLAY_SIGNAL,
    SIGM_EVENT_SOUND_FORMAT,
    SIGM_EVENT_HDMI_HOTPLUG,
    SIGM_EVENT_HDMI_HOTUNPLUG,
    SIGM_EVENT_INX_MAX
} SIGM_EVENT_INDEX;

typedef struct HALSYS_CALLBACK_FUNCSINFO {
    sigma_handle_t ptInst;
    SIGM_EVENT_INDEX InfoID;
    trid_uint32 sub_id;
} CallBackFuncsInfo_t;

typedef void (*sigm_callback_t)(CallBackFuncsInfo_t* pCbInfo, void* pParam, void* pUserParam);

#if 0
typedef struct DISPLAY_BOARD_CONFIG {
    const char* pSettingCfgXML;
    const char* pSystemCfgXML;
} DisplayBoardConfig_t;
#endif

typedef struct BOARD_CONFIG {
    const char* pSoundBin;
    const char* pAudioPathXML;
    const char* pDisplayFilePath;
} BoardConfig_t;

#ifdef __cplusplus
}
#endif
#endif
