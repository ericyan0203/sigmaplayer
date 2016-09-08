#ifndef SIGM_BROADCAST_API_H
#define SIGM_BROADCAST_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "SIGM_Types.h"
#include "SIGM_DigitalTV_Types.h"

Error_Type_e HalSys_DigitalTV_Initialize(void);

Error_Type_e HalSys_DigitalTV_Deinit(void);

Error_Type_e HalSys_DigitalTV_Open(Demux_Config_t* ptDemuxConfig, sigma_handle_t* ptInst);

Error_Type_e HalSys_DigitalTV_Close(sigma_handle_t ptInst);

Error_Type_e HalSys_DigitalTV_Start(sigma_handle_t ptInst, Channel_Config_t* ptChannelConfig, sigma_handle_t* ptChannel);

// Error_Type_e HalSys_DigitalTV_Switch(sigma_handle_t ptInst, Video_Sink_e* ptVideoSink);

Error_Type_e HalSys_DigitalTV_Stop(sigma_handle_t ptInst);

#if 0
Error_Type_e HalSys_DigitalTV_RequestSecFilter(sigma_handle_t ptInst, Filter_Config_t* ptFileterConfig, filter_ptr* ptFilter);

// Error_Type_e HalSys_DigitalTV_InstallDataCallback(filter_ptr ptFilter, FilterDataCBFunction_t callback);

// Error_Type_e HalSys_DigitalTV_UnInstallDataCallback(filter_ptr ptFilter, FilterDataCBFunction_t callback);

Error_Type_e HalSys_DigitalTV_StartSecFilter(filter_ptr ptFilter);

Error_Type_e HalSys_DigitalTV_StopSecFilter(filter_ptr ptFilter);

Error_Type_e HalSys_DigitalTV_ReleaseSecFilter(filter_ptr ptFilter);

// Error_Type_e HalSys_DigitalTV_InstallPVRCallback(case_ptr
// ptInst,PVRDataCBFunction_t data_callback,PVRIndexCBFunction_t
// index_callback);

// Error_Type_e HalSys_DigitalTV_Pause(sigma_handle_t ptInst);

// Error_Type_e HalSys_DigitalTV_Resume(sigma_handle_t ptInst);

Error_Type_e HalSys_DigitalTV_SetParameter(sigma_handle_t ptInst, SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

Error_Type_e HalSys_DigitalTV_GetParameter(sigma_handle_t ptInst, SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

Error_Type_e HalSys_DigitalTV_InstallCallback(CallBackFuncsInfo_t* const cbInfo, sigm_callback_t pInfoRoutine, void* pUserParam);

Error_Type_e HalSys_DigitalTV_UnInstallCallback(CallBackFuncsInfo_t* ptCallbackInfo);
#endif

#ifdef __cplusplus
}
#endif

#endif
