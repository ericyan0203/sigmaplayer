#include  "SigmaMediaPlayer.h"
#include  <StrongPointer.h>
#include <String8.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define FILE_PATH 	"d://halsys.avc"
#define SERVER_IP 	"10.86.62.6"
#define SERVER_PORT 52116

int main(int argc, char* argv[])
{

	initialize_string8(); 

	sp<SigmaMediaPlayer> player = new SigmaMediaPlayer(SERVER_IP,SERVER_PORT);

	player->setDataSource(FILE_PATH);
	player->start();

	if(1)
	{
#ifdef WIN32
				Sleep(30000);
#else
			 	usleep(50 * US_PER_MS);
#endif
	}
	player->stop();
	player.clear();
	terminate_string8();
	return 0;
}
