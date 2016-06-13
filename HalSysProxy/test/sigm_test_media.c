#include <stdio.h>
#include <errno.h>

#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "SIGM_Types.h"
#include "sigm_test_internal.h"

HalSysConfig_t tConfig;
sigma_handle_t pMediaHandle;

trid_uint8 data[DATA_UNIT_SIZE];

#define SOUND_BIN_PATH "/opt/Misc/sound_preset.bin"
#define AUDIO_PATH_XML "/opt/Misc/audiopath_cfg.xml"
#define DISPLAY_CONFIG_PATH "/opt/"
#define HOST_IP		 "10.86.62.6"
#define HOST_PORT	 52116

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

static Error_Type_e sigma_test_media_init(void) {
    Error_Type_e ret = SIGM_ErrorNone;
    ret = HalSys_Media_Initialize();
    return ret;
}

static Error_Type_e sigma_test_open_file(const char* fileName) {
    Error_Type_e ret = SIGM_ErrorNone;
    const char* file = tConfig.pInput;
    FILE* fp = NULL;

    if (file != NULL && (fp = fopen(file, "rb")) != NULL) {
        TRID_INFO("Open file ok! file:%s\n", file);
    } else if (!file) {
        TRID_ERROR("Can't Open file %s error %s\n", file, strerror(errno));
        fp = NULL;
        ret = SIGM_ErrorNotReady;
    } else {
        TRID_ERROR("Encounter NULL file name !\n");
        fp = NULL;
        ret = SIGM_ErrorBadParameter;
    }

    tConfig.fp = fp;
    return ret;
}

static trid_uint32 sigma_test_read_file(FILE* fp, pMedia_Buffer_t pBuffer) {
    trid_uint32 nSize = 0;

    if ((NULL == fp) || (NULL == pBuffer->pBuffer) || (0 == pBuffer->nAllocLen)) {
        TRID_ERROR("Invalid Parameter \n");
        return -1;
    }

    pBuffer->nAllocLen = DATA_UNIT_SIZE;
    pBuffer->nOffset = 0;
    pBuffer->nSize = 0;
    pBuffer->nTimeStamp = -1ULL;

    nSize = fread(pBuffer->pBuffer, 1, DATA_UNIT_SIZE, fp);

    if (0 != nSize) {
        pBuffer->nSize = nSize;
        pBuffer->nFilledLen = nSize;
    }
   // TRID_ERROR("Size %x\n", nSize);
    return nSize;
}

static Error_Type_e sigma_test_media_open(void) {
    Error_Type_e ret = SIGM_ErrorNone;
    Media_Config_t tMediaConfig;
    tMediaConfig.bLowLatency = trid_false;
    tMediaConfig.eClockMode = CLOCK_MODE_TIMER;

    tMediaConfig.tAudioConfig.eAudioFormat = SIGM_AUDIO_CodingUnused;
    tMediaConfig.tAudioConfig.eSoundSink = 0;

    tMediaConfig.tVideoConfig.eVideoDisplayMode = SIGM_SEAMLESS_NONE;
    tMediaConfig.tVideoConfig.eVideoFormat = tConfig.eVideoType;
    tMediaConfig.tVideoConfig.eVideoPlayMode = SIGM_PLAYMODE_NORMAL;
    tMediaConfig.tVideoConfig.eVideoStreamMode = SIGM_STREAMMODE_NORMAL;
    tMediaConfig.tVideoConfig.eVideoSink = SIGM_VIDEO_SINK_MP;
	tMediaConfig.tVideoConfig.eMuxType = SIGM_MUX_PES;
    ret = HalSys_Media_Open(&tMediaConfig, &pMediaHandle);

    return ret;
}

static Error_Type_e sigma_test_media_close(void) { return HalSys_Media_Close(pMediaHandle); }

static Error_Type_e sigma_test_push_data(sigma_handle_t ptInst, pMedia_Buffer_t pBuffer) { return HalSys_Media_PushFrame(ptInst, pBuffer); }

