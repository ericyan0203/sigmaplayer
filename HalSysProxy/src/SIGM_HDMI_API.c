#include "SIGM_HDMI_API.h"
#include "halsys_stub.h"

Error_Type_e HalSys_HDMI_Initialize(void) {
	halsys_ret ret;
	ret = halsys_hdmi_initialize();
	return (Error_Type_e)(ret.common_ret.ret);

}

Error_Type_e HalSys_HDMI_Deinit(void) {
	halsys_ret ret;
	ret = halsys_hdmi_deinit();
	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_HDMI_Open(HDMI_Config_t* ptConfig, sigma_handle_t* ptInst) {
	halsys_ret ret;
	hdmi_open_param param;
	; 
	param.bFrc = ptConfig->bFrc;

	ret = halsys_hdmi_open(&param);

	*ptInst = (sigma_handle_t)ret.open_ret.instance;

	return (Error_Type_e)(ret.open_ret.ret);
}

Error_Type_e HalSys_HDMI_Start(sigma_handle_t ptInst, HDMI_PortConfig_t* ptConfig, sigma_handle_t* ptPort) {
	halsys_ret ret;
	hdmi_start_param param;

	param.instance = (unsigned int)ptInst;
	param.audio_sink = (unsigned int)ptConfig->nAudioSink;
	param.video_sink = (unsigned int)ptConfig->eVideoSink;
	param.display_source = (unsigned int)ptConfig->eHDMIInput;
	
	ret = halsys_hdmi_start(&param);

	*ptPort = (sigma_handle_t)ret.open_ret.instance;

	return (Error_Type_e)(ret.open_ret.ret);
}	

Error_Type_e HalSys_HDMI_Stop(sigma_handle_t ptPort) {
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptPort;

	ret = halsys_hdmi_stop(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_HDMI_Close(sigma_handle_t ptInst){
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_hdmi_close(&param);

	return (Error_Type_e)(ret.common_ret.ret);
	
}

//Error_Type_e HalSys_HDMI_SetParameter(sigma_handle_t ptPort, SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

//Error_Type_e HalSys_HDMI_GetParameter(sigma_handle_t ptPort, SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

//Error_Type_e HalSys_HDMI_InstallCallback(CallBackFuncsInfo_t* const cbInfo, sigm_callback_t pInfoRoutine, void* pUserParam);

//Error_Type_e HalSys_HDMI_UnInstallCallback(CallBackFuncsInfo_t* ptCallbackInfo);
