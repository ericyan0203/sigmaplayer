#include "HalSysClient.h"
#include "SIGM_API.h"
#include <utils_socket.h>

static bool bGlbInit = false;
static uint32_t bRef = 0;

#define SOUND_BIN_PATH "/opt/Misc/sound_preset.bin"
#define AUDIO_PATH_XML "/opt/Misc/audiopath_cfg.xml"
#define DISPLAY_CONFIG_PATH "/opt/"

static Error_Type_e sigma_test_init(void) {
    BoardConfig_t tBoardConfig;
    Error_Type_e ret = SIGM_ErrorNone;

    memset(&tBoardConfig, 0, sizeof(BoardConfig_t));

    tBoardConfig.pSoundBin = SOUND_BIN_PATH;
    tBoardConfig.pAudioPathXML = AUDIO_PATH_XML;
    tBoardConfig.pDisplayFilePath = DISPLAY_CONFIG_PATH;
    ret = HalSys_Initialize(&tBoardConfig);
    return ret;
}

static Error_Type_e sigma_test_deinit(void) { 
		return SIGM_ErrorNotImplemented; 
}

HalSysClient::HalSysClient()
		:mIP(String8()),
		 mPort(-1){
}

HalSysClient::~HalSysClient() {

}

Error_Type_e HalSysClient::connect(String8 & ip, uint32_t port){

	if(bGlbInit == true)
	{
		bRef++;
		return SIGM_ErrorNone;
	}

	mIP = ip;
	mPort = port;
	
	socket_connect(mIP.string(),port,3000);

    return sigma_test_init();
}

Error_Type_e HalSysClient::disconnect() {
	if(bGlbInit == false) 
		return SIGM_ErrorInvalidState;

	bRef--;

	if(bRef == 0)
	{
		sigma_test_deinit();
		socket_disconnect();
	}

	return SIGM_ErrorNone;
}
