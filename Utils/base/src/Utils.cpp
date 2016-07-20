#include <Utils.h>
#include <Log.h>
#include <String8.h>


void utils_init(const char * path) 
{
	
	initialize_string8(); 

	utils_log_init(path);
}

void utils_deinit()
{
	terminate_string8();
	utils_log_deinit();
}

