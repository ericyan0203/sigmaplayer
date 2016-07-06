#include "ffmpeg_vp68_sc.h"

#define MEDIA_OBJ_FORMAT 0x444b4946	
#define FOURCC_VP68      0x56503830



int ffmepg_vp68_append_sequence_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st)
{ 
    unsigned int offset = 0;
	unsigned int framerate = 0;
	unsigned long num = 0;
	unsigned int vp6 = 0;
	unsigned int payload_length = 0;

	printf("codec Id %x\n",st->codec->codec_id); 
	if(st->codec->codec_id == AV_CODEC_ID_VP6 || st->codec->codec_id == AV_CODEC_ID_VP6F) vp6 = 1;

	if(vp6) payload_length = 0;
	else payload_length = 32;
    /* start code*/
	dataout[0] = 0x00;
	dataout[1] = 0x00;
	dataout[2] = 0x01;
	dataout[3] = 0x31;
	
    /* sequence payload  length */
	dataout[4] = (char)((payload_length + VP68_GENERIC_SC_LENGTH)>>16);
	dataout[5] = (char)((payload_length + VP68_GENERIC_SC_LENGTH)>>8);
	dataout[6] = 0x4e;
	dataout[7] = (char)((payload_length + VP68_GENERIC_SC_LENGTH));
    dataout[8] = 0x36;
	dataout[9] = 1;

	dataout[10] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
    dataout[11] = (st->codec->width &0x00FF); /* width 0:7 */
    dataout[12] =  0x58; /* X*/
    dataout[13] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
    dataout[14] =  (st->codec->height &0x00FF); /* height 0:7 */
    dataout[15] =  0x50; /* height 0:7 */
  
	offset += 16;
    
	printf(" width %d height %d\n",st->codec->width,st->codec->height );

	if(!vp6){

	    GST_WRITE_UINT32_BE(dataout+offset,MEDIA_OBJ_FORMAT);
	    offset+=4;
	    GST_WRITE_UINT32_LE(dataout+offset,0);
	    offset+=4;
	    GST_WRITE_UINT32_LE(dataout+offset, FOURCC_VP68); //missing in the start code doc
	    offset+=4;        
	    GST_WRITE_UINT16_BE(dataout+offset, st->codec->width); 
	    offset+=2;
	    GST_WRITE_UINT16_BE(dataout+offset, st->codec->height);
	    offset+=2;

	    for(num =0;num < 8; num++)
	    {
	        GST_WRITE_UINT16_BE(dataout+offset, 0);
		    offset+=2;
	    }
	}
	return offset;
     	
}

int ffmpeg_vp68_append_picture_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint32_t size)
{

    
    unsigned int offset = 0 ,  i = 0;
    unsigned int length = 0;
    unsigned int vp6 = 0;

	if(st->codec->codec_id == AV_CODEC_ID_VP6 || st->codec->codec_id == AV_CODEC_ID_VP6F) vp6 = 1;

	if(vp6) length = VP68_GENERIC_SC_LENGTH + size;
	else length = VP68_GENERIC_SC_LENGTH + 8 + size; //timestamp H/L 8 bytes
	
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
  
	dataout[8] = 0x36; 
    dataout[9] = 1;

    dataout[10] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
    dataout[11] = (st->codec->width &0x00FF); /* width 0:7 */
    dataout[12] =  0x58; /* X*/
    dataout[13] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
    dataout[14] =  (st->codec->height &0x00FF); /* height 0:7 */
    dataout[15] =  0x50; /* height 0:7 */
 
    offset += 16;
	
    if(!vp6){
	    GST_WRITE_UINT32_BE(dataout + offset,0);  //timestamp is zero
	    offset+=4;
	    //some field seems un-touched by malone. fake it.
	    GST_WRITE_UINT32_BE(dataout + offset, 0);
	    offset+=4;
    }

	return offset;
   
}

int ffmpeg_vp68_append_slice(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint8_t* datain,uint32_t size)
{
    int offset = 0;
	
    if( datain == NULL || dataout == NULL) return -1;

	memcpy(dataout,datain,size); //copy all of data directly
      
	return size; 

}


