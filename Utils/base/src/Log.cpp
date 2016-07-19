#include <String8.h>
#include <stdio.h>

#ifdef WIN32
#include<windows.h>
#include<winbase.h>
#endif

#define BUF_SIZE 1024

FILE * mDumpFile = NULL;

static void utils_log_default_callback(void* ptr, int level, const char* fmt, va_list vl) {
 
    char *pos;  
    char _n = '\n';  
    char message[BUF_SIZE] = {0};  
    int nMessageLen = 0;  
    int sz;  

#ifdef WIN32
    SYSTEMTIME st;
	String8 strTime;
	GetLocalTime(&st);

	strTime = String8::format("%u:%u:%u:%u",st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
	if(st.wMilliseconds == 99)
		st.wMilliseconds = 99;
	pos = (char *)strTime.string();
#else 	
    now = time(NULL);
    pos = ctime(&now);
#endif
    sz = strlen(pos);  
  //  pos[sz]=']';  
    _snprintf(message, BUF_SIZE, "[%s] ", pos);  
    for (pos = message; *pos; pos++);  
    sz = pos - message;   
    nMessageLen = vsnprintf(pos, BUF_SIZE - sz, fmt, vl);  
 
    if (nMessageLen <= 0) return;  
	
    if (NULL == mDumpFile)  
    {  
        printf("%s", message);  
        return;  
    }  

	printf("%s", message);  
	fwrite(message,strlen(message),1,mDumpFile);
	fflush(mDumpFile);
}

void utils_log_init(const char * path)
{
	if(NULL != path) {
		mDumpFile = fopen(path, "wb+");
	}
}

void utils_log_deinit()
{
	if(NULL != mDumpFile) {
		fclose(mDumpFile);
		mDumpFile = NULL;
	}
		
}

static void (*utils_log_callback)(void*, int, const char*, va_list) = utils_log_default_callback;


void utils_log_set_callback(void (*callback)(void*, int, const char*, va_list))
{
    utils_log_callback = callback;
}

void utils_vlog(void* avcl, int level, const char *fmt, va_list vl)
{
    void (*log_callback)(void*, int, const char*, va_list) = utils_log_callback;
    if (log_callback)
        log_callback(avcl, level, fmt, vl);
}

void utils_log(int level, const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    utils_vlog(NULL, level, fmt, vl);
    va_end(vl);
}



