#include "SIGM_DigitalTV_API.h"
#include "halsys_stub.h"


Error_Type_e HalSys_DigitalTV_Initialize(void) {
	halsys_ret ret;
	ret = halsys_dtv_initialize();
	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_DigitalTV_Deinit(void){
	halsys_ret ret;
	ret = halsys_dtv_deinit();
	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_DigitalTV_Open(Demux_Config_t* ptDemuxConfig, sigma_handle_t* ptInst) {
	halsys_ret ret;
	dtv_open_param param;

	param.demux_input = ptDemuxConfig->eDemuxInput;
	param.demux_cipath = ptDemuxConfig->eDemuxCIPath;

	ret = halsys_dtv_open(&param);

	*ptInst = (sigma_handle_t)ret.open_ret.instance;

	return (Error_Type_e)(ret.open_ret.ret);
}

Error_Type_e HalSys_DigitalTV_Close(sigma_handle_t ptInst) {
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_dtv_close(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_DigitalTV_Start(sigma_handle_t ptInst, Channel_Config_t* ptChannelConfig, sigma_handle_t* ptChannel) {
	halsys_ret ret;
	dtv_start_param param;

	param.instance = (unsigned int)ptInst;
	param.video_pid = ptChannelConfig->tVideoConfig.nVideoPID;
	param.video_format = ptChannelConfig->tVideoConfig.eVideoFormat;
	param.video_sink = ptChannelConfig->tVideoConfig.eVideoSink;
	param.video_display_mode = ptChannelConfig->tVideoConfig.eVideoDisplayMode;
	param.video_stream_mode = ptChannelConfig->tVideoConfig.eVideoStreamMode;

	param.audio_pid = ptChannelConfig->tAudioConfig.nAudioPID;
	param.audio_format = ptChannelConfig->tAudioConfig.eAudioFormat;
	param.audio_sink = ptChannelConfig->tAudioConfig.eSoundSink;

	param.teletext_pid = ptChannelConfig->tTeletextConfig.nPID;
	param.teletext_pid = ptChannelConfig->tSubtitleConfig.nPID;
	param.pcr_pid = ptChannelConfig->nPcrPID;
	param.low_latency = ptChannelConfig->bLowlatency;
	
	ret = halsys_dtv_start(&param);

	*ptChannel = (sigma_handle_t)ret.open_ret.instance;

	return (Error_Type_e)(ret.open_ret.ret);
}

Error_Type_e HalSys_DigitalTV_Stop(sigma_handle_t ptInst) {
	halsys_ret ret;
	halsys_common_param param;

	param.instance = (unsigned int)ptInst;

	ret = halsys_dtv_stop(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

