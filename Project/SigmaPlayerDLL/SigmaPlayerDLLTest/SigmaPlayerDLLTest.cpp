#include "SigmaPlayerDLL.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifndef WIN32  
#define Sleep(x) usleep(x*1000); 
#endif


#if 1
#define FILE_PATH 	"d://BBC.mp4"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116
#endif

int main(int argc, char* argv[])
{
	void * player = NULL;	

	InitPlatform(SERVER_IP,SERVER_PORT);
	
	CreatePlayer(FILE_PATH,&player);

	if(1) {

		Sleep(10000);
	  //  FlushPlayer(player,30000);

		Sleep(20000);

	   FlushPlayer(player,2000);
	   Sleep(2000);
	   //FlushPlayer(player,4000);

	//	ResumePlayer(player);	
//
//	   FlushPlayer(player,300000);

		Sleep(10000);
		
	 //	FlushPlayer(player,90000);
	//	while(1)
		{
			Sleep(30000);
		}

	}

	DestroyPlayer(player);
	DeInitPlatform();
	return 0;
}