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

#ifndef _SNMP_H_
#define _SNMP_H_

#include "asn1.h"
#include "list.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

//#define LOGOFF

#ifdef SYSLOG
  #ifdef LOGOFF
    #define CREDO_SNMP_LOG(level, fmt...)
  #else
    #define CREDO_SNMP_LOG(level, fmt...) \
      do { \
        switch (level) { \
          case SNMP_LOG_DEBUG: \
          case SNMP_LOG_INFO: \
          case SNMP_LOG_WARNING: error(fmt); break;\
          case SNMP_LOG_ERROR: die(fmt);     break; \
          default:                           break; \
        } \
      } while (0)
  #endif
#else
  #ifdef LOGOFF
    #define CREDO_SNMP_LOG(level, fmt...)
  #else
    #define CREDO_SNMP_LOG(level, fmt...) \
      do { \
        switch (level) { \
          case SNMP_LOG_DEBUG: \
          case SNMP_LOG_INFO: \
          case SNMP_LOG_WARNING: fprintf(stdout, fmt); break; \
          case SNMP_LOG_ERROR: fprintf(stderr, fmt);   break; \
          default:                                     break; \
        } \
      } while (0)
  #endif
#endif

/* SNMP request type */
#define SNMP_REQ_GET        0xA0
#define SNMP_REQ_GETNEXT    0xA1
#define SNMP_RESP           0xA2
#define SNMP_REQ_SET        0xA3
#define SNMP_REQ_BULKGET    0xA5
#define SNMP_REQ_INF        0xA6
#define SNMP_TRAP           0xA7
#define SNMP_REPO           0xA8

/* vocal information */
typedef enum snmp_log_level {
  SNMP_LOG_ALL,
  SNMP_LOG_DEBUG = 1,
  SNMP_LOG_INFO,
  SNMP_LOG_WARNING,
  SNMP_LOG_ERROR,
  SNMP_LOG_OFF,

  SNMP_LOG_LEVEL_NUM
} SNMP_LOG_LEVEL_E;

/* Error status */
typedef enum snmp_err_stat {
  /* v1 */
  SNMP_ERR_STAT_NO_ERR,
  SNMP_ERR_STAT_TOO_BIG = 1,
  SNMP_ERR_STAT_NO_SUCH_NAME,
  SNMP_ERR_STAT_BAD_VALUE,
  SNMP_ERR_STAT_READ_ONLY,
  SNMP_ERR_STAT_GEN_ERR,

  /* v2c */
  SNMP_ERR_STAT_ON_ACCESS,
  SNMP_ERR_STAT_WRONG_TYPE,
  SNMP_ERR_STAT_WRONG_LEN,
  SNMP_ERR_STAT_ENCODING,
  SNMP_ERR_STAT_WRONG_VALUE,
  SNMP_ERR_STAT_NO_CREATION,
  SNMP_ERR_STAT_INCONSISTENT_VALUE,
  SNMP_ERR_STAT_RESOURCE_UNAVAIL,
  SNMP_ERR_STAT_COMMIT_FAILED,
  SNMP_ERR_STAT_UNDO_FAILED,
  SNMP_ERR_STAT_AUTHORIZATION,
  SNMP_ERR_STAT_NOT_WRITABLE,
  SNMP_ERR_STAT_INCONSISTENT_NAME,
} SNMP_ERR_STAT_E;

/* return code */
typedef enum snmp_err_code {
  SNMP_ERR_OK           = 0,

  SNMP_ERR_PDU_TYPE     = -100,
  SNMP_ERR_PDU_LEN      = -101,
  SNMP_ERR_PDU_REQID    = -102,
  SNMP_ERR_PDU_ERRSTAT  = -103,
  SNMP_ERR_PDU_ERRIDX   = -104,

  SNMP_ERR_VB_TYPE      = -200,
  SNMP_ERR_VB_LEN       = -201,
  SNMP_ERR_VB_VAR       = -202,
  SNMP_ERR_VB_VALUE_LEN = -203,
  SNMP_ERR_VB_OID_LEN   = -204,
} SNMP_ERR_CODE_E;

struct var_bind {
  struct list_head link;

  oid_t *oid;
  uint32_t vb_len;
  uint32_t oid_len;

  uint32_t value_len;
  uint8_t value_type;
  uint8_t value[0];
};

struct pdu_hdr {
  uint8_t pdu_type;
  uint32_t pdu_len;
  /* request id */
  uint32_t req_id_len;
  integer_t req_id;
  /* Error status */
  uint32_t err_stat_len;
  integer_t err_stat;
  /* Error index */
  uint32_t err_idx_len;
  integer_t err_idx;
};

struct snmp_datagram {
  lua_State *lua_state;

  void *recv_buf;
  void *send_buf;

  uint32_t data_len;
  /* version */
  integer_t version;
  uint32_t ver_len;
  /* community */
  octstr_t community[41];
  uint32_t comm_len;

  struct pdu_hdr pdu_hdr;

  uint32_t vb_list_len;

  uint32_t vb_in_cnt;
  uint32_t vb_out_cnt;
  struct list_head vb_in_list;
  struct list_head vb_out_list;
};

void snmp_send_response(struct snmp_datagram *sdg);
void snmp_recv(uint8_t *buffer, int len, void *arg);
void snmpd_send(uint8_t *buf, int len);

#include <stdarg.h>

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

#endif /* _SNMP_H_ */
