#ifndef FFMPEG_VP68_SC_H_
#define FFMPEG_VP68_SC_H_

#define VP68_GENERIC_SC_LENGTH  12

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"

#include "./ffmpeg_io.h"



int ffmepg_vp68_append_sequence_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st);


int ffmpeg_vp68_append_picture_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint32_t size);



int ffmpeg_vp68_append_slice(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint8_t* datain,uint32_t size);


#endif

