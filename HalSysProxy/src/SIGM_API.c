#include "SIGM_API.h"
#include "halsys_stub.h"
#include <string.h>

Error_Type_e HalSys_Initialize(BoardConfig_t* ptBoardConfig) {
	halsys_ret ret;
    halsys_init_param param;

	strncpy(param.audio_path,ptBoardConfig->pAudioPathXML,PATHLEN);
	strncpy(param.display_path, ptBoardConfig->pDisplayFilePath,PATHLEN);
	strncpy(param.sound_path, ptBoardConfig->pSoundBin,PATHLEN);
	
    ret = halsys_initialize(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}

Error_Type_e HalSys_Deinit(void){
	halsys_ret ret;
	ret = halsys_deinit(NULL);
	return (Error_Type_e)(ret.common_ret.ret);
}	

//Error_Type_e HalSys_Suspend(void);

//Error_Type_e HalSys_SetParameter(SIGM_PARAMETER_INDEX eParameter, datatype_ptr pData);

//Error_Type_e HalSys_GetParameter(SIGM_PARAMETER_INDEX eParameter, datatype_ptr* pData);
