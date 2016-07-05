#include "ffmpeg_rv_sc.h"
#include <utils/Log.h>


#define LOG_TAG "FfmpeRVSC"

#define FOURCC_RV30 MAKE_FOURCC('R','V','3','0') //rv8
#define FOURCC_RV40 MAKE_FOURCC('R','V','4','0') //rv9+10

static int get_slice_offset(const uint8_t *buf, int n)
{   
   int ret = GST_READ_UINT32_LE(buf + n*8 - 4);
   if(ret == 0x00000001){
       return  GST_READ_UINT32_LE(buf + n*8);
   }
   else if( ret == 0x01000000){
       return   GST_READ_UINT32_BE(buf + n*8); 
   }else{
       ALOGI("Invalid ret %x return le anyway \n",ret); 
       return   GST_READ_UINT32_LE(buf + n*8);
   }

   return 0;
}


	

int ffmpeg_rv_parse_slice_info(uint8_t * datain,VSliceInfo *info)
{
      int offset = 0;
      int i = 0;
	  uint8_t * slice_hdr;

	  if(info == NULL || datain == NULL) return -1;

	  info->slice_count = *(uint8_t *)datain + 1;

	  if(info->slice_count>256) return -1;
	  
	  offset += 1;

	  slice_hdr = datain + 5;

	  for(i=0;i<info->slice_count;i++)
	  {
	      info->slice_offset[i] =  get_slice_offset(slice_hdr,i);
		  offset += 8;
	  }

	  return offset; //1+8*slice_cnt
	  
}

int ffmepg_rv_append_sequence_header(uint8_t * dataout,AVFormatContext *s, 
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
	dataout[4] = (unsigned char)((st->codec->chunk_size + RV_GENERIC_SC_LENGTH)>>16);
	dataout[5] = (unsigned char)((st->codec->chunk_size + RV_GENERIC_SC_LENGTH)>>8);
	dataout[6] = 0x4e;
	dataout[7] = (unsigned char)((st->codec->chunk_size + RV_GENERIC_SC_LENGTH));

	if(st->codec->codec_id == AV_CODEC_ID_RV30){
		dataout[8] = 0x28;
	}
	else if(st->codec->codec_id == AV_CODEC_ID_RV40){
		dataout[8] = 0x29;
	}
	else {
		ALOGI("append sequence codec %x why here \n",st->codec->codec_id);
		dataout[8] = 0x00; //should not be here
	}

	dataout[9] = 1;

	dataout[10] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
    dataout[11] = (st->codec->width &0x00FF); /* width 0:7 */
    dataout[12] =  0x58; /* X*/
    dataout[13] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
    dataout[14] =  (st->codec->height &0x00FF); /* height 0:7 */
    dataout[15] =  0x50; /* height 0:7 */
  
	offset += 16;

	ALOGI("mediaobject 0x%x  codec_tag %x width %d height %d\n",st->codec->media_object_format,st->codec->codec_tag,st->codec->width,st->codec->height );

    GST_WRITE_UINT32_BE(dataout+offset, st->codec->chunk_size);
	offset+=4;
	GST_WRITE_UINT32_LE(dataout+offset, st->codec->media_object_format);
	offset+=4;
	GST_WRITE_UINT32_LE(dataout+offset, st->codec->codec_tag);
	offset+=4;        
	GST_WRITE_UINT16_BE(dataout+offset, st->codec->width); 
	offset+=2;
	GST_WRITE_UINT16_BE(dataout+offset, st->codec->height);
	offset+=2;
	GST_WRITE_UINT16_BE(dataout+offset, st->codec->bpp); 
	offset+=2;
	GST_WRITE_UINT16_BE(dataout+offset, st->codec->pad_width);
	offset+=2;
	GST_WRITE_UINT16_BE(dataout+offset, st->codec->pad_height);
	offset+=2;

#if 0
    if (st->r_frame_rate.den)
    {
         num = (unsigned long)(st->r_frame_rate.num * 1000);
         framerate = (int)(num/st->r_frame_rate.den);
    }
    else
    {
         framerate = 0;
    }
#endif

    framerate = st->codec->fps;

	ALOGW("frame rate %x  bpp %x pad %x x %x",st->codec->fps,st->codec->bpp,st->codec->pad_width,st->codec->pad_height);
	
	GST_WRITE_UINT32_BE(dataout+offset, framerate);
	offset += 4;        
	if(st->codec->extradata_size&& st->codec->extradata){
		memcpy(dataout+offset, st->codec->extradata, st->codec->extradata_size);  
		offset+=st->codec->extradata_size;
	}

	ALOGI("chunk size %d offset %d extra %d\n", st->codec->chunk_size,(offset-16),st->codec->extradata_size);
	return offset;
     	
}

