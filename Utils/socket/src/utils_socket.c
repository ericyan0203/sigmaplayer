#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils_socket.h"

#ifdef	WIN32 
#include <winsock2.h>
#pragma   comment(lib,"ws2_32.lib") 
//include "pthread.h"
#endif

#define INT_DATABUFFER_SIZE (6*1024*1024)


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
static SOCKET _socket_server =  INVALID_SOCKET;
char databuff[INT_DATABUFFER_SIZE]; 

Listener _Listener = NULL;

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

static int online_server(const char *host_name, int port, int dtimeout)
{
    WSADATA wsaData;
	int iResult;
	struct sockaddr_in addrServer;
    int bReuseAddr = 1; 
	unsigned int i = 0;  
    SOCKET sockAccept;    
    struct sockaddr_in addrAccept;    
    int iAcceptLen = sizeof(addrAccept);       
    int iRecvSize;    
        
    struct sockaddr_in addrTemp;    
    int iTempLen;    
    
    fd_set fd; 

	WSAStartup(0x0101,&wsaData);    

	if(_socket_server != INVALID_SOCKET) {
		printf("socket has already established\n");
		return 0;
	}
	
     _socket_server = socket(AF_INET,SOCK_STREAM,0);
	 
    if (INVALID_SOCKET == _socket_server)    
    {    
        printf("Failed to create socket!\n");    
        WSACleanup();    
        return -1;    
    }    
    
    memset(&addrServer,0,sizeof(struct sockaddr_in));    
    addrServer.sin_family = AF_INET;    
    addrServer.sin_port = htons(port);    
    addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);      
      
    iResult = setsockopt(_socket_server,SOL_SOCKET,SO_REUSEADDR,(char *)&bReuseAddr,sizeof(bReuseAddr));    

	if(SOCKET_ERROR == iResult){    
        printf("Failed to set resueaddr socket!\n");    
        WSACleanup();    
        return -1;    
    }      
    
    iResult = bind(_socket_server,(struct sockaddr *)&addrServer,sizeof(addrServer));    
    if (SOCKET_ERROR == iResult)    
    {    
        printf("Failed to bind address!\n");    
        WSACleanup();    
        return -1;    
    }    
    
    if (0 != listen(_socket_server,5))    
    {    
        printf("Failed to listen client!\n");    
        WSACleanup();    
        return -1;    
    }    
       
    FD_ZERO(&fd);    
    FD_SET(_socket_server,&fd);    
      
    printf("Start server...\n");    
    while(1)    
    {    
        fd_set fdOld = fd;    
        iResult = select(0,&fdOld,NULL,NULL,/*&tm*/NULL);    
        if (0 <= iResult)    
        {    
            for(i = 0;i < fd.fd_count; i++)    
            {    
                if (FD_ISSET(fd.fd_array[i],&fdOld))    
                {    
                    if (fd.fd_array[i] == _socket_server)    
                    {    
                        memset(&addrAccept,0,sizeof(addrTemp));    
                        sockAccept = accept(_socket_server,(struct sockaddr *)&addrAccept,&iAcceptLen);    
                        if (INVALID_SOCKET != sockAccept)    
                        {    
                            FD_SET(sockAccept,&fd);    
                            //FD_SET(sockAccept,&fdOld);    
                            printf("%s:%d has connected server!\n",inet_ntoa(addrAccept.sin_addr),    
                                ntohs(addrAccept.sin_port));    
                        }    
                    }    
                    else   
                    {   
                    	cb_param param;
						
                        memset(databuff,0,INT_DATABUFFER_SIZE);    
                        iRecvSize = recv(fd.fd_array[i],(char *)&param,sizeof(cb_param),0);   
						
                        memset(&addrTemp,0,sizeof(addrTemp));    
                        iTempLen = sizeof(addrTemp);    
                        getpeername(fd.fd_array[i],(struct sockaddr *)&addrTemp,&iTempLen);    
                            
                        if (SOCKET_ERROR == iRecvSize)  {    
                            closesocket(fd.fd_array[i]);    
                            FD_CLR(fd.fd_array[i],&fd);    
                            i--;    
                            printf("Failed to recv data ,%s:%d errorcode:%d.\n",    
                                inet_ntoa(addrTemp.sin_addr),ntohs(addrTemp.sin_port),WSAGetLastError());    
                            continue;    
                        }else if( 0 == iRecvSize) {        
                            printf("%s:%d has closed!\n",inet_ntoa(addrTemp.sin_addr),    
                                ntohs(addrTemp.sin_port));    
                                
                            closesocket(fd.fd_array[i]);    
                            FD_CLR(fd.fd_array[i],&fd);    
                            i--;        
                        }    
#if 0      
                        if (0 < iRecvSize)    
                        {        
                            printf("recv %s:%d data:%d\n",inet_ntoa(addrTemp.sin_addr),    
                                ntohs(addrTemp.sin_port),param.size);    
                        }
#endif

						if(param.size > 0) {
							iRecvSize = recv(fd.fd_array[i],databuff,param.size,0); 

							if (SOCKET_ERROR == iRecvSize) {
                            	closesocket(fd.fd_array[i]);    
                            	FD_CLR(fd.fd_array[i],&fd);    
                            	i--;    
                            	printf("Failed to recv data ,%s:%d errorcode:%d.\n",    
                               		inet_ntoa(addrTemp.sin_addr),ntohs(addrTemp.sin_port),WSAGetLastError());    
                            	continue;    
                        	}else if( 0 == iRecvSize) {        
                            	printf("%s:%d has closed!\n",inet_ntoa(addrTemp.sin_addr),    
                               		ntohs(addrTemp.sin_port));    
                                
                            	closesocket(fd.fd_array[i]);    
                            	FD_CLR(fd.fd_array[i],&fd);    
                            	i--;        
                        	}    
						}

						if(_Listener) _Listener(param.inst,param.type,param.size,databuff);
						
                    }       
                }    
            }    
        }    
        else if (SOCKET_ERROR == iResult)    
        {    
            //WSACleanup();     
        //  printf("Faild to select sockt in server!\n");    
            Sleep(100);    
        }    
    }    
    WSACleanup();
	_socket_server = INVALID_SOCKET;
	return 0;
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

	WSAStartup(0x0101,&ws);
