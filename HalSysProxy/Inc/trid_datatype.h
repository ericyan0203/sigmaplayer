#ifndef __TRID_DATATYPE_H__
#define __TRID_DATATYPE_H__

#if defined __cplusplus || defined __cplusplus__
extern "C" {
#endif


typedef unsigned __int64 trid_uint64;
typedef unsigned long trid_uint32;   //!< TODO
typedef unsigned short trid_uint16;  //!< TODO
typedef unsigned char trid_uint8;    //!< TODO
typedef signed __int64 trid_sint64;  //!< TODO

typedef signed long trid_sint32;   //!< TODO
typedef signed short trid_sint16;  //!< TODO
typedef signed char trid_sint8;    //!< TODO
typedef char trid_char;            //!< TODO

#define trid_void void            //!< to avoid C++ compile error.
typedef unsigned char trid_bool;  //!< TODO


#if !defined(NULL)
#define NULL (void*)0  //!< TODO
#endif

#define trid_false (0)           //!< TODO
#define trid_true (!trid_false)  //!< TODO

#define trid_failed (-1)  //!< TODO
#define trid_success (0)  //!< TODO

#if defined __cplusplus || defined __cplusplus__
}
#endif

#endif






