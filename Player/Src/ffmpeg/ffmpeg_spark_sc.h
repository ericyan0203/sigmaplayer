#ifndef FFMPEG_SPARK_SC_H_
#define FFMPEG_SPARK_SC_H_

#define SPARK_GENERIC_SC_LENGTH  12

#include "avformat.h"
#include "avcodec.h"
#include "avutil.h"

#include "ffmpeg_io.h"



int ffmepg_spark_append_sequence_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st);


int ffmpeg_spark_append_picture_header(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint32_t size);


int ffmpeg_spark_append_slice(uint8_t * dataout,AVFormatContext *s, 
						AVStream *st, uint8_t* datain,uint32_t size);


#endif

