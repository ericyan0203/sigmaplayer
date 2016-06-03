#ifndef __TRID_AVDEF_COMMON_H__
#define __TRID_AVDEF_COMMON_H__

#include "trid_datatype.h"

/**
 * @brief TODO
 * @note Important: this structure must be the same as the on in Master!!!!!!!!!
 */
typedef struct {
    trid_sint32 cropOffsetTop;
    trid_sint32 cropOffsetBottom;
    trid_sint32 cropOffsetLeft;
    trid_sint32 cropOffsetRight;
} CropInfo_t;

/**
 * @brief Aspect ratio enumeration
 */
typedef enum {
    ASPECT_Dont_Care = -1, /** H.264 and AVS use this flag*/
    ASPECT_None = 0,       /**<none*/
    ASPECT_Square = 1,     /**<1:1*/
    ASPECT_4_3,            /**<4:3*/
    ASPECT_16_9,           /**<16:9*/
    ASPECT_221_1,          /**<2.21 : 1*/
    ASPECT_14_9,           /**<14:9*/
    ASPECT_Total,
} AspectCode_e;

/**
 * @brief Frame rate enumeration
 */
typedef enum {
    FRAMERATE_OTHER = 0,     /**<other frame rate*/
    FRAMERATE_23_976_HZ = 1, /**<23.976Hz*/
    FRAMERATE_24_HZ = 2,     /**<24Hz*/
    FRAMERATE_25_HZ = 3,     /**<25Hz*/
    FRAMERATE_29_97_HZ = 4,  /**<29.97Hz*/
    FRAMERATE_30_HZ = 5,     /**<30Hz*/
    FRAMERATE_50_HZ = 6,     /**<50Hz*/
    FRAMERATE_59_94_HZ = 7,  /**<59.94Hz*/
    FRAMERATE_60_HZ = 8,     /**<60Hz*/
    FRAMERATE_8_HZ = 9,
    FRAMERATE_12_HZ = 10,
    FRAMERATE_15_HZ = 11,
    FRAMERATE_100_HZ = 12,
    FRAMERATE_119_HZ = 13,
    FRAMERATE_120_HZ = 14,
    FRAMERATE_UNKNOWN,
    FRAMERATE_EXT
} FrameRate_e;

/**
 * @brief IP(Interlace/Progressive) enumeration
 */
typedef enum {
    PROGSEQ_INTERLACE = 0, /**<Interlace*/
    PROGSEQ_PROGRESSIVE    /**<Progressive*/
} ProgSeq_e;

/**
 * @brief TODO
 */
typedef enum {
    BPP8 = 0,
    BPP10,
} PixelDepth_e;

/**
 * @brief Active format definition
 */
typedef enum _ACTIVEFORMAT {
    AFD_AUTO = 8,        /**<1000*/
    AFD_4_3 = 9,         /**<1001*/
    AFD_16_9 = 10,       /**<1010*/
    AFD_14_9 = 11,       /**<1011*/
    AFD_FUTURE = 12,     /**<1100*/
    AFD_4_3_SHOOT = 13,  /**<1101*/
    AFD_16_9_SHOOT = 14, /**<1110*/
    AFD_SHOOT = 15       /**<1111*/
} ActiveFormat_e;

#endif
