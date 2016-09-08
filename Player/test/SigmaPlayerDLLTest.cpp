#include "SigmaPlayerDLL.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>

#if 1
//#define FILE_PATH 	"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
//#define FILE_PATH 	"d://halsys/bbb_2160p30_VP9.webm"
//#define FILE_PATH 	"d://halsys/RV9_AAC.rmvb"
//#define FILE_PATH 	"d://halsys/HEVC_AAC.mp4"
//#define FILE_PATH 	"d://halsys/HEVC.hevc"
//#define FILE_PATH 	"d://halsys/150528_1_t2_642_HBBTV_EPG_5_service.ts"
//#define FILE_PATH "d://BBC.mp4"
#define FILE_PATH "d://test_es/test.vc1"
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116
#endif
static void callback(void* cookie, int msg, int ext1, int ext2, unsigned int *obj){
	printf("get callback reason %d  cookie %p\n",msg,cookie);
	return;
}
int main(int argc, char* argv[])
{
	void * player = NULL;
	unsigned long long curtime = 0;
	unsigned long long duration = 0;

	halsys_player_init(SERVER_IP,SERVER_PORT);
	
	halsys_media_player_create(FILE_PATH,&player);

	printf("create player %p\n",player);

	halsys_media_player_installcb(player,callback);

	halsys_media_player_start(player);

	while(1) {
#ifdef WIN32
				Sleep(3000);
#else
			 	usleep(50 * US_PER_MS);
#endif
		halsys_media_player_getcurrtime(player,&curtime);
		halsys_media_player_getduration(player,&duration);
		printf("cur time %lld ms duration %lld ms\n",curtime,duration);
	}

	halsys_media_player_stop(player);
	halsys_media_player_destroy(player);
	halsys_player_deinit();
	return 0;
}