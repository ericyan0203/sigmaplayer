#ifndef _SIGMA_PLAYER_DLL_H
#define _SIGMA_PLAYER_DLL_H

typedef void (*callback_t)(void* cookie, int msg, int ext1, int ext2, unsigned int *obj);

extern "C"  int __declspec(dllexport)sigma_player_init(const char * ip, const int port);

extern "C"  int __declspec(dllexport)sigma_player_deinit();

extern "C"  int  __declspec(dllexport)sigma_player_create(const char * url ,void** phandle);

extern "C"  int  __declspec(dllexport)sigma_player_destroy(void * phandle);

extern "C"  int  __declspec(dllexport)sigma_player_pause(void * phandle);

extern "C"  int  __declspec(dllexport)sigma_player_resume(void * phandle);

extern "C"  int  __declspec(dllexport)sigma_player_seek(void * phandle,unsigned long long ms);

extern "C"  int  __declspec(dllexport)sigma_player_getduration(void * phandle,unsigned long long * duration);

extern "C"  int  __declspec(dllexport) sigma_player_getcurrtime(void * phandle,unsigned long long * pCurrTime);

extern "C" int  __declspec(dllexport)sigma_player_installcb(void * phandle,callback_t cb);
#endif