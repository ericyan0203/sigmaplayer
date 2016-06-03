#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils_socket.h"

#ifdef	WIN32 
#include <winsock2.h>
#pragma   comment(lib,"ws2_32.lib") 
#endif

#ifdef	__linux

#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <linux/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

typedef int SOCKET;  
#define SOCKET_ERROR  (-1)
#define INVALID_SOCKET  0  
#define HANDLE pthread_mutex_t*  
#endif

static SOCKET _socket = INVALID_SOCKET;

static HANDLE Lock(char* name)
{
    HANDLE mutex = NULL;
    //mutex = CreateMutex(NULL, FALSE, name);
    //WaitForSingleObject(mutex, INFINITE);
    return mutex;
}

static int Unlock(HANDLE mutex)
{
    //ReleaseMutex(mutex);
    //CloseHandle(mutex);
    return 1;
}

int socket_connect(const char *host_name, int port, int dtimeout)
{
    struct sockaddr_in  host_addr;
    int                 opt_val = 1;
    int                 opt_len = sizeof(opt_val);
    unsigned long       mode;
    int ret = 0;
    struct timeval      timeout;
    fd_set              set;
	
#ifdef	WIN32
	WSADATA ws;
#endif

			 
    memset(&host_addr, 0, sizeof(host_addr));

    host_addr.sin_family           = AF_INET;
#ifdef	WIN32 
    host_addr.sin_addr.S_un.S_addr = inet_addr(host_name);
#endif

#ifdef __linux
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
#endif

    host_addr.sin_port             = htons((short)port);

#ifdef	WIN32
    if (_socket != INVALID_SOCKET)
    {
    	printf("socket has already established\n");
		return 0;
		//closesocket(_socket);
    }
	
	WSAStartup(0x0101,&ws);
#endif

#ifdef __linux
	if (_socket != INVALID_SOCKET)
	{
		printf("socket has already established\n");
		return 0;
		//close(_socket);
	}
#endif


    _socket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt_val, opt_len);
    setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&opt_val, opt_len);
    mode = 1;
#ifdef	WIN32 
    ioctlsocket(_socket, FIONBIO, &mode);
#endif

#ifdef __linux
    fcntl(_socket,F_SETFL, O_NONBLOCK);
#endif

    ret = connect(_socket, (struct sockaddr*)&host_addr, sizeof(host_addr));
    printf("connect ret %d\n", ret);
	
#ifdef	WIN32 
    if (-1 == ret)
    {
		FD_ZERO(&set);
		FD_SET(_socket, &set);
		timeout.tv_sec = dtimeout / 1000;
		timeout.tv_usec = dtimeout % 1000;

		if (select(0, 0, &set, 0, &timeout) <= 0)
		{
			mode = 0;
#ifdef	WIN32 
			ioctlsocket(_socket, FIONBIO, &mode);
			closesocket(_socket);

#endif
			
#ifdef __linux
			fcntl(_socket,F_SETFL, O_NONBLOCK);
			close(_socket);
#endif
			printf("connect failed here\n");
			_socket = INVALID_SOCKET;
			return -1;
		}
	}
#endif

    mode = 0;
#ifdef	WIN32 
    ioctlsocket(_socket, FIONBIO, &mode);
#endif
	
#ifdef __linux
    fcntl(_socket,F_SETFL, O_NONBLOCK);
#endif

    setsockopt(_socket,   SOL_SOCKET,   SO_SNDTIMEO, (char*)&dtimeout,   sizeof(dtimeout));
    setsockopt(_socket,   SOL_SOCKET,   SO_RCVTIMEO, (char*)&dtimeout,   sizeof(dtimeout));
    
    return 0;
}

int socket_send(packet * pack)
{
    HANDLE hMutex;
    int  nRet;
    unsigned char  *p;
    unsigned int result_length, length, count;
	int failed_exit = 0, failed_recv = 0;
    if (_socket == INVALID_SOCKET) return -1;

    hMutex = Lock("CMDCLT_SEND");
   
	failed_exit = 0;
    
	nRet = send(_socket, (const char *)pack->hdr, pack->hdr_len, 0);

	if(nRet < 0)
	{
		printf("send length error %d\n", nRet);
		failed_exit = 1;
		goto ERROR_OUT;
	}

	nRet = send(_socket, (const char *)pack->data, pack->data_len, 0);
	if(nRet < 0)
	{
		printf("send data error %d\n", nRet);
		failed_exit = 1;
		goto ERROR_OUT;
	}
    
    result_length = pack->ret_len;
    p = (unsigned char*)pack->ret;
    count = result_length;

    while (count > 0)
    {
        length = recv(_socket,(char *)p, count, 0);

        if (length > 0)
        {
            count -= length;
            p += length;
        }
        else
        {
			failed_recv = 1;
            goto ERROR_OUT;
        }
    }

    Unlock(hMutex);
    return result_length;

ERROR_OUT:
    /* do not need result */
    if ( failed_exit || failed_recv)
    {
        Unlock(hMutex);
    }

	return -1;
}

void socket_disconnect(void)
{
#ifdef	WIN32
    closesocket(_socket);
 
	 WSACleanup();
#endif

#ifdef __linux
	close(_socket);
#endif

	_socket = INVALID_SOCKET;

	printf("socket closed\n");
	return;
}
