#include  "SigmaPlayerDLL.h"
#include  "SigmaMediaPlayer.h"
#include  <StrongPointer.h>
#include <String8.h>
#include  <Utils.h>
#include  <string.h>
#include  <stdio.h>
#include  <utils_socket.h>
#include "SIGM_Utils.h"
#if 0
#define FILE_PATH 	"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116
#endif

sp<SigmaMediaPlayer> player = NULL;

int mPort = 52116;
char mIP[256]= {0};

int sigma_player_init(const char * ip, const int port){
	utils_init("./sigma.log");
	Utils_InitInfoArray();
	
	strncpy(mIP,ip,10);
	mPort = port;

	socket_connect(ip,port, 3000);
	socket_setListener((Observer)Utils_InvokeCallback);
	client_server_start();
	return 0;
}

int sigma_player_deinit() {
	utils_deinit();
	Utils_DeinitInfoArray();
	socket_disconnect();
	client_server_stop();
	return 0;
}


int  sigma_player_create(const char * url,void** phandle){
	 player = new SigmaMediaPlayer();
	 player->setDataSource(url);
	 player->start();

	 *phandle = (void *)player.get();

	 return 0;
}

int  sigma_player_destroy(void * phandle) {
	if(phandle == player.get()) {
	 	player->stop();
	 	player.clear();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}


int  sigma_player_pause(void * phandle) {
	if(phandle == player.get()) {
	 	player->pause();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}

int  sigma_player_resume(void * phandle) {
	if(phandle == player.get()) {
	 	player->resume();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}


int sigma_player_seek(void * phandle,unsigned long long ms) {
	if(phandle == player.get()) {
	 	player->seekTo(ms);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}

int sigma_player_getduration(void * phandle,unsigned long long * duration) {
	if(phandle == player.get()) {
	 	player->getParameter(MEDIA_DURATION,(void *)duration);
		utils_log(AV_DUMP_ERROR,"Duration %lld\n",*duration);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}
	 
	 return 0;
}

int sigma_player_getcurrtime(void * phandle,unsigned long long * pCurrTime) {
	if(phandle == player.get()) {
	 	player->getParameter(MEDIA_CURRENTTIME,(void *)pCurrTime);
		utils_log(AV_DUMP_ERROR,"curtime %lld\n",*pCurrTime);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}
	 
	 return 0;
	
}
int sigma_player_installcb(void * phandle,callback_t cb)
{
	if(phandle == player.get()) {
	 	player->setNotifyCallback(phandle,(notify_callback_t)cb);
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}
	 
	 return 0;
}