#endif

	timeout.tv_sec = dtimeout / 1000;
	timeout.tv_usec = dtimeout % 1000;
	
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

	printf("dtimeout %d\n",dtimeout);

    setsockopt(_socket,   SOL_SOCKET,   SO_SNDTIMEO, (char*)&dtimeout,   sizeof(dtimeout));
    setsockopt(_socket,   SOL_SOCKET,   SO_RCVTIMEO, (char*)&dtimeout,   sizeof(dtimeout));
    
    return 0;
}

int socket_send(packet * pack)
{
    HANDLE hMutex;
    int  nRet;
    unsigned char  *p;
    unsigned int result_length,  count;
	int length;
	
	int failed_exit = 0, failed_recv = 0;
    if (_socket == INVALID_SOCKET) return -1;

    hMutex = Lock("CMDCLT_SEND");
   
	failed_exit = 0;
	
#ifdef SOCK_DEBUG
	printf("send hrd len %d\n",pack->hdr_len);
#endif

	nRet = send(_socket, (const char *)pack->hdr, pack->hdr_len, 0);

	if(nRet < 0)
	{
		printf("send length error %d\n", nRet);
		failed_exit = 1;
		goto ERROR_OUT;
	}

#ifdef SOCK_DEBUG
	printf("send data len %d\n",pack->data_len);
#endif	
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

#ifdef SOCK_DEBUG
	printf("recv ret expected len %d\n",pack->ret_len);
#endif
    while (count > 0)
    {
        length = recv(_socket,(char *)p, count, 0);
#ifdef SOCK_DEBUG
		printf("recv ret length %x\n",length);
#endif
		
        if (length > 0)
        {
            count -= length;
            p += length;
        }
        else
        {	
            printf("recv wsa error %x\n", WSAGetLastError());
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

	_Listener = NULL;

	printf("socket closed\n");
	return;
}

void socket_setListener(Listener func) {
	_Listener = func;
}

