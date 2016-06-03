#include "SIGM_Media_API.h"
#include "halsys_stub.h"

Error_Type_e HalSys_Media_Initialize(void) {
}

Error_Type_e HalSys_Media_Deinit(void){
}

Error_Type_e HalSys_Media_Open(Media_Config_t* ptMediaConfig, sigma_handle_t* ptInst) {
}

Error_Type_e HalSys_Media_Close(sigma_handle_t ptInst){
}	

Error_Type_e HalSys_Media_Start(sigma_handle_t ptInst){
}

Error_Type_e HalSys_Media_Stop(sigma_handle_t ptInst) {
}

Error_Type_e HalSys_Media_Pause(sigma_handle_t ptInst){
}

Error_Type_e HalSys_Media_Resume(sigma_handle_t ptInst){
}

Error_Type_e HalSys_Media_Flush(sigma_handle_t ptInst, FlushMode_e eMode){
}

//Error_Type_e HalSys_Media_SetParameter(sigma_handle_t ptInst, SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

//Error_Type_e HalSys_Media_GetParameter(sigma_handle_t ptInst, SIGM_PARAMETER_INDEX eParameter, datatype_ptr* pData);

Error_Type_e HalSys_Media_PushFrame(sigma_handle_t ptInst, Media_Buffer_t* pMediaBuffer) {
}

//Error_Type_e HalSys_Media_InstallCallback(CallBackFuncsInfo_t* const pCbInfo, sigm_callback_t pInfoRoutine, void* pUserParam);

//Error_Type_e HalSys_Media_UnInstallCallback(CallBackFuncsInfo_t* ptCallbackInfo);
