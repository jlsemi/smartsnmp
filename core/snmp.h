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
#include "transport.h"

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
  SNMP_ERR_VB_VAR       = -201,
  SNMP_ERR_VB_VALUE_LEN = -202,
  SNMP_ERR_VB_OID_LEN   = -203,
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
  struct transport_operation *trans_ops;

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

extern struct snmp_datagram snmp_datagram;

uint32_t ber_value_enc_test(const void *value, uint32_t len, uint8_t type);
uint32_t ber_value_enc(const void *value, uint32_t len, uint8_t type, uint8_t *buf);
uint32_t ber_length_enc_test(uint32_t value);
uint32_t ber_length_enc(uint32_t value, uint8_t *buf);

uint32_t ber_value_dec_test(const uint8_t *buf, uint32_t len, uint8_t type);
uint32_t ber_value_dec(const uint8_t *buf, uint32_t len, uint8_t type, void *value);
uint32_t ber_length_dec_test(const uint8_t *buf);
uint32_t ber_length_dec(const uint8_t *buf, uint32_t *value);

void snmp_send_response(struct snmp_datagram *sdg);
void snmpd_recv(uint8_t *buffer, int len);
void snmpd_send(uint8_t *buf, int len);

void snmpd_init(int port);
int snmpd_open(void);
void snmpd_run(void);
int snmpd_mib_node_reg(const oid_t *grp_id, int id_len, int grp_cb);
int snmpd_mib_node_unreg(const oid_t *grp_id, int id_len);

#endif /* _SNMP_H_ */
