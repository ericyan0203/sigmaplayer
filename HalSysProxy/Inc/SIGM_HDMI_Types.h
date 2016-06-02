#ifndef HDMI_DATATYPE_H
#define HDMI_DATATYPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SOURCE_DETECT_HDMI_1 0x01
#define SOURCE_DETECT_HDMI_2 0x02
#define SOURCE_DETECT_HDMI_3 0x04
#define SOURCE_DETECT_HDMI_4 0x08
#define SOURCE_DETECT_HDMI_ALL (SOURCE_DETECT_HDMI_1 | SOURCE_DETECT_HDMI_2 | SOURCE_DETECT_HDMI_3 | SOURCE_DETECT_HDMI_4)
#define SOURCE_DETECT_NONE 0

typedef enum HDMI_PORT_STATUS { SIGM_HDMI_STATUS_CONNECT, SIGM_HDMI_STATUS_DISCONNECT } HDMI_Port_Status_e;

typedef struct HDMI_CONFIG { trid_bool bFrc; } HDMI_Config_t, *pHDMI_Config_t;

typedef struct HDMI_PORT_CONFIG {
    Display_Source_e eHDMIInput;
    trid_uint32 nAudioSink;
    Video_Sink_e eVideoSink;
} HDMI_PortConfig_t, *pHDMI_PortConfig_t;

#ifdef __cplusplus
}
#endif

#endif
