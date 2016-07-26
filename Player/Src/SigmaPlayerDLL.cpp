#include  "SigmaPlayerDLL.h"
#include  "SigmaMediaPlayer.h"
#include  <StrongPointer.h>
#include <String8.h>
#include  <Utils.h>
#include  <string.h>
#include  <stdio.h>
#if 0
#define FILE_PATH 	"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116
#endif

sp<SigmaMediaPlayer> player = NULL;

int mPort = 52116;
char mIP[256]= {0};

int InitPlatform(const char * ip, const int port){
	utils_init();
	strncpy(mIP,ip,10);
	mPort = port;
	return 0;
}

int DeInitPlatform() {
	utils_deinit();
	return 0;
}
int  CreatePlayer(const char * url,void** phandle){
	 player = new SigmaMediaPlayer(mIP,mPort);
	 player->setDataSource(url);
	 player->start();

	 *phandle = (void *)player.get();

	 return 0;
}

int  DestroyPlayer(void * phandle) {
	if(phandle == player.get()) {
	 	player->stop();
	 	player.clear();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}


int  PausePlayer(void * phandle) {
	if(phandle == player.get()) {
	 	player->pause();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}

int  ResumePlayer(void * phandle) {
	if(phandle == player.get()) {
	 	player->resume();
	}else {
		printf("handle pointer isn't correct\n");
		return -1;
	}

	 return 0;
}