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
  SNMP_ERR_STAT_NO_ACCESS,
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
  SNMP_ERR_OK                      = 0,

  SNMP_ERR_VERSION                 = -100,

  SNMP_ERR_GLOBAL_DATA_LEN         = -200,
  SNMP_ERR_GLOBAL_ID               = -201,
  SNMP_ERR_GLOBAL_SIZE             = -202,
  SNMP_ERR_GLOBAL_FLAGS            = -203,
  SNMP_ERR_GLOBAL_MODEL            = -204,

  SNMP_ERR_SECURITY_STR            = -300,
  SNMP_ERR_SECURITY_SEQ            = -301,
  SNMP_ERR_SECURITY_ENGINE_ID      = -302,
  SNMP_ERR_SECURITY_ENGINE_ID_LEN  = -303,
  SNMP_ERR_SECURITY_ENGINE_BOOTS   = -304,
  SNMP_ERR_SECURITY_ENGINE_TIME    = -305,
  SNMP_ERR_SECURITY_USER_NAME      = -306,
  SNMP_ERR_SECURITY_USER_NAME_LEN  = -307,
  SNMP_ERR_SECURITY_AUTH_PARA      = -308,
  SNMP_ERR_SECURITY_AUTH_PARA_LEN  = -309,
  SNMP_ERR_SECURITY_PRIV_PARA      = -310,
  SNMP_ERR_SECURITY_PRIV_PARA_LEN  = -311,

  SNMP_ERR_SCOPE_PDU_SEQ           = -400,

  SNMP_ERR_CONTEXT_ID              = -500,
  SNMP_ERR_CONTEXT_ID_LEN          = -501,
  SNMP_ERR_CONTEXT_NAME            = -502,
  SNMP_ERR_CONTEXT_NAME_LEN        = -503,

  SNMP_ERR_PDU_TYPE                = -600,
  SNMP_ERR_PDU_LEN                 = -601,
  SNMP_ERR_PDU_REQID               = -602,
  SNMP_ERR_PDU_ERRSTAT             = -603,
  SNMP_ERR_PDU_ERRIDX              = -604,

  SNMP_ERR_VB_LIST_SEQ             = -700,
  SNMP_ERR_VB_SEQ                  = -701,
  SNMP_ERR_VB_OID_TYPE             = -702,
  SNMP_ERR_VB_VAR                  = -703,
  SNMP_ERR_VB_VALUE_LEN            = -704,
  SNMP_ERR_VB_OID_LEN              = -705,
} SNMP_ERR_CODE_E;

struct var_bind {
  struct list_head link;

  oid_t *oid;
  uint32_t vb_len;
  uint32_t oid_len;

  /* Number of elements as vb_in,
   * number of bytes as vb_out. */
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
  void *recv_buf;
  void *send_buf;

  uint32_t data_len;
  /* version */
  integer_t version;
  uint32_t ver_len;
  /* global data */
  uint32_t msg_len;
  integer_t msg_id;
  uint32_t msg_id_len;
  integer_t msg_max_size;
  uint32_t msg_size_len;
  octstr_t msg_flags;
  uint32_t msg_flags_len;
  integer_t msg_security_model;
  uint32_t msg_model_len;
  /* security parameter */
  uint32_t secur_str_len;
  uint32_t secur_para_len;
  octstr_t engine_id[33];
  uint32_t engine_id_len;
  integer_t engine_boots;
  uint32_t engine_boots_len;
  integer_t engine_time;
  uint32_t engine_time_len;
  octstr_t user_name[41];
  uint32_t user_name_len;
  octstr_t auth_para[41];
  uint32_t auth_para_len;
  octstr_t priv_para[41];
  uint32_t priv_para_len;
  /* context */
  uint32_t scope_len;
  octstr_t context_id[33];
  uint32_t context_id_len;
  octstr_t context_name[41];
  uint32_t context_name_len;
  /* PDU */
  struct pdu_hdr pdu_hdr;
  /* varbind */
  uint32_t vb_list_len;
  uint32_t vb_in_cnt;
  uint32_t vb_out_cnt;
  struct list_head vb_in_list;
  struct list_head vb_out_list;
};

extern struct snmp_datagram snmp_datagram;

uint32_t ber_value_enc_try(const void *value, uint32_t len, uint8_t type);
uint32_t ber_value_enc(const void *value, uint32_t len, uint8_t type, uint8_t *buf);
uint32_t ber_length_enc_try(uint32_t value);
uint32_t ber_length_enc(uint32_t value, uint8_t *buf);

uint32_t ber_value_dec_try(const uint8_t *buf, uint32_t len, uint8_t type);
uint32_t ber_value_dec(const uint8_t *buf, uint32_t len, uint8_t type, void *value);
uint32_t ber_length_dec_try(const uint8_t *buf);
uint32_t ber_length_dec(const uint8_t *buf, uint32_t *value);

void snmpd_recv(uint8_t *buf, int len);

void snmp_get(struct snmp_datagram *sdg);
void snmp_getnext(struct snmp_datagram *sdg);
void snmp_set(struct snmp_datagram *sdg);
void snmp_bulkget(struct snmp_datagram *sdg);
void snmp_response(struct snmp_datagram *sdg);
#endif /* _SNMP_H_ */
