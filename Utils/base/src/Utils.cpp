#include <Utils.h>
#include <Log.h>
#include <String8.h>


static inline int utils_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 0x20;
    return c;
}

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


int utils_strncasecmp(const char *a, const char *b, size_t n)
{
    const char *end = a + n;
    uint8_t c1, c2;
    do {
        c1 = utils_tolower(*a++);
        c2 = utils_tolower(*b++);
    } while (a < end && c1 && c1 == c2);
    return c1 - c2;
}

