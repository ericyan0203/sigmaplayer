#ifndef FFMPEG_RV_SC_H_
#define FFMPEG_RV_SC_H_

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"

#include "./ffmpeg_io.h"

#define RV_MAX_SLICE 256   //the slice number should not be above 256 , at least have one
#define RV_GENERIC_SC_LENGTH  12


typedef struct
{
  unsigned int slice_count;
  unsigned int slice_offset[RV_MAX_SLICE];
}VSliceInfo;


int ffmpeg_rv_parse_slice_info(uint8_t * datain,VSliceInfo *info);

int ffmepg_rv_append_sequence_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st);


int ffmpeg_rv_append_picture_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, VSliceInfo * info ,uint32_t size);




int ffmpeg_rv_append_slice(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, VSliceInfo * info,uint8_t* datain,uint32_t size);



#endif

