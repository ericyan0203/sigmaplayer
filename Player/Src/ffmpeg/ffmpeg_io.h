#ifndef FFMPEG_IO_H_
#define FFMPEG_IO_H_

#include <stdint.h>

#if 0
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#endif

#define MAKE_FOURCC(a,b,c,d) (uint32_t)((a)|(b)<<8|(c)<<16|(d)<<24)


/* Define PUT and GET functions for unaligned memory */
#define _GST_GET(__data, __idx, __size, __shift) \
	(((uint##__size##_t)(((const uint8_t *) (__data))[__idx])) << (__shift))

#define _GST_PUT(__data, __idx, __size, __shift, __num) \
	(((uint8_t *)(__data))[__idx] = (((uint##__size##_t) (__num)) >> (__shift)) & 0xff)

/**
 * GST_READ_UINT64_BE:
 * @data: memory location
 *
 * Read a 64 bit unsigned integer value in big endian format from the memory buffer.
 */
#define GST_READ_UINT64_BE(data)	(_GST_GET (data, 0, 64, 56) | \
					 _GST_GET (data, 1, 64, 48) | \
					 _GST_GET (data, 2, 64, 40) | \
					 _GST_GET (data, 3, 64, 32) | \
					 _GST_GET (data, 4, 64, 24) | \
					 _GST_GET (data, 5, 64, 16) | \
					 _GST_GET (data, 6, 64,  8) | \
					 _GST_GET (data, 7, 64,  0))

/**
 * GST_READ_UINT64_LE:
 * @data: memory location
 *
 * Read a 64 bit unsigned integer value in little endian format from the memory buffer.
 */
#define GST_READ_UINT64_LE(data)	(_GST_GET (data, 7, 64, 56) | \
					 _GST_GET (data, 6, 64, 48) | \
					 _GST_GET (data, 5, 64, 40) | \
					 _GST_GET (data, 4, 64, 32) | \
					 _GST_GET (data, 3, 64, 24) | \
					 _GST_GET (data, 2, 64, 16) | \
					 _GST_GET (data, 1, 64,  8) | \
					 _GST_GET (data, 0, 64,  0))

/**
 * GST_READ_UINT32_BE:
 * @data: memory location
 *
 * Read a 32 bit unsigned integer value in big endian format from the memory buffer.
 */
#define GST_READ_UINT32_BE(data)	(_GST_GET (data, 0, 32, 24) | \
					 _GST_GET (data, 1, 32, 16) | \
					 _GST_GET (data, 2, 32,  8) | \
					 _GST_GET (data, 3, 32,  0))

/**
 * GST_READ_UINT32_LE:
 * @data: memory location
 *
 * Read a 32 bit unsigned integer value in little endian format from the memory buffer.
 */
#define GST_READ_UINT32_LE(data)	(_GST_GET (data, 3, 32, 24) | \
					 _GST_GET (data, 2, 32, 16) | \
					 _GST_GET (data, 1, 32,  8) | \
					 _GST_GET (data, 0, 32,  0))

/**
 * GST_READ_UINT24_BE:
 * @data: memory location
 *
 * Read a 24 bit unsigned integer value in big endian format from the memory buffer.
 *
 * Since: 0.10.22
 */
#define GST_READ_UINT24_BE(data)	(_GST_GET (data, 0, 32, 16) | \
					 _GST_GET (data, 1, 32,  8) | \
					 _GST_GET (data, 2, 32,  0))

/**
 * GST_READ_UINT24_LE:
 * @data: memory location
 *
 * Read a 24 bit unsigned integer value in little endian format from the memory buffer.
 *
 * Since: 0.10.22
 */
#define GST_READ_UINT24_LE(data)	(_GST_GET (data, 2, 32, 16) | \
					 _GST_GET (data, 1, 32,  8) | \
					 _GST_GET (data, 0, 32,  0))



/**
 * GST_READ_UINT16_BE:
 * @data: memory location
 *
 * Read a 16 bit unsigned integer value in big endian format from the memory buffer.
 */
#define GST_READ_UINT16_BE(data)	(_GST_GET (data, 0, 16,  8) | \
					 _GST_GET (data, 1, 16,  0))

/**
 * GST_READ_UINT16_LE:
 * @data: memory location
 *
 * Read a 16 bit unsigned integer value in little endian format from the memory buffer.
 */
#define GST_READ_UINT16_LE(data)	(_GST_GET (data, 1, 16,  8) | \
					 _GST_GET (data, 0, 16,  0))

/**
 * GST_READ_UINT8:
 * @data: memory location
 *
 * Read an 8 bit unsigned integer value from the memory buffer.
 */
#define GST_READ_UINT8(data)		(_GST_GET (data, 0,  8,  0))


/**
 * GST_WRITE_UINT64_BE:
 * @data: memory location
 * @num: value to store
 *
 * Store a 64 bit unsigned integer value in big endian format into the memory buffer.
 */
