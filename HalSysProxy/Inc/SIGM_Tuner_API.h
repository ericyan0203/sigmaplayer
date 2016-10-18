#ifndef SIGM_TUNER_API_H
#define SIGM_TUNER_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef enum {
    HalSys_DemodType_INVALID,
    HalSys_DemodType_ATSC_AIR,
    HalSys_DemodType_ATSC_STD,
    HalSys_DemodType_ATSC_HRC,
    HalSys_DemodType_ATSC_IRC,
    HalSys_DemodType_ATSC_ATV,
    HalSys_DemodType_DVBC_DTV,
    HalSys_DemodType_DVBC_ATV,
    HalSys_DemodType_DVBT_DTV,
    HalSys_DemodType_DVBT_ATV,
    HalSys_DemodType_MAX
} HalSys_DemodType_e;

int HalMisc_Tuner_Lock(HalSys_DemodType_e stacktype, int bandwidth, int symbolrate, int freqkhz);

#ifdef __cplusplus
}
#endif

#endif
