#ifndef __UTILS_SOCKET__
#define __UTILS_SOCKET__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct{
	unsigned int  hdr_len;
 	unsigned char * hdr;
	unsigned int  data_len;//payload
	unsigned char * data;
	unsigned int  ret_len;  
	unsigned char * ret;  //filled by  socket
}packet;

int socket_connect(const char *host_name, int port, int dtimeout);
int socket_send(packet * pack);
void socket_disconnect(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
