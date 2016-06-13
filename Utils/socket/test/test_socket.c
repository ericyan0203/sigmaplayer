#include "utils_socket.h"
#include <stdio.h>
#define DEBUG_SERVER_PORT 65526

static char *ip_addr = "10.86.17.62";

int main(void)
{
	printf("socket test start\n");

	if(socket_connect(ip_addr, DEBUG_SERVER_PORT, 3000) < 0)
	{
		printf("connect failed\n");
		//return -1;
	}
	else
	{
	
	}

	socket_disconnect();
	while(1);
	return 0;
}
