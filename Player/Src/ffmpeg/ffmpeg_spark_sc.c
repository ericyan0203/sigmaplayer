#include "ffmpeg_spark_sc.h"
#include <utils/Log.h>


#define LOG_TAG "FfmpeSparkSC"


int ffmepg_spark_append_sequence_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st)
{ 
    unsigned int offset = 0;
	unsigned int framerate = 0;
	unsigned long num = 0;
    /* start code*/
	dataout[0] = 0x00;
	dataout[1] = 0x00;
	dataout[2] = 0x01;
	dataout[3] = 0x31;
	
    /* sequence payload  length */
	dataout[4] = (char)((SPARK_GENERIC_SC_LENGTH)>>16);
	dataout[5] = (char)((SPARK_GENERIC_SC_LENGTH)>>8);
	dataout[6] = 0x4e;
	dataout[7] = (char)((SPARK_GENERIC_SC_LENGTH));
    dataout[8] = 0x39;
	dataout[9] = 1;

	dataout[10] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
    dataout[11] = (st->codec->width &0x00FF); /* width 0:7 */
    dataout[12] =  0x58; /* X*/
    dataout[13] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
    dataout[14] =  (st->codec->height &0x00FF); /* height 0:7 */
    dataout[15] =  0x50; /* height 0:7 */
  
	offset += 16;

	ALOGI(" width %d height %d\n",st->codec->width,st->codec->height );

    return offset;
     	
}

int ffmpeg_spark_append_picture_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint32_t size)
{
    
    unsigned int offset = 0 ,  i = 0;
    unsigned int length = 0;

	length = SPARK_GENERIC_SC_LENGTH + size; //timestamp H/L 8 bytes
    /* start code*/
    dataout[0] = 0x00;
    dataout[1] = 0x00;
    dataout[2] = 0x01;
    dataout[3] = 0x32;
   
    /* sequence payload	length */
    dataout[4] = (char)((length)>>16);
    dataout[5] = (char)((length)>>8);
    dataout[6] = 0x4e;
    dataout[7] = (char)(length);
  
	dataout[8] = 0x39; 
    dataout[9] = 1;

    dataout[10] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
    dataout[11] = (st->codec->width &0x00FF); /* width 0:7 */
    dataout[12] =  0x58; /* X*/
    dataout[13] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
    dataout[14] =  (st->codec->height &0x00FF); /* height 0:7 */
    dataout[15] =  0x50; /* height 0:7 */
 
    offset += 16;

	return offset;
   
}

int ffmpeg_spark_append_slice(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint8_t* datain,uint32_t size)
{
    int offset = 0;
	
    if( datain == NULL || dataout == NULL) return -1;

	memcpy(dataout,datain,size); //copy all of data directly
      
	return size; 
}


