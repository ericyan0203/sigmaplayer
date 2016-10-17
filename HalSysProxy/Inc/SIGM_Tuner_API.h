#ifndef SIGM_TUNER_API_H
#define SIGM_TUNER_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


typedef enum {
    HalSys_DemodType_INVALID,
    HalSys_DemodType_ATSC,
    HalSys_DemodType_DVBC,
    HalSys_DemodType_DVBT,
    HalSys_DemodType_MAX
} HalSys_DemodType_e;

int HalMisc_Tuner_Lock(HalSys_DemodType_e stacktype, int bandwidth, int symbolrate, int freqkhz);

#ifdef __cplusplus
}
#endif

#endif
