#include "SigmaPlayerDLL.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>

#if 1
#define FILE_PATH 	"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
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

	sigma_player_init(SERVER_IP,SERVER_PORT);
	
	sigma_player_create(FILE_PATH,&player);

	printf("create player %p\n",player);

	sigma_player_installcb(player,callback);

	if(1) {
#ifdef WIN32
				Sleep(250000);
#else
			 	usleep(50 * US_PER_MS);
#endif
	}

	sigma_player_destroy(player);
	sigma_player_deinit();
	return 0;
}