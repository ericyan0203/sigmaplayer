#ifndef SIGM_EVENT_DATATYPES_H
#define SIGM_EVENT_DATATYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "SIGM_Types.h"
#include "trid_avdef.h"

typedef struct SIGM_VIDEO_RESOLUTION {
    trid_uint32 nWidth;
    trid_uint32 nHeight;
} VideoResolution_t;

typedef struct SIGM_VIDEO_FORMATINFO {
    VideoResolution_t tDecodeRes;
    VideoResolution_t tDisplayRes;
    CropInfo_t tCropInfo;
    trid_uint32 nStride;
    AspectCode_e eAspectCode; /**< aspect code*/
    FrameRate_e eFrameRate;
    ProgSeq_e eProgressive;
    PixelDepth_e ePixelDepth;
    ActiveFormat_e eAFD; /**< active format*/
    trid_uint32 nPar;    /**< image PAR*/
} VideoFormatInfo_t;

#ifdef __cplusplus
}
#endif

#endif
