#include "SIGM_Tuner_API.h"
#include "SIGM_Types.h"
#include "halsys_stub.h"

int HalMisc_Tuner_Lock(HalSys_DemodType_e stacktype, int bandwidth, int symbolrate, int freqkhz) {
	halsys_ret ret;
	halmisc_tuner_param  param;

	param.demod_type = stacktype;
	param.bandwidth =  bandwidth;
	param.symbolrate = symbolrate;
	param.frequence = freqkhz;

	ret = halmisc_tuner_lock(&param);

	return (Error_Type_e)(ret.common_ret.ret);
}
