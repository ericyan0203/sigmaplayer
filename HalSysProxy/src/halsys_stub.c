#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include  <string.h>
#include  <stdio.h>
#include "halsys_stub.h"
#include "utils_socket.h"
#include "pthread.h"

#if 0 
#pragma comment(lib, "pthreadVC2.lib") 
//include "pthread.h"
#endif

#define MAX_DATA_LEN (6*1024*1024)
#define MAX_HDR_LEN   (1024)

#define DOMAIN_NAME "HalSys"

#define ENTER() do{\
	pthread_mutex_lock(&lock);}while(0)

#define EXIT()  do { \
	pthread_mutex_unlock(&lock);return;}while(0)

#if defined ( WIN32 )
#define ROUTINE __FUNCTION__
#else
#define ROUTINE __func__
#endif

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static char hdr_buf[MAX_HDR_LEN];
static char data_buf[2][MAX_DATA_LEN];
static char result_buf[MAX_HDR_LEN];

#if 0
#define BUILD_HDR(header,size,domain) 									\
	do {																\
			header.length = size;										\
			strncpy((header.domain_name),domain,HALSYS_NAME_LENGTH);	\
	}while(0)

#define BUILD_DATA(data,size,routine) 								\
	do {															\	 
			data.param_len = size;									\
			strncpy((data.func_name),routine, HALSYS_NAME_LENGTH);	\
	}while(0)
#endif
static __inline void build_msg(packet* p,unsigned int hdr_len,char * hdrbuf,unsigned int data_len,char * databuf,unsigned int ret_len,char * retbuf) {
     p->hdr_len = hdr_len; 
	 p->hdr = hdrbuf; 
	 p->data_len = data_len; 
	 p->data = databuf; 
	 p->ret_len = ret_len;
	 p->ret = retbuf;
	 return;
} 

static __inline void send_msg(packet * msg)  {
	ENTER();
	socket_send(msg);
	EXIT();
} 

static __inline halsys_ret halsys_param_fixed_stub(const char * routine,void * arg, unsigned int size)
{
	packet packet;
	halsys_ret ret;
	halsys_packet_header packet_header;
	static halsys_packet_data   packet_data;
	unsigned int length = size;
	unsigned int payload_size = size - sizeof(halsys_packet_data);

	packet_header.length = length;
	strncpy(packet_header.domain_name,DOMAIN_NAME,HALSYS_NAME_LENGTH);	

	packet_data.param_len = payload_size;	
	strncpy(packet_data.func_name,routine, HALSYS_NAME_LENGTH);
	//BUILD_HDR(packet_header,length ,DOMAIN_NAME);
	//BUILD_DATA(packet_data,payload_size,(ROUTINE));
	
	memcpy((void *)data_buf[0],(void *)&packet_data,sizeof(halsys_packet_data));
	
	if(payload_size) 
		memcpy((void *)(data_buf[0] + sizeof(halsys_packet_data)),(void *)arg,payload_size);
	
	build_msg(&packet,sizeof(halsys_packet_header),(char *)&packet_header,length,data_buf[0],sizeof(halsys_ret),(char *)&ret);

	send_msg(&packet);

	return ret;
}

static __inline halsys_ret  halsys_cb_routine(const char * routine,halsys_cb_param * arg) {
	packet packet;
	halsys_ret ret;
	halsys_packet_header packet_header;
	halsys_packet_data	packet_data;
	char * buffer = data_buf[0];
	unsigned int length = sizeof(halsys_packet_data) + sizeof(halsys_cb_param);
		
	packet_header.length = length;
	strncpy(packet_header.domain_name,DOMAIN_NAME,HALSYS_NAME_LENGTH);	
	
	packet_data.param_len = sizeof(halsys_cb_param);	
	strncpy(packet_data.func_name,routine, HALSYS_NAME_LENGTH);
		
	memcpy((void *)buffer,(void *)&packet_data,sizeof(halsys_packet_data));
	memcpy((void *)(buffer + sizeof(halsys_packet_data)),(void *)arg,sizeof(halsys_cb_param));
		
	build_msg(&packet,sizeof(halsys_packet_header),&packet_header,length,buffer,sizeof(halsys_ret),(char *)&ret);
	
	send_msg(&packet);

	return ret;
}
//*************************************************************
// Function
//*************************************************************
halsys_ret halsys_initialize(halsys_init_param * arg) {
	unsigned int length = sizeof(halsys_init_param) + sizeof(halsys_packet_data);
	printf("length %d\n",length);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_deinit(void) {
	unsigned int length = sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,NULL, length);
}

