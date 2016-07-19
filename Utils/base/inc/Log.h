#ifndef   _UTILS_LOG_H 
#define   _UTILS_LOG_H 

#include <stdarg.h>


#define AV_DUMP_ERROR  			0
#define AV_DUMP_WARNING  		1
#define AV_DUMP_DEBUG			2
#define AV_DUMP_INFO 			3
#define AV_DUMP_LOG				4

#if 0 //seems VC doesn't support __VA_ARGS__
#define AV_ERROR(fmt,args...)	 	utils_log(NULL,AV_DUMP_ERROR , fmt, args)

#define AV_WARNING(fmt,args...) 	utils_log(NULL,AV_DUMP_WARNING,fmt,args)

#define AV_DEBUG(fmt,args...) 		utils_log(NULL,AV_DUMP_DEBUG , fmt,args)

#define AV_INFO(fmt,args...) 		utils_log(NULL,AV_DUMP_ERROR , fmt,args)

#define AV_LOG(fmt,args...) 		utils_log(NULL,AV_DUMP_ERROR , fmt,args)
#endif



void utils_log_init(const char * path = NULL);

void utils_log_deinit();

void utils_log_set_callback(void (*callback)(void*, int, const char*, va_list));

void utils_log(int level, const char *fmt, ...);


#endif

