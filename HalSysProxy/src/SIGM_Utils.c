#include <pthread.h>
#include <SIGM_Utils.h>
#include <stdio.h>

#define MAX_CALLBACKINFO_NUM 20 /**< maximum event number */
#define MAX_CALLBACK_NUMBER 10  /**< maximum callbacks of  event */
#define INVALID_INFO_SUBID 0xFFFFFFFF
#define INVALID_INFOID 0xFEFEFEFE
#define MAX_INSTANCE_NUM  10

typedef struct INFO_CB_ENTRY {
    trid_bool bBroadCast;
    sigm_callback_t Routine;
    void* pUserHandler;
} InfoCbEntry_t;

typedef struct INFO_CALLBACK {
    trid_uint32 InfoID;
    trid_uint32 ulUsedFlag;
    InfoCbEntry_t tEntry[MAX_CALLBACK_NUMBER];
} InfoCallback_t;

typedef struct INFO_HANDLER {
    pthread_mutex_t tInfoLock;
    InfoCallback_t InfoArray[MAX_CALLBACKINFO_NUM];
} InfoHandler_t;

typedef struct INFO_ARRAY {
	sigma_handle_t ptInst;
	InfoHandler_t  tCbInfo;
}InfoArray_t;

InfoArray_t tInfoArray[MAX_INSTANCE_NUM];
pthread_mutex_t   tInfoLock = PTHREAD_MUTEX_INITIALIZER;
static trid_bool bInit = trid_false;

void Utils_InitInfoArray(void) {
	int i = 0 , j = 0;
	
	if(bInit) return;

	bInit = trid_true;

	memset(tInfoArray,0,sizeof(InfoArray_t)*MAX_INSTANCE_NUM);
	
	for(i = 0;i < MAX_INSTANCE_NUM; i++) {
		tInfoArray[i].ptInst = -1LL;
		pthread_mutex_init(&(tInfoArray[i].tCbInfo.tInfoLock), NULL);

		for(j = 0;j < MAX_CALLBACKINFO_NUM; j++) {
			tInfoArray[i].tCbInfo.InfoArray[j].InfoID = INVALID_INFOID;
		}
	}
	return;
}

void Utils_DeinitInfoArray(void) {
	int i = 0;
	
	if(!bInit) return;
	
	bInit = trid_false;

	for(i =0;i < MAX_INSTANCE_NUM; i++) {
		tInfoArray[i].ptInst = -1LL;
		pthread_mutex_destroy(&(tInfoArray[i].tCbInfo.tInfoLock));
	}
	return;	
}

static InfoHandler_t* sf_GetInfoHandlerByInst(sigma_handle_t ptInst) {
	int i = 0;
	InfoHandler_t* pInfoHandler = NULL;

	pthread_mutex_lock(&tInfoLock);
	for(i =0;i < MAX_INSTANCE_NUM; i++) {
		if(tInfoArray[i].ptInst == ptInst) {
			pInfoHandler = &(tInfoArray[i].tCbInfo);
			break;
		}
	}
	pthread_mutex_unlock(&tInfoLock);

	if(NULL != pInfoHandler) return pInfoHandler;

	pthread_mutex_lock(&tInfoLock);
	for(i = 0;i < MAX_INSTANCE_NUM; i++) {
		if(tInfoArray[i].ptInst == -1LL) {
			pInfoHandler = &(tInfoArray[i].tCbInfo);
			tInfoArray[i].ptInst= ptInst;
			break;
		}
	}
	pthread_mutex_unlock(&tInfoLock);

	return pInfoHandler;
}

#if 0
static InfoHandler_t * sf_GetFreeInfoHandler(sigma_handle_t ptInst) {
	int i = 0;
	InfoHandler_t* pInfoHandler = NULL;

	pthread_mutex_lock(&tInfoLock);
	for(i =0;i < MAX_INSTANCE_NUM; i++) {
		if(tInfoArray[i].ptInst == -1LL) {
			pInfoHandler = &(tInfoArray[i].tCbInfo);
			tInfoArray[i] = ptInst;
			break;
		}
	}
	pthread_mutex_unlock(&tInfoLock);
	return pInfoHandler;
}
#endif
static InfoHandler_t * sf_RequestInfoHandler(sigma_handle_t pInst) {
	InfoHandler_t* pInfoHandler = sf_GetInfoHandlerByInst(pInst);

	return pInfoHandler;
}

static void sf_ReleaseInfoHandler(sigma_handle_t pInst) {
	InfoHandler_t* pInfoHandler = NULL;
	InfoArray_t * pArray = NULL;
	int i = 0;

	pthread_mutex_lock(&tInfoLock);
	for(i =0;i < MAX_INSTANCE_NUM; i++) {
		if(tInfoArray[i].ptInst == pInst) {
			pInfoHandler = &(tInfoArray[i].tCbInfo);
			pArray = &tInfoArray[i];
			break;
		}
	}
	pthread_mutex_unlock(&tInfoLock);
	
	if(NULL == pInfoHandler) return;
	
	pArray->ptInst = -1LL;
	return;
}

