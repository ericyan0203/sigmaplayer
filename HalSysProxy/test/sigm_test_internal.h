#ifndef SIGM_TEST_INTERNAL_H
#define SIGM_TEST_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stdio.h>
#include "SIGM_Types.h"
#include "SIGM_API.h"
#include "SIGM_Media_API.h"
#include "SIGM_Media_Types.h"
#include "SIGM_DigitalTV_API.h"
#include "SIGM_DigitalTV_Types.h"
#include "SIGM_HDMI_Types.h"
#include "SIGM_HDMI_API.h"
#include "trid_datatype.h"

	
#define DATA_UNIT_SIZE	(1024 * 512)
#define US_PER_MS		1000
#define TRID_ERROR		printf
#define TRID_WARNING	printf
#define TRID_INFO		printf
#define TRID_DEBUG		printf


typedef struct HalSysConfig {
    const char* pInput;
    FILE* fp;
    Video_CodingType_e eVideoType;
} HalSysConfig_t, *pHalSysConfig_t;

#ifdef __cplusplus
}
#endif

#endif
