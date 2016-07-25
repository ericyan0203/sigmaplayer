#ifndef _SIGMA_PLAYER_DLL_H
#define _SIGMA_PLAYER_DLL_H

extern "C"  int __declspec(dllexport)InitPlatform(const char * ip, const int port);

extern "C"  int __declspec(dllexport)DeInitPlatform();

extern "C"  int  __declspec(dllexport)CreatePlayer(const char * url ,void** phandle);

extern "C"  int  __declspec(dllexport)DestroyPlayer(void * phandle);

#endif