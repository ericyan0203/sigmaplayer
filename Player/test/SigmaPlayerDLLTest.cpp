#include "SigmaPlayerDLL.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#if 1
#define FILE_PATH 	"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116
#endif

int main(int argc, char* argv[])
{
	void * player = NULL;	

	InitPlatform(SERVER_IP,SERVER_PORT);
	
	CreatePlayer(FILE_PATH,&player);

	if(1) {
#ifdef WIN32
				Sleep(100000);
#else
			 	usleep(50 * US_PER_MS);
#endif
	}

	DestroyPlayer(player);
	DeInitPlatform();
	return 0;
}