#include <string.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "SIGM_Media_API.h"
#include "halsys_stub.h"

#define data_len  (6*1024*1024)
static char send_buf[2][data_len];

Error_Type_e HalSys_Media_Initialize(void) {
	halsys_ret ret;
	ret = halsys_media_initialize();
	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_Media_Deinit(void){
	halsys_ret ret;
	ret = halsys_media_deinit();
	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_Media_Open(Media_Config_t* ptMediaConfig, sigma_handle_t* ptInst) {
	halsys_ret ret;
	media_open_param param;

	param.video_format = (unsigned char)ptMediaConfig->tVideoConfig.eVideoFormat;
	param.video_playback_mode = (unsigned char)ptMediaConfig->tVideoConfig.eVideoPlayMode;
	param.video_seamless_mode = (unsigned char)ptMediaConfig->tVideoConfig.eVideoDisplayMode;
	param.video_stream_mode = (unsigned char)ptMediaConfig->tVideoConfig.eVideoStreamMode;
	param.video_sink = (unsigned char)ptMediaConfig->tVideoConfig.eVideoSink;
	param.video_mux = (unsigned char)ptMediaConfig->tVideoConfig.eMuxType;

	param.audio_format =(unsigned char)ptMediaConfig->tAudioConfig.eAudioFormat;
	param.audio_sink = (unsigned char)ptMediaConfig->tAudioConfig.eSoundSink;

	param.clock_mode = (unsigned char)ptMediaConfig->eClockMode;
	param.lowlatency = (unsigned char)ptMediaConfig->bLowLatency;

	printf("video format %d\n",param.video_format);
	ret = halsys_media_open(&param);

	*ptInst = (sigma_handle_t)ret.open_ret.instance;

	printf("video format %d inst %x\n",param.video_format,ret.open_ret.instance);
	return (Error_Type_e)(ret.open_ret.ret);
	
}

Error_Type_e HalSys_Media_Close(sigma_handle_t ptInst){
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_media_close(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}	

Error_Type_e HalSys_Media_Start(sigma_handle_t ptInst){
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_media_start(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_Media_Stop(sigma_handle_t ptInst) {
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_media_stop(&param);

	return (Error_Type_e)(ret.common_ret.ret);
	
}

Error_Type_e HalSys_Media_Pause(sigma_handle_t ptInst){
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_media_pause(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_Media_Resume(sigma_handle_t ptInst){
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_media_resume(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_Media_Flush(sigma_handle_t ptInst, FlushMode_e eMode){
	halsys_ret ret;
	media_flush_param param;

	param.instance = (unsigned int)ptInst;
	param.flush_mode = (unsigned char)eMode;
 
	ret = halsys_media_flush(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

//Error_Type_e HalSys_Media_SetParameter(sigma_handle_t ptInst, SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

//Error_Type_e HalSys_Media_GetParameter(sigma_handle_t ptInst, SIGM_PARAMETER_INDEX eParameter, datatype_ptr* pData);


Error_Type_e HalSys_Media_PushFrame(sigma_handle_t ptInst, Media_Buffer_t* pMediaBuffer) {
	halsys_ret ret;
	media_push_param * param = NULL;

	param = (media_push_param *)send_buf[0];
	
	param->instance = (unsigned int)ptInst;
	param->size = pMediaBuffer->nSize;
	param->alloc_size = pMediaBuffer->nAllocLen;
	param->fill_size = pMediaBuffer->nFilledLen;
	param->offset = pMediaBuffer->nOffset;
	param->pts = pMediaBuffer->nTimeStamp;
	param->flags = pMediaBuffer->nFlags;

	if(pMediaBuffer->nAllocLen) memcpy((void *)param->data,pMediaBuffer->pBuffer,pMediaBuffer->nAllocLen);
	ret = halsys_media_pushframe(param);

	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_Media_InstallCallback(CallBackFuncsInfo_t* const pCbInfo, sigm_callback_t pInfoRoutine, void* pUserParam) {
	halsys_ret ret;
	halsys_cb_param * param = (halsys_cb_param *)send_buf[0];

	param->instance = pCbInfo->ptInst;
	param->reason = pCbInfo->InfoID;

	ret = halsys_media_installcb(param);

	pCbInfo->sub_id = ret.cb_ret.subid;

	return (Error_Type_e)(ret.cb_ret.ret);
}

Error_Type_e HalSys_Media_UnInstallCallback(CallBackFuncsInfo_t* pCbInfo) {
	halsys_ret ret;
	halsys_cb_param * param = (halsys_cb_param *)send_buf[0];

	param->instance = pCbInfo->ptInst;
	param->reason = pCbInfo->InfoID;
	param->subid = pCbInfo->sub_id;

	ret = halsys_media_uninstallcb(param);

	return (Error_Type_e)(ret.cb_ret.ret);
}