#define GST_WRITE_UINT64_BE(data, num)	do { \
					  _GST_PUT (data, 0, 64, 56, num); \
					  _GST_PUT (data, 1, 64, 48, num); \
					  _GST_PUT (data, 2, 64, 40, num); \
					  _GST_PUT (data, 3, 64, 32, num); \
					  _GST_PUT (data, 4, 64, 24, num); \
					  _GST_PUT (data, 5, 64, 16, num); \
					  _GST_PUT (data, 6, 64,  8, num); \
					  _GST_PUT (data, 7, 64,  0, num); \
					} while (0)

/**
 * GST_WRITE_UINT64_LE:
 * @data: memory location
 * @num: value to store
 *
 * Store a 64 bit unsigned integer value in little endian format into the memory buffer.
 */
#define GST_WRITE_UINT64_LE(data, num)	do { \
					  _GST_PUT (data, 0, 64,  0, num); \
					  _GST_PUT (data, 1, 64,  8, num); \
					  _GST_PUT (data, 2, 64, 16, num); \
					  _GST_PUT (data, 3, 64, 24, num); \
					  _GST_PUT (data, 4, 64, 32, num); \
					  _GST_PUT (data, 5, 64, 40, num); \
					  _GST_PUT (data, 6, 64, 48, num); \
					  _GST_PUT (data, 7, 64, 56, num); \
					} while (0)
					

/**
 * GST_WRITE_UINT32_BE:
 * @data: memory location
 * @num: value to store
 *
 * Store a 32 bit unsigned integer value in big endian format into the memory buffer.
 */
#define GST_WRITE_UINT32_BE(data, num)	do { \
					  _GST_PUT (data, 0, 32, 24, num); \
					  _GST_PUT (data, 1, 32, 16, num); \
					  _GST_PUT (data, 2, 32,  8, num); \
					  _GST_PUT (data, 3, 32,  0, num); \
					} while (0)

/**
 * GST_WRITE_UINT32_LE:
 * @data: memory location
 * @num: value to store
 *
 * Store a 32 bit unsigned integer value in little endian format into the memory buffer.
 */
#define GST_WRITE_UINT32_LE(data, num)	do { \
					  _GST_PUT (data, 0, 32,  0, num); \
					  _GST_PUT (data, 1, 32,  8, num); \
					  _GST_PUT (data, 2, 32, 16, num); \
					  _GST_PUT (data, 3, 32, 24, num); \
					} while (0)


/**
 * GST_WRITE_UINT24_BE:
 * @data: memory location
 * @num: value to store
 *
 * Store a 24 bit unsigned integer value in big endian format into the memory buffer.
 *
 * Since: 0.10.22
 */
#define GST_WRITE_UINT24_BE(data, num)	do { \
					  _GST_PUT (data, 0, 32,  16, num); \
					  _GST_PUT (data, 1, 32,  8, num); \
					  _GST_PUT (data, 2, 32,  0, num); \
					} while (0)

/**
 * GST_WRITE_UINT24_LE:
 * @data: memory location
 * @num: value to store
 *
 * Store a 24 bit unsigned integer value in little endian format into the memory buffer.
 *
 * Since: 0.10.22
 */
#define GST_WRITE_UINT24_LE(data, num)	do { \
					  _GST_PUT (data, 0, 32,  0, num); \
					  _GST_PUT (data, 1, 32,  8, num); \
					  _GST_PUT (data, 2, 32,  16, num); \
					} while (0)

/**
	 * GST_WRITE_UINT16_BE:
	 * @data: memory location
	 * @num: value to store
	 *
	 * Store a 16 bit unsigned integer value in big endian format into the memory buffer.
*/
#define GST_WRITE_UINT16_BE(data, num)	do { \
						  _GST_PUT (data, 0, 16,  8, num); \
						  _GST_PUT (data, 1, 16,  0, num); \
						} while (0)
	
	/**
	 * GST_WRITE_UINT16_LE:
	 * @data: memory location
	 * @num: value to store
	 *
	 * Store a 16 bit unsigned integer value in little endian format into the memory buffer.
	 */
#define GST_WRITE_UINT16_LE(data, num)	do { \
						  _GST_PUT (data, 0, 16,  0, num); \
						  _GST_PUT (data, 1, 16,  8, num); \
						} while (0)
	
	/**
	 * GST_WRITE_UINT8:
	 * @data: memory location
	 * @num: value to store
	 *
	 * Store an 8 bit unsigned integer value into the memory buffer.
	 */
#define GST_WRITE_UINT8(data, num)	do { \
						  _GST_PUT (data, 0,  8,  0, num); \
						} while (0)



#ifndef AV_RL32
#   define AV_RL32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[3] << 24) |    \
               (((const uint8_t*)(x))[2] << 16) |    \
               (((const uint8_t*)(x))[1] <<  8) |    \
                ((const uint8_t*)(x))[0])
#endif

#ifndef AV_WL32
#   define AV_WL32(p, darg) do {                \
        unsigned d = (darg);                    \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
    } while(0)
#endif


#endif

