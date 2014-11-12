/*
 * This file is part of SmartSNMP
 * Copyright (C) 2014, Credo Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdarg.h>

#define alloc_nr(x) (((x) + 2) * 3 / 2)
#define uint_sizeof(n) ((n + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1))

#define HTON16(x) NTOH16(x)
#define HTON32(x) NTOH32(x)

#define NTOH16(x) ((uint16_t)( \
  (((uint16_t)(x) & 0x00ff) << 8) | \
  (((uint16_t)(x) & 0xff00) >> 8)))

#define NTOH32(x) ((uint32_t)( \
  (((uint32_t)(x) & 0x000000ffU) << 24) | \
  (((uint32_t)(x) & 0x0000ff00U) << 8) | \
  (((uint32_t)(x) & 0x00ff0000U) >> 8) | \
  (((uint32_t)(x) & 0xff000000U) >> 24)))

#define NTOH64(x) ((uint64_t)( \
  (((uint64_t)(x) & 0x00000000000000ffU) << 56) | \
  (((uint64_t)(x) & 0x000000000000ff00U) << 40) | \
  (((uint64_t)(x) & 0x0000000000ff0000U) << 24) | \
  (((uint64_t)(x) & 0x00000000ff000000U) << 8) | \
  (((uint64_t)(x) & 0x000000ff00000000U) >> 8) | \
  (((uint64_t)(x) & 0x0000ff0000000000U) >> 24) | \
  (((uint64_t)(x) & 0x00ff000000000000U) >> 40) | \
  (((uint64_t)(x) & 0xff00000000000000U) >> 56)))

//#define LOGOFF

#ifdef SYSLOG
  #ifdef LOGOFF
    #define SMARTSNMP_LOG(level, fmt...)
  #else
    #define SMARTSNMP_LOG(level, fmt...) \
      do { \
        switch (level) { \
          case L_DEBUG: \
          case L_INFO: \
          case L_WARNING: error(fmt); break;\
          case L_ERROR: die(fmt);     break; \
          default:                           break; \
        } \
      } while (0)
  #endif
#else
  #ifdef LOGOFF
    #define SMARTSNMP_LOG(level, fmt...)
  #else
    #define SMARTSNMP_LOG(level, fmt...) \
      do { \
        switch (level) { \
          case L_DEBUG: \
          case L_INFO: \
          case L_WARNING: fprintf(stdout, fmt); break; \
          case L_ERROR: fprintf(stderr, fmt);   break; \
          default:                                     break; \
        } \
      } while (0)
  #endif
#endif

/* vocal information */
typedef enum smartsnmp_log_level {
  L_ALL,
  L_DEBUG = 1,
  L_INFO,
  L_WARNING,
  L_ERROR,
  L_OFF,
  L_LEVEL_NUM
} SMARTSNMP_LOG_LEVEL_E;

static inline void
report(const char *prefix, const char *err, va_list params)
{
  fputs(prefix, stderr);
  vfprintf(stderr, err, params);
  fputs("\n", stderr);
}

static inline void
usage(const char *err)
{
  fprintf(stderr, "usage: %s\n", err);
  exit(1);
}

static inline void
die(const char *err, ...)
{
  va_list params;

  va_start(params, err);
  report("fatal: ", err, params);
  va_end(params);
  exit(1);
}

static inline int
error(const char *err, ...)
{
  va_list params;

  va_start(params, err);
  report("error: ", err, params);
  va_end(params);
  return -1;
}

static inline void *
xmalloc(int size)
{
  void *ret = malloc(size);
  if (!ret)
    die("Out of memory, malloc failed");
  return ret;
}

static inline void *
xrealloc(void *ptr, int size)
{
  void *ret = realloc(ptr, size);
  if (!ret)
    die("Out of memory, realloc failed");
  return ret;
}

static inline void *
xcalloc(int nr, int size)
{
  void *ret = calloc(nr, size);
  if (!ret)
    die("Out of memory, calloc failed");
  return ret;
}

#endif /* _UTIL_H_ */
