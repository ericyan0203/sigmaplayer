/*   ISO   C9x     7.18     Integer   types   <stdint.h> 
  *   Based   on   ISO/IEC   SC22/WG14   9899   Committee   draft   (SC22   N2794) 
  * 
  *     THIS   SOFTWARE   IS   NOT   COPYRIGHTED 
  * 
  *     Contributor:   Danny   Smith   <danny_r_smith_2001@yahoo.co.nz> 
  * 
  *     This   source   code   is   offered   for   use   in   the   public   domain.   You   may 
  *     use,   modify   or   distribute   it   freely. 
  * 
  *     This   code   is   distributed   in   the   hope   that   it   will   be   useful   but 
  *     WITHOUT   ANY   WARRANTY.   ALL   WARRANTIES,   EXPRESS   OR   IMPLIED   ARE   HEREBY 
  *     DISCLAIMED.   This   includes   but   is   not   limited   to   warranties   of 
  *     MERCHANTABILITY   or   FITNESS   FOR   A   PARTICULAR   PURPOSE. 
  * 
  *     Date:   2000-12-02 
  */ 


#ifndef   _UTILS_H 
#define   _UTILS_H 

#include   <stddef.h> 
#include   <Log.h>
/*self define*/
typedef signed long long ssize_t; 
typedef  signed int status_t;

#ifndef va_copy 
# ifdef __va_copy 
# define va_copy(DEST,SRC) __va_copy((DEST),(SRC)) 
# else 
# define va_copy(DEST, SRC) memcpy((&DEST), (&SRC), sizeof(va_list)) 
# endif 
#endif  

typedef _int64 off64_t;
typedef unsigned int uid_t;

#if defined(_MSC_VER)
#define strcasecmp   stricmp
#define strncasecmp  strnicmp 
#endif

void utils_init(const char * path = NULL);
void utils_deinit();
#endif 