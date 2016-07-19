#include  "SigmaMediaPlayer.h"
#include  <StrongPointer.h>
#include <String8.h>
#include  <Utils.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define FILE_PATH 	"d://[H.264 BP][AC3]-1.mkv"//"d://[H.264 BP][AC3]-1.mkv"//"d://BBC.mp4"//"d://[H.264 HP][AAC].mp4" //"d://halsys.avc" //[H.264 BP][AC3]-1.mkv
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116
#define LOG  "d://test.log"

int main(int argc, char* argv[])
{
	utils_init(LOG);
	
	sp<SigmaMediaPlayer> player = new SigmaMediaPlayer(SERVER_IP,SERVER_PORT);

	player->setDataSource(FILE_PATH);
	player->start();

	while(1)
	{
#ifdef WIN32
				Sleep(1000000);
#else
			 	usleep(50 * US_PER_MS);
#endif
	}
	player->stop();
	player.clear();

	utils_deinit();
	return 0;
}