static Error_Type_e sigma_test_media_start(void) { return HalSys_Media_Start(pMediaHandle); }

static Error_Type_e sigma_test_media_stop(void) { return HalSys_Media_Stop(pMediaHandle); }

static Error_Type_e sigma_test_common_deinit(void) { return SIGM_ErrorNotImplemented; }

trid_uint32 bGdb = trid_true;

int main(int argc, char* argv[]) {
    int size = 0;
    Error_Type_e ret = SIGM_ErrorNone;
    Media_Buffer_t tBuffer;
	int file_size = 0;

    memset(&tBuffer, 0, sizeof(Media_Buffer_t));

    tBuffer.pBuffer = data;
    tBuffer.nAllocLen = DATA_UNIT_SIZE;

	tConfig.eVideoType = SIGM_VIDEO_CodingAVC;
	tConfig.pInput = "d://halsys.avc";

    // Get input params
#if 0
    if (!ParseParam(argc, argv)) {
        PrintUsage();
        return -1;
    }
#endif

    // while(bGdb);

    bGdb = trid_false;

	socket_connect(HOST_IP,HOST_PORT,3000);

    ret = sigma_test_init();
    if (SIGM_ErrorNone != ret) {
        TRID_ERROR("sigma_test_init error %x\n", ret);
        return -1;
    }

    ret = sigma_test_media_init();
    if (SIGM_ErrorNone != ret) {
        TRID_ERROR("sigma_test_media_init error %x\n", ret);
        return -1;
    }

    ret = sigma_test_open_file(tConfig.pInput);
    if (SIGM_ErrorNone != ret) {
        TRID_ERROR("sigma_test_open_file error %x\n", ret);
        return -1;
    }

    ret = sigma_test_media_open();
    if (SIGM_ErrorNone != ret) {
        TRID_ERROR("sigma_test_media_open error %x\n", ret);
        return -1;
    }

    ret = sigma_test_media_start();
    if (SIGM_ErrorNone != ret) {
        TRID_ERROR("sigma_test_media_start error %x\n", ret);
        return -1;
    }

    do {
        size = sigma_test_read_file(tConfig.fp, &tBuffer);
        tBuffer.nFlags = SIGM_BUFFERFLAG_VIDEO_BL | SIGM_BUFFERFLAG_BLOCKCALL;
        tBuffer.nFilledLen = tBuffer.nSize = size;
		file_size += size;
        if (size > 0) {
            do {
                ret = sigma_test_push_data(pMediaHandle, &tBuffer);
               // TRID_ERROR("ret %x\n", ret);
#ifdef WIN32
				Sleep(50);
#else
			 	usleep(50 * US_PER_MS);
#endif
            } while (ret == SIGM_ErrorInsufficientResources);
        }

    } while (size > 0);

    if (size == 0) {
        tBuffer.nFlags = SIGM_BUFFERFLAG_VIDEO_BL | SIGM_BUFFERFLAG_BLOCKCALL | SIGM_BUFFERFLAG_ENDOFSTREAM;
        tBuffer.nFilledLen = tBuffer.nSize = 0;
        ret = sigma_test_push_data(pMediaHandle, &tBuffer);

        TRID_ERROR("notify eos ret %x file size %d\n", ret,file_size);
    }
	
	Sleep(60000);
#if 0
    while (1) {
       #ifdef WIN32
			Sleep(10);
		#else
			 usleep(10 * US_PER_MS);
		#endif
    }
#endif
    ret = sigma_test_media_stop();
    if (SIGM_ErrorNone != ret) {
        TRID_ERROR("sigma_test_media_init error %x\n", ret);
        return -1;
    }

    ret = sigma_test_media_close();
    if (SIGM_ErrorNone != ret) {
        TRID_ERROR("sigma_test_media_open error %x\n", ret);
        return -1;
    }

    return 0;
}
