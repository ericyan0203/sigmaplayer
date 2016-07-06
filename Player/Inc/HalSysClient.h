#ifndef _HALSYS_CLIENT_
#define _HALSYS_CLIENT_

#include <String8.h>
#include "SIGM_Types.h"

class HalSysClient {
public:
   	HalSysClient();
    virtual ~HalSysClient();
	
    Error_Type_e connect(String8 & ip, uint32_t port);
    Error_Type_e disconnect();

private:
	String8 mIP;
	uint32_t mPort;
    HalSysClient(const HalSysClient &);
    HalSysClient &operator=(const HalSysClient &);
};
#endif
