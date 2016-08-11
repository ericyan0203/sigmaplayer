#ifndef SIGM_UTILS_API_H
#define SIGM_UTILS_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "SIGM_Types.h"


#define SetBits(RegValue, StartBit, Bits, NewValue)                       \
    do {                                                                  \
        (RegValue) &= ~(((0x1 << (Bits)) - 1) << (StartBit));             \
        (RegValue) |= ((NewValue) & ((0x1 << (Bits)) - 1)) << (StartBit); \
    } while (0)

#define GetBits(RegValue, StartBit, Bits) (((RegValue) >> (StartBit)) & ((0x1 << (Bits)) - 1))

Error_Type_e Utils_RegisteCallback(CallBackFuncsInfo_t* const pCbInfo, sigm_callback_t pInfoRoutine,void* pUserParam);

Error_Type_e Utils_UnregisteCallback(CallBackFuncsInfo_t* const pCbInfo);

Error_Type_e Utils_InvokeCallback(sigma_handle_t ptInst, SIGM_EVENT_INDEX nIndex, datatype_ptr Param);

void Utils_InitInfoArray(void);

void Utils_DeinitInfoArray(void);

#ifdef __cplusplus
}
#endif

#endif


