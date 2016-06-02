/**
 * @file   SIGM_API.h
 * @author Eric (eric_yan@sigmadesigns.com)
 * @date   Feb 2016
 * @brief  API for the system.
 *
 */

#ifndef SIGM_API_H
#define SIGM_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "SIGM_Types.h"

Error_Type_e HalSys_Initialize(BoardConfig_t* ptBoardConfig);

Error_Type_e HalSys_Deinit(void);

Error_Type_e HalSys_Suspend(void);

Error_Type_e HalSys_SetParameter(SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

Error_Type_e HalSys_GetParameter(SIGM_PARAMETER_INDEX eParameter, datatype_ptr* pData);

#ifdef __cplusplus
}
#endif

#endif