Error_Type_e Utils_RegisteCallback(CallBackFuncsInfo_t* const pCbInfo, sigm_callback_t pInfoRoutine,
                                        void* pUserParam) {
    trid_uint32 i = 0, j = 0;
    InfoCallback_t* pInfoCb = NULL;
    Error_Type_e ret = SIGM_ErrorFailed;
	InfoHandler_t* pInfoHandler = sf_GetInfoHandlerByInst(pCbInfo->ptInst);

    if (!pInfoHandler || !pCbInfo) return SIGM_ErrorBadParameter;

    pthread_mutex_lock(&pInfoHandler->tInfoLock);
	j = pCbInfo->sub_id;
	
    for (i = 0; i < MAX_CALLBACKINFO_NUM; i++) {
        pInfoCb = &pInfoHandler->InfoArray[i];
        if (pInfoCb->InfoID != INVALID_INFOID) {
            if (pInfoCb->InfoID == pCbInfo->InfoID) {
                if (!GetBits(pInfoCb->ulUsedFlag, j, 1)) {  // try to find one free entry
                    pInfoCb->tEntry[j].Routine = pInfoRoutine;
                    pInfoCb->tEntry[j].bBroadCast = trid_true;
                    pInfoCb->tEntry[j].pUserHandler = pUserParam;
                    SetBits(pInfoCb->ulUsedFlag, j, 1, 1);  // used
                    //pCbInfo->sub_id = j;  //this should be filled by remote
                    ret = SIGM_ErrorNone;
                }else {
              	    printf("the slot has not been released yet\n");
					ret = SIGM_ErrorFailed;
                }
				break;
            }
        } else {
            pInfoCb->InfoID = pCbInfo->InfoID;
            pInfoCb->tEntry[0].Routine = pInfoRoutine;
            pInfoCb->tEntry[0].bBroadCast = trid_true;
            pInfoCb->tEntry[0].pUserHandler = pUserParam;
            SetBits(pInfoCb->ulUsedFlag, j, 1, 1);  // used
            //pCbInfo->sub_id = 0;
            ret = SIGM_ErrorNone;
            break;
        }
    }

    pthread_mutex_unlock(&pInfoHandler->tInfoLock);

    return ret;
}

Error_Type_e Utils_UnregisteCallback(CallBackFuncsInfo_t* const pCbInfo) {
    trid_uint32 i = 0;
    InfoCallback_t* pInfoCb = NULL;
    Error_Type_e ret = SIGM_ErrorFailed;
	InfoHandler_t* pInfoHandler = sf_GetInfoHandlerByInst(pCbInfo->ptInst);

    if (!pInfoHandler || !pCbInfo) return SIGM_ErrorBadParameter;

    pthread_mutex_lock(&pInfoHandler->tInfoLock);

    for (i = 0; i < MAX_CALLBACKINFO_NUM; i++) {
        pInfoCb = &pInfoHandler->InfoArray[i];
        if (pInfoCb->InfoID == pCbInfo->InfoID) {
            if (pCbInfo->sub_id >= MAX_CALLBACK_NUMBER) {
                printf("sub_id is error, we never install this callback, sub_id=%d\n", pCbInfo->sub_id);
                ret = SIGM_ErrorOutOfIndex;
            } else {
                SetBits(pInfoCb->ulUsedFlag, pCbInfo->sub_id, 1, 0);  // free the entry
                if (!GetBits(pInfoCb->ulUsedFlag, 0, MAX_CALLBACK_NUMBER)) {
                    pInfoCb->InfoID = INVALID_INFOID;
					sf_ReleaseInfoHandler(pCbInfo->ptInst);
                }

                pInfoCb->tEntry[pCbInfo->sub_id].Routine = NULL;
                pInfoCb->tEntry[pCbInfo->sub_id].bBroadCast = trid_false;
                ret = SIGM_ErrorNone;
            }
            break;
        }
    }

    pthread_mutex_unlock(&pInfoHandler->tInfoLock);

    return ret;
}

Error_Type_e Utils_InvokeCallback(sigma_handle_t ptInst, SIGM_EVENT_INDEX nIndex, datatype_ptr Param) {
    trid_uint32 i = 0, j = 0;
    InfoCallback_t* pInfoCb = NULL;
    Error_Type_e ret = SIGM_ErrorFailed;
    sigm_callback_t InfoCallback = NULL;
    void* pUserParam = NULL;
    CallBackFuncsInfo_t tCbInfo;
	InfoHandler_t* pInfoHandler = sf_GetInfoHandlerByInst(ptInst);

    if (!pInfoHandler) return SIGM_ErrorBadParameter;

    tCbInfo.InfoID = nIndex;
    tCbInfo.ptInst = ptInst;

    for (i = 0; i < MAX_CALLBACKINFO_NUM; i++) {
        pInfoCb = &(pInfoHandler->InfoArray[i]);

        if (pInfoCb->InfoID == nIndex) {
            for (j = 0; j < MAX_CALLBACK_NUMBER; j++) {
                pthread_mutex_lock(&pInfoHandler->tInfoLock);
                if (GetBits(pInfoCb->ulUsedFlag, j, 1)) {
                    InfoCallback = pInfoCb->tEntry[j].Routine;
                    // bBroadCast = pInfoCb->tEntry[j].bBroadCast;
                    pUserParam = pInfoCb->tEntry[j].pUserHandler;
                    tCbInfo.sub_id = j;
                    InfoCallback(&tCbInfo, Param, pUserParam);
                }
                pthread_mutex_unlock(&pInfoHandler->tInfoLock);
            }
            ret = SIGM_ErrorNone;
            break;
        }
    }

    return ret;
}
