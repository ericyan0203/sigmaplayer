#include  "SigmaMediaPlayer.h"
#include  <StrongPointer.h>
#include <String8.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define FILE_PATH 	"d://halsys.avc"
#define SERVER_IP 	"127.0.0.1"
#define SERVER_PORT 1111

int main(int argc, char* argv[])
{

	initialize_string8(); 

	sp<SigmaMediaPlayer> player = new SigmaMediaPlayer(SERVER_IP,SERVER_PORT);

	player->setDataSource(FILE_PATH);

	while(1)
	{
#ifdef WIN32
				Sleep(50);
#else
			 	usleep(50 * US_PER_MS);
#endif
	}
	terminate_string8();
	return 0;
}
