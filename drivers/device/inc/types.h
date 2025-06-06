/*
 * types.h
 *
 *  Created on: 01 oct. 2022
 *      Author: Ludo
 */

#ifndef __TYPES_H__
#define __TYPES_H__

#ifdef MPMCM
#include "stdint.h"
#endif

/*!******************************************************************
 * \brief Custom variables types.
 *******************************************************************/

typedef char                char_t;

#ifndef MPMCM

typedef signed char         int8_t;
typedef unsigned char       uint8_t;

typedef signed short        int16_t;
typedef unsigned short      uint16_t;

typedef signed int          int32_t;
typedef unsigned int        uint32_t;

#endif

typedef signed long long    int64_t;
typedef unsigned long long  uint64_t;

typedef float               float32_t;
typedef double              float64_t;

#define NULL                ((void*) 0)

#define UNUSED(x)           ((void) x)

#endif /* __TYPES_H__ */
