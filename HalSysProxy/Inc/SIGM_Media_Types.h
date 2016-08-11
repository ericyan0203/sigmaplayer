#ifndef MEDIA_DATATYPE_H
#define MEDIA_DATATYPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SIGM_BUFFERFLAG_AUDIOFRAME 0x1

#define SIGM_BUFFERFLAG_VIDEO_EL 0x2

#define SIGM_BUFFERFLAG_VIDEO_BL 0x4

#define SIGM_BUFFERFLAG_VIDEO_MATADATA 0x8

#define SIGM_BUFFERFLAG_ENDOFSTREAM 0x10

#define SIGM_BUFFERFLAG_ENDOFFRAME 0x20

#define SIGM_BUFFERFLAG_CONFIGDATA 0x40

#define SIGM_BUFFERFLAG_BLOCKCALL 0x80

#define SIGM_BUFFERFLAG_HWDEMUX 0x100

#define SIGM_BUFFERFLAG_FAKEFRAME 0x200

typedef enum FLUSH_MODE {
    SIGM_FLUSHMODE_INVALID,
    SIGM_FLUSHMODE_AUDIO,
    SIGM_FLUSHMODE_VIDEO,
    SIGM_FLUSHMODE_BOTH,
    SIGM_FLUSHMODE_MAX
} FlushMode_e;

typedef struct MEDIA_BUFFER {
    trid_uint32 nSize;      /**< size of the structure in bytes */
    trid_uint8* pBuffer;    /**< Pointer to actual block of memory
                             that is acting as the buffer */
    trid_uint32 nAllocLen;  /**< size of the buffer allocated, in bytes */
    trid_uint32 nFilledLen; /**< number of bytes currently in the
                             buffer */
    trid_uint32 nOffset;    /**< start offset of valid data in bytes from
                             the start of the buffer */
    trid_uint64 nTimeStamp; /**< Timestamp corresponding to the sample
                            starting at the first logical sample
                            boundary in the buffer. Timestamps of
                            successive samples within the buffer may
                            be inferred by adding the duration of the
                            of the preceding buffer to the timestamp
                            of the preceding buffer.*/
    trid_uint32 nFlags;     /**< buffer specific flags */
} Media_Buffer_t, *pMedia_Buffer_t;

typedef struct MEDIA_VIDEO_CONFIG {
    Video_CodingType_e eVideoFormat;
    Video_PlayMode_e eVideoPlayMode;
    Video_DisplayMode_e eVideoDisplayMode;
    Video_StreamMode_e eVideoStreamMode;
    Video_Sink_e eVideoSink;
    Video_MuxType_e eMuxType;
} Meida_VideoConfig_t, *pMeida_VideoConfig_t;

typedef struct MEDIA_AUDIO_CONFIG {
    Audio_CodingType_e eAudioFormat;
    trid_uint32 eSoundSink;
} Media_AudioConfig_t, *pMedia_AudioConfig_t;

typedef struct MEDIA_CONFIG {
    Meida_VideoConfig_t tVideoConfig;
    Media_AudioConfig_t tAudioConfig;
    Clock_Mode_e eClockMode;
    trid_bool bLowLatency;
} Media_Config_t, *pMedia_Config_t;

typedef struct MEDIA_FLUSH_CONFIG {
	FlushMode_e eMode;
	trid_uint64 nTimeStamp;
}Media_FlushConfig_t,*pMedia_FlushConfig_t;

#ifdef __cplusplus
}
#endif

#endif
