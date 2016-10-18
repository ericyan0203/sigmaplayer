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
#define FILE_PATH "d://halsys/old_town_cross_1080p50_2M.ivf"

//#define FILE_PATH "d://test_es/test.vc1"
//#define FILE_PATH "D://halsys/Seamless/avc/setting.txt"
//#define FILE_PATH "D://AV_S4_video1.avc"
#define SERVER_IP 	"10.86.60.228"
#define SERVER_PORT 52116
#endif

#define DTVTEST

static void callback(void* cookie, int msg, int ext1, int ext2, unsigned int *obj){
	printf("get callback reason %d  cookie %p\n",msg,cookie);
	return;
}
int main(int argc, char* argv[])
{
	void * player = NULL;
	void * dtvInst = NULL;
	void * channelInst = NULL;
	unsigned long long curtime = 0;
	unsigned long long duration = 0;

	halsys_player_init(SERVER_IP,SERVER_PORT);

#ifdef MMTEST
	halsys_media_player_create(FILE_PATH,&player);

	printf("create player %p\n",player);

	halsys_media_player_installcb(player,callback);

	halsys_media_player_start(player);

	while(1) {
#ifdef WIN32
				Sleep(30000);
#else
			 	usleep(50 * US_PER_MS);
#endif
		halsys_media_player_getcurrtime(player,&curtime);
		halsys_media_player_getduration(player,&duration);
		printf("cur time %lld ms duration %lld ms\n",curtime,duration);
	}

	halsys_media_player_stop(player);
	halsys_media_player_destroy(player);
#endif

#ifdef DTVTEST
    halsys_dtv_player_create(&dtvInst);
	halsys_tuner_lock((STACK_ATSC1<<16|TYPE_AIR),0,0,28);
	
    while (1) {
		channel_config_t tConfig;
		tConfig.video_pid = 0x51;
		tConfig.video_format = VIDEO_CodingMPEG12;
		tConfig.audio_pid = 0x54;
		tConfig.audio_format = AUDIO_CodingAC3;
		tConfig.pcr_pid = 0x51;
		
		halsys_dtv_player_start(dtvInst,&tConfig,&channelInst);
		
#ifdef WIN32
				Sleep(10000);
#else
			 	usleep(10000 * US_PER_MS);
#endif
		halsys_dtv_player_stop(channelInst);
		channelInst = NULL;
		
	   	tConfig.video_pid = 0x41;
		tConfig.video_format = VIDEO_CodingMPEG12;
		tConfig.audio_pid = 0x44;
		tConfig.audio_format = AUDIO_CodingAC3;
		tConfig.pcr_pid = 0x41;

		halsys_dtv_player_start(dtvInst,&tConfig,&channelInst);
		
#ifdef WIN32
				Sleep(10000);
#else
			 	usleep(10000 * US_PER_MS);
#endif
		halsys_dtv_player_stop(channelInst);
		channelInst = NULL;
		
    }
	halsys_dtv_player_destroy(dtvInst);
#endif

	halsys_player_deinit();
	return 0;
}