halsys_ret halsys_media_initialize(void) {
	unsigned int length = sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,NULL, length);
}

halsys_ret halsys_media_deinit(void) {
	unsigned int length = sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,NULL, length);
}

halsys_ret halsys_media_open(media_open_param * arg) {
	unsigned int length = sizeof(media_open_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_media_close(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_media_start(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_media_stop(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_media_pause(halsys_common_param* arg){
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_media_resume(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_media_flush(media_flush_param* arg) {
	unsigned int length = sizeof(media_flush_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_media_pushframe(media_push_param * arg) {
	packet packet;
	halsys_ret ret;
	halsys_packet_header packet_header;
	static halsys_packet_data   packet_data;
	char * buffer = NULL;
	unsigned int length = sizeof(halsys_packet_data) + sizeof(media_push_param)+ arg->alloc_size;

	buffer = data_buf[0];
	
	packet_header.length = length;
	strncpy(packet_header.domain_name,DOMAIN_NAME,HALSYS_NAME_LENGTH);	

	packet_data.param_len = sizeof(media_push_param)+ arg->alloc_size;	
	strncpy(packet_data.func_name,ROUTINE, HALSYS_NAME_LENGTH);
	//BUILD_HDR(packet_header,length ,DOMAIN_NAME);
	//BUILD_DATA(packet_data,sizeof(halsys_common_param),ROUTINE);
	
	memcpy((void *)buffer,(void *)&packet_data,sizeof(halsys_packet_data));
	memcpy((void *)(buffer + sizeof(halsys_packet_data)),(void *)arg,sizeof(media_push_param));
	if(arg->alloc_size > 0) memcpy((void *)(buffer + sizeof(halsys_packet_data)+sizeof(media_push_param)),(void *)arg->data,arg->alloc_size);
	
	build_msg(&packet,sizeof(halsys_packet_header),&packet_header,length,buffer,sizeof(halsys_ret),(char *)&ret);

	send_msg(&packet);

	return ret;
}

halsys_ret halsys_media_installcb(halsys_cb_param * arg) {
	unsigned int length = sizeof(halsys_cb_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}	

halsys_ret halsys_media_uninstallcb(halsys_cb_param * arg) {
	unsigned int length = sizeof(halsys_cb_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_hdmi_initialize(void) {
	unsigned int length = sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,NULL, length);
}

halsys_ret halsys_hdmi_deinit(void) {
	unsigned int length = sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,NULL, length);
}

halsys_ret halsys_hdmi_open(hdmi_open_param* arg) {
	unsigned int length = sizeof(hdmi_open_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_hdmi_close(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_hdmi_start(hdmi_start_param* arg) {
	unsigned int length = sizeof(hdmi_start_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_hdmi_stop(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_dtv_initialize(void) {
	unsigned int length = sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,NULL, length);
}

halsys_ret halsys_dtv_deinit(void) {
	unsigned int length = sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,NULL, length);
}

halsys_ret halsys_dtv_open(dtv_open_param* arg) {
	unsigned int length = sizeof(dtv_open_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_dtv_close(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_dtv_start(dtv_start_param* arg) {
	unsigned int length = sizeof(dtv_start_param) + sizeof(halsys_packet_data);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}

halsys_ret halsys_dtv_stop(halsys_common_param* arg) {
	unsigned int length = sizeof(halsys_packet_data)+ sizeof(halsys_common_param);
	return halsys_param_fixed_stub(ROUTINE,(void *)arg, length);
}