int ffmpeg_rv_append_picture_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, VSliceInfo * info ,uint32_t size)
{
    
    unsigned int offset = 0 ,  i = 0;
    unsigned int length = 0;

	length = RV_GENERIC_SC_LENGTH + 20 + 8*info->slice_count;
    /* start code*/
    dataout[0] = 0x00;
    dataout[1] = 0x00;
    dataout[2] = 0x01;
    dataout[3] = 0x32;
   
    /* sequence payload	length */
    dataout[4] = (unsigned char)((length)>>16);
    dataout[5] = (unsigned char)((length)>>8);
    dataout[6] = 0x4e;
    dataout[7] = (unsigned char)(length);

    if(st->codec->codec_id == AV_CODEC_ID_RV30){
  	   dataout[8] = 0x28;
    }
    else if(st->codec->codec_id == AV_CODEC_ID_RV40){
	   dataout[8] = 0x29;
    }
    else {
	   ALOGI("append picture codec %x why here \n",st->codec->codec_id);
	   dataout[8] = 0x00; //should not be here
    }

    dataout[9] = 1;

    dataout[10] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
    dataout[11] = (st->codec->width &0x00FF); /* width 0:7 */
    dataout[12] =  0x58; /* X*/
    dataout[13] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
    dataout[14] =  (st->codec->height &0x00FF); /* height 0:7 */
    dataout[15] =  0x50; /* height 0:7 */
 
    offset += 16;

	GST_WRITE_UINT32_BE(dataout + offset,size);
	offset+=4;
	//some field seems un-touched by malone. fake it.
	GST_WRITE_UINT32_BE(dataout + offset, 0);
	offset+=4;
	GST_WRITE_UINT16_BE(dataout + offset, 0);
	offset+=2;
	GST_WRITE_UINT16_BE(dataout + offset, 0);
	offset+=2;
	GST_WRITE_UINT32_BE(dataout + offset, 0); //set as 0    
	offset+=4;    
	GST_WRITE_UINT32_BE(dataout + offset, info->slice_count); 
	offset+=4; 
	for (i=0; i<info->slice_count; i++)    {        
		GST_WRITE_UINT32_BE(dataout + offset, 0x00000001);        
		offset+=4;        
		GST_WRITE_UINT32_BE(dataout + offset, info->slice_offset[i]); 
	//	ALOGI("slice %d offset %d\n",i,info->slice_offset[i]);
		offset+=4;    
	}

	return offset;
   
}

int ffmpeg_rv_append_slice(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, VSliceInfo * info,uint8_t* datain,uint32_t size)
{
    
    unsigned int offset = 0 , i = 0;
    unsigned int packet_length = 0;
	unsigned int ioffset = 0;

	if(info == NULL || st == NULL || s == NULL || dataout ==NULL) return -1;

	if(info->slice_count == 0 || info->slice_count >= 256) return -1;

	for(i=0;i<info->slice_count;i++){

		if (i == info->slice_count - 1) {		
			packet_length = size - info->slice_offset[i];	 
		}else {	 	
		    packet_length = info->slice_offset[i+1] - info->slice_offset[i];	 
		}

		//ALOGW("slice %d length %x\n",i,packet_length);
        /* start code*/
	    dataout[0 + offset] = 0x00;
	    dataout[1 + offset] = 0x00;
	    dataout[2 + offset] = 0x01;
	    dataout[3 + offset] = 0x33;
	
        /* sequence payload  length */
	    dataout[4 + offset] = (unsigned char)((packet_length + RV_GENERIC_SC_LENGTH)>>16);
	    dataout[5 + offset] = (unsigned char)((packet_length + RV_GENERIC_SC_LENGTH)>>8);
	    dataout[6 + offset] = 0x4e;
	    dataout[7 + offset] = (unsigned char)((packet_length + RV_GENERIC_SC_LENGTH));

	    if(st->codec->codec_id == AV_CODEC_ID_RV30){
		    dataout[8 + offset] = 0x28;
	    }
	    else if(st->codec->codec_id == AV_CODEC_ID_RV40){
		    dataout[8 + offset] = 0x29;
	    }
	    else {
		    ALOGI("append slice codec %x why here \n",st->codec->codec_id);
		    dataout[8 + offset] = 0x00; //should not be here
	    }

	    dataout[9 + offset] = 1;

	    dataout[10 + offset] = (st->codec->width &0xFF00)>>8; /* width 15:8 */
        dataout[11 + offset] = (st->codec->width &0x00FF); /* width 0:7 */
        dataout[12 + offset] =  0x58; /* X*/
        dataout[13 + offset] = (st->codec->height &0xFF00)>>8; /* height 15:8 */
        dataout[14 + offset] =  (st->codec->height &0x00FF); /* height 0:7 */
        dataout[15 + offset] =  0x50; /* height 0:7 */
  
	    offset += 16;

		memcpy(dataout+offset,datain+info->slice_offset[i],packet_length);

		offset += packet_length;
	}

	return offset;
}


