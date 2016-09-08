#ifndef BROADCAST_DATATYPE_H
#define BROADCAST_DATATYPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "SIGM_Types.h"

#define MAX_PATTERN_LENGTH 20

typedef void* filter_ptr;

typedef enum SIGM_DMX_INPUT {
    SIGM_INPUT_INTERNAL_PARALLEL_DEMOD_0 = 0,  // hsdp4 parallel input
    SIGM_INPUT_INTERNAL_SERIAL_DEMOD_0,        // hsdp4 serial   input
    SIGM_INPUT_EXTERNAL_SERIAL_DEMOD_0,        // hsdp0 serial   input
    SIGM_INPUT_EXTERNAL_SERIAL_DEMOD_1,        // hsdp1 serial   input
    SIGM_INPUT_DMA_0,                          // DMA 0   input
    SIGM_INPUT_DMA_1,                          // DMA 1   input
    SIGM_INPUT_PARALLEL_TS,                    // hsdp2 input
    SIGM_INPUT_EXTERNAL_CI,                    // extern CI  input
    SIGM_INPUT_PARALLEL_DEMOD_TS1_HSDP2,       // TS1 -> HSDP2 -> IN2
    SIGM_INPUT_PARALLEL_DEMOD_TS2_HSDP3,       // TS2 -> HSDP3 -> IN3
    SIGM_INPUT_PARALLEL_DEMOD_TS2_HSDP2,       // TS2 -> HSDP2 -> IN2 (optional)
    SIGM_INPUT_PARALLEL_DEMOD_TS1_HSDP3,       // TS1 -> HSDP3 -> IN3 (optional)
    SIGM_INPUT_DMA_BACK_HSDP3_TS2,             // OUT3 -> HSDP3 -> TS2 -> TS1 -> HSPD2 -> IN2
    SIGM_INPUT_DMA_BACK_HSDP3_TS1,             // OUT3 -> HSDP3 -> TS1 -> TS2-> HSPD2 -> IN2
    SIGM_INPUT_PARALLEL_DEMOD_TS4_HSDP4,       // TS4 -> HSDP4
    SIGM_INPUT_MAX
} Demux_Input_e;

/**
 *  Define CI pass type
 *  CI_Bypass :     demod->demux
 *  CI_INOUT_TS : demod->CI Card->demux
 *  CI_IN_TS :    demod->CI Card & demod->demux
 */
typedef enum Demux_CIPath {
    SIGM_CI_PATH_BYPASS = 0,
    SIGM_CI_PATH_INOUT_TS,  // ts1 out, ts2 in
    SIGM_CI_PATH_IN_TS,     // ts1 out
    SIGM_CI_PATH_MAX
} Demux_CIPath_e;

typedef struct VIDEO_DTV_CONFIG {
    trid_uint32 nVideoPID;
    Video_CodingType_e eVideoFormat;
    Video_Sink_e eVideoSink;
    Video_DisplayMode_e eVideoDisplayMode;
    Video_StreamMode_e eVideoStreamMode;
} DTV_VideoConfig_t, *pDTV_VideoConfig_t;

typedef struct AUDIO_DTV_CONFIG {
    trid_uint32 nAudioPID;
    Audio_CodingType_e eAudioFormat;
    trid_uint32 eSoundSink;
} DTV_AudioConfig_t, *pDTV_AudioConfig_t;

typedef struct SUBTITLE_CONFIG { trid_uint32 nPID; } Subtitle_Config_t, *pSubtitle_Config_t;

typedef struct TELETEXT_CONFIG { trid_uint32 nPID; } Teletext_Config_t, *pTeletext_Config_t;

typedef struct DEMUX_CONFIG {
    Demux_Input_e eDemuxInput;
    Demux_CIPath_e eDemuxCIPath;
} Demux_Config_t, *pDemux_Config_t;

typedef struct CHANNEL_CONFIG {
    DTV_VideoConfig_t tVideoConfig;
    DTV_AudioConfig_t tAudioConfig;
    Subtitle_Config_t tSubtitleConfig;
    Teletext_Config_t tTeletextConfig;
	trid_uint32 nPcrPID;
    trid_bool bLowlatency;
} Channel_Config_t, *pChannel_Config_t;

typedef struct FILTER_PATTEN_CONFIG {
    trid_uint8 nLength;
    trid_uint8 nPattern[MAX_PATTERN_LENGTH];
    trid_uint8 nEqualMask[MAX_PATTERN_LENGTH];
    trid_uint8 nNotEqualMask[MAX_PATTERN_LENGTH];
} Filter_Pattern_Config_t;

typedef struct FILTER_CONFIG {
    trid_uint32 nFilterPID;
    Filter_Pattern_Config_t tFilterPatternConfig;
} Filter_Config_t, *pFilter_Config_t;

// typedef Error_Type_e (*FilterDataCBFunction_t)(filter_ptr ptFilter, trid_uint32 nSize, datatype_byte* ptSectionData);
// typedef  Error_Type_e  (*PVRDataCBFunction_t)(case_ptr ptInst, trid_uint32
// nSize, datatype_byte * ptPVRData);
// typedef  Error_Type_e  (*PVRIndexCBFunction_t)(case_ptr ptInst, trid_uint32
// nSize, datatype_byte * ptPVRIndexData);

#ifdef __cplusplus
}
#endif

#endif
