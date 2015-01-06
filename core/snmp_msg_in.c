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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mib.h"
#include "snmp.h"
#include "util.h"

static struct err_msg_map snmp_err_msg[] = {
  { SNMP_ERR_OK, "Every thing is OK!" },

  { SNMP_ERR_VERSION, "SNMP version tag should be integer!" },

  { SNMP_ERR_GLOBAL_DATA_LEN, "SNMP global data tag should be sequence!" },
  { SNMP_ERR_GLOBAL_ID, "SNMP message id tag should be integer!" },
  { SNMP_ERR_GLOBAL_SIZE, "SNMP message max size tag should be integer!" },
  { SNMP_ERR_GLOBAL_FLAGS, "SNMP message flags tag should be octect string!" },
  { SNMP_ERR_GLOBAL_MODEL, "SNMP security model tag should be integer!" },

  { SNMP_ERR_SECURITY_STR, "SNMP security string tag should be octect string!" },
  { SNMP_ERR_SECURITY_SEQ, "SNMP security tag should be sequence!" },
  { SNMP_ERR_SECURITY_ENGINE_ID, "SNMP engine id tag should be integer!" },
  { SNMP_ERR_SECURITY_ENGINE_ID_LEN, "SNMP engine id length should not exceed 32 bytes!" },
  { SNMP_ERR_SECURITY_ENGINE_BOOTS, "SNMP engine boots tag should be integer!" },
  { SNMP_ERR_SECURITY_ENGINE_TIME, "SNMP engine time tag should be integer!" },
  { SNMP_ERR_SECURITY_USER_NAME, "SNMP user name tag should be octect string!" },
  { SNMP_ERR_SECURITY_USER_NAME_LEN, "SNMP user name length exceeds!" },
  { SNMP_ERR_SECURITY_AUTH_PARA, "SNMP authoritative parameter tag should be octect string!" },
  { SNMP_ERR_SECURITY_AUTH_PARA_LEN, "SNMP authoritative parameter length exceeds!" },
  { SNMP_ERR_SECURITY_PRIV_PARA, "SNMP privative parameter tag should be octect string!" },
  { SNMP_ERR_SECURITY_PRIV_PARA_LEN, "SNMP privative parameter length exceeds!" },

  { SNMP_ERR_SCOPE_PDU_SEQ, "SNMP scope PDU tag should be sequence!" },

  { SNMP_ERR_CONTEXT_ID, "SNMP context id tag should be octect string!" },
  { SNMP_ERR_CONTEXT_ID_LEN, "SNMP context id length should not exceed 32 bytes!" },
  { SNMP_ERR_CONTEXT_NAME, "SNMP context name tag should be octect string!" },
  { SNMP_ERR_CONTEXT_NAME_LEN, "SNMP context name length exceeds!" },

  { SNMP_ERR_PDU_TYPE, "SNMP PDU tag should be sequence!" },
  { SNMP_ERR_PDU_LEN, "SNMP PDU length not match!" },
  { SNMP_ERR_PDU_REQID, "SNMP PDU request id tag should be integer!" },
  { SNMP_ERR_PDU_ERRSTAT, "SNMP PDU error status tag should be integer!" },
  { SNMP_ERR_PDU_ERRIDX, "SNMP PDU error index tag should be integer!" },

  { SNMP_ERR_VB_LIST_SEQ, "SNMP varbind list tag should be sequence!" },
  { SNMP_ERR_VB_SEQ, "SNMP varbind tag should be sequence!" },
  { SNMP_ERR_VB_OID_TYPE, "SNMP varbind oid tag should be object identifier!" },
  { SNMP_ERR_VB_VAR, "SNMP varbind allocation fail!" },
  { SNMP_ERR_VB_VALUE_LEN, "SNMP varbind value length exceeds!" },
  { SNMP_ERR_VB_OID_LEN, "SNMP varbind oid length exceeds!" },
};

static struct var_bind *
vb_new(uint32_t oid_len, uint32_t val_len)
{
  struct var_bind *vb = xmalloc(sizeof(*vb) + val_len);
  vb->oid = xmalloc(oid_len * sizeof(oid_t));
  return vb;
}

static void
vb_delete(struct var_bind *vb)
{
  free(vb->oid);
  free(vb);
}

static void
snmp_vb_list_free(struct list_head *vb_list)
{
  struct list_head *pos, *n;

  list_for_each_safe(pos, n, vb_list) {
    struct var_bind *vb = list_entry(pos, struct var_bind, link);
    list_del(&vb->link);
    vb_delete(vb);
  }
}

static void
snmp_datagram_clear(struct snmp_datagram *sdg)
{
  snmp_vb_list_free(&sdg->vb_in_list);
  snmp_vb_list_free(&sdg->vb_out_list);
  memset(sdg, 0, sizeof(*sdg));
  INIT_LIST_HEAD(&sdg->vb_in_list);
  INIT_LIST_HEAD(&sdg->vb_out_list);
}

/* Alloc buffer for var bind decoding */
static struct var_bind *
var_bind_alloc(uint8_t *buf, enum snmp_err_code *err)
{
  struct var_bind *vb;
  uint8_t oid_type, val_type;
  uint32_t oid_len, oid_dec_len, val_len;
  uint8_t *buf1;

  /* OID */
  oid_type = *buf++;
  if (oid_type != ASN1_TAG_OBJID) {
    *err = SNMP_ERR_VB_OID_TYPE;
    return NULL;
  }
  buf += ber_length_dec(buf, &oid_len);
  buf1 = buf;
  buf += oid_len;

  /* OID length decoding, keep from overflow. */
  oid_dec_len = ber_value_dec_try(buf1, oid_len, ASN1_TAG_OBJID);
  if (oid_dec_len > MIB_OID_MAX_LEN) {
    *err = SNMP_ERR_VB_OID_LEN;
    return NULL;
  }

  /* Value */
  val_type = *buf++;
  buf += ber_length_dec(buf, &val_len);
  if (val_len > MIB_VALUE_MAX_LEN) {
    *err = SNMP_ERR_VB_VALUE_LEN;
    return NULL;
  }

  /* Varbind allocation */
  vb = vb_new(oid_dec_len, val_len);
  if (vb == NULL) {
    *err = SNMP_ERR_VB_VAR;
    return NULL;
  }

  /* vb->oid_len is the actual length of oid */
  vb->oid_len = ber_value_dec(buf1, oid_len, ASN1_TAG_OBJID, vb->oid);

  /* Value */
  vb->value_type = val_type;
  vb->value_len = val_len;
  memcpy(vb->value, buf, val_len);

  *err = SNMP_ERR_OK;
  return vb;
}

/* Parse PDU header */
static SNMP_ERR_CODE_E
pdu_hdr_parse(struct snmp_datagram *sdg, uint8_t **buffer)
{
  SNMP_ERR_CODE_E err;
  struct pdu_hdr *ph;
  uint8_t *buf;

  err = SNMP_ERR_OK;
  buf = *buffer;
  ph = &sdg->pdu_hdr;

  ph->pdu_type = *buf++;
  buf += ber_length_dec(buf, &ph->pdu_len);

  /* Request ID */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_PDU_REQID;
    return err;
  }
  buf += ber_length_dec(buf, &ph->req_id_len);
  ber_value_dec(buf, ph->req_id_len, ASN1_TAG_INT, &ph->req_id);
  buf += ph->req_id_len;

  /* Error status */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_PDU_ERRSTAT;
    return err;
  }
  buf += ber_length_dec(buf, &ph->err_stat_len);
  ber_value_dec(buf, ph->err_stat_len, ASN1_TAG_INT, &ph->err_stat);
  buf += ph->err_stat_len;

  /* Error index */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_PDU_ERRIDX;
    return err;
  }
  buf += ber_length_dec(buf, &ph->err_idx_len);
  ber_value_dec(buf, ph->err_idx_len, ASN1_TAG_INT, &ph->err_idx);
  buf += ph->err_idx_len;

  *buffer = buf;
  return err;
}

/* Parse varbind */
static SNMP_ERR_CODE_E
var_bind_parse(struct snmp_datagram *sdg, uint8_t **buffer)
{
  SNMP_ERR_CODE_E err;
  struct var_bind *vb;
  uint8_t *buf;
  uint32_t vb_len, len_len;
  const uint32_t tag_len = 1;

  err = SNMP_ERR_OK;
  buf = *buffer;

  /* Varbind sequence length */
  if (*buf++ != ASN1_TAG_SEQ) {
    err = SNMP_ERR_VB_LIST_SEQ;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->vb_list_len);

  while (sdg->vb_list_len > 0) {
    /* check vb_list type */
    if (*buf++ != ASN1_TAG_SEQ) {
      err = SNMP_ERR_VB_SEQ;
      break;
    }
    len_len = ber_length_dec(buf, &vb_len);
    buf += len_len;

    /* Alloc a new var_bind and add into var_bind list. */
    vb = var_bind_alloc(buf, &err);
    if (vb == NULL) {
      break;
    }
    list_add_tail(&vb->link, &sdg->vb_in_list);
    sdg->vb_in_cnt++;

    buf += vb_len;
    sdg->vb_list_len -= tag_len + len_len + vb_len;
  }

  *buffer = buf;
  return err;
}

/* Global data */
static SNMP_ERR_CODE_E
global_data_decode(struct snmp_datagram *sdg, uint8_t **buffer)
{
  SNMP_ERR_CODE_E err;
  uint8_t *buf;

  err = SNMP_ERR_OK;
  buf = *buffer;

  /* Global ID */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_GLOBAL_ID;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->msg_id_len);
  ber_value_dec(buf, sdg->msg_id_len, ASN1_TAG_INT, &sdg->msg_id);
  buf += sdg->msg_id_len;

  /* Global max size */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_GLOBAL_SIZE;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->msg_size_len);
  ber_value_dec(buf, sdg->msg_size_len, ASN1_TAG_INT, &sdg->msg_max_size);
  buf += sdg->msg_size_len;

  /* Global flags */
  if (*buf++ != ASN1_TAG_OCTSTR) {
    err = SNMP_ERR_GLOBAL_FLAGS;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->msg_flags_len);
  ber_value_dec(buf, sdg->msg_flags_len, ASN1_TAG_OCTSTR, &sdg->msg_flags);
  buf += sdg->msg_flags_len;

  /* Global security model */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_GLOBAL_MODEL;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->msg_model_len);
  ber_value_dec(buf, sdg->msg_model_len, ASN1_TAG_INT, &sdg->msg_security_model);
  buf += sdg->msg_model_len;

  *buffer = buf;
  return err;
}

/* Security parameter  */
static SNMP_ERR_CODE_E
security_parameter_decode(struct snmp_datagram *sdg, uint8_t **buffer)
{
  SNMP_ERR_CODE_E err;
  uint8_t *buf;

  err = SNMP_ERR_OK;
  buf = *buffer;

  /* Security string length */
  if (*buf++ != ASN1_TAG_OCTSTR) {
    err = SNMP_ERR_SECURITY_STR;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->secur_str_len);

  /* Security sequence length */
  if (*buf++ != ASN1_TAG_SEQ) {
    err = SNMP_ERR_SECURITY_SEQ;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->secur_para_len);

  /* Engine ID */
  if (*buf++ != ASN1_TAG_OCTSTR) {
    err = SNMP_ERR_SECURITY_ENGINE_ID;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->engine_id_len);
  if (sdg->engine_id_len + 1 > sizeof(sdg->engine_id)) {
    err = SNMP_ERR_SECURITY_ENGINE_ID_LEN;
    return err;
  }
  ber_value_dec(buf, sdg->engine_id_len, ASN1_TAG_OCTSTR, &sdg->engine_id);
  buf += sdg->engine_id_len;

  /* Engine boots */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_SECURITY_ENGINE_BOOTS;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->engine_boots_len);
  ber_value_dec(buf, sdg->engine_boots_len, ASN1_TAG_INT, &sdg->engine_boots);
  buf += sdg->engine_boots_len;

  /* Engine time */
  if (*buf++ != ASN1_TAG_INT) {
    err = SNMP_ERR_SECURITY_ENGINE_TIME;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->engine_time_len);
  ber_value_dec(buf, sdg->engine_time_len, ASN1_TAG_INT, &sdg->engine_time);
  buf += sdg->engine_time_len;

  /* User name */
  if (*buf++ != ASN1_TAG_OCTSTR) {
    err = SNMP_ERR_SECURITY_USER_NAME;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->user_name_len);
  if (sdg->user_name_len + 1 > sizeof(sdg->user_name)) {
    err = SNMP_ERR_SECURITY_USER_NAME_LEN;
    return err;
  }
  ber_value_dec(buf, sdg->user_name_len, ASN1_TAG_OCTSTR, &sdg->user_name);
  buf += sdg->user_name_len;

  /* Authorative parameter */
  if (*buf++ != ASN1_TAG_OCTSTR) {
    err = SNMP_ERR_SECURITY_AUTH_PARA;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->auth_para_len);
  if (sdg->auth_para_len + 1 > sizeof(sdg->auth_para)) {
    err = SNMP_ERR_SECURITY_AUTH_PARA_LEN;
    return err;
  }
  ber_value_dec(buf, sdg->auth_para_len, ASN1_TAG_OCTSTR, &sdg->auth_para);
  buf += sdg->auth_para_len;

  /* Privative parameter */
  if (*buf++ != ASN1_TAG_OCTSTR) {
    err = SNMP_ERR_SECURITY_ENGINE_TIME;
    return err;
  }
  buf += ber_length_dec(buf, &sdg->priv_para_len);
  if (sdg->priv_para_len + 1 > sizeof(sdg->priv_para)) {
    err = SNMP_ERR_SECURITY_PRIV_PARA_LEN;
    return err;
  }
  ber_value_dec(buf, sdg->priv_para_len, ASN1_TAG_OCTSTR, &sdg->priv_para);
  buf += sdg->priv_para_len;

  *buffer = buf;
  return err;
}

/* Decode snmp datagram */
static void
snmp_decode(struct snmp_datagram *sdg)
{
  SNMP_ERR_CODE_E err;
  uint8_t *buf, dec_fail = 0;
  const uint32_t tag_len = 1;

  /* Skip tag and length */
  buf = sdg->recv_buf + tag_len;
  buf += ber_length_dec(buf, &snmp_datagram.data_len);

  /* Version */
  if (*buf++ != ASN1_TAG_INT) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_VERSION, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_VERSION));
    dec_fail = 1;
    goto DECODE_FINISH;
  }
  buf += ber_length_dec(buf, &sdg->ver_len);
  ber_value_dec(buf, sdg->ver_len, ASN1_TAG_INT, &sdg->version);
  buf += sdg->ver_len;

  /* SNMPv3 */
  if (sdg->version >= 3) {
    /* Global data length */
    if (*buf++ != ASN1_TAG_SEQ) {
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_GLOBAL_DATA_LEN, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_GLOBAL_DATA_LEN));
      dec_fail = 1;
      goto DECODE_FINISH;
    }
    buf += ber_length_dec(buf, &sdg->msg_len);
 
    /* Global data */
    err = global_data_decode(sdg, &buf);
    if (err) { 
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(snmp_err_msg, elem_num(snmp_err_msg), err));
      dec_fail = 1;
      goto DECODE_FINISH;
    }

    /* Security parameter */
    err = security_parameter_decode(sdg, &buf);
    if (err) { 
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(snmp_err_msg, elem_num(snmp_err_msg), err));
      dec_fail = 1;
      goto DECODE_FINISH;
    }

    /* Scope PDU length */
    if (*buf++ != ASN1_TAG_SEQ) {
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_SCOPE_PDU_SEQ, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_SCOPE_PDU_SEQ));
      dec_fail = 1;
      goto DECODE_FINISH;
    }
    buf += ber_length_dec(buf, &sdg->scope_len);

    /* Context ID */
    if (*buf++ != ASN1_TAG_OCTSTR) {
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_CONTEXT_ID, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_CONTEXT_ID));
      dec_fail = 1;
      goto DECODE_FINISH;
    }
    buf += ber_length_dec(buf, &sdg->context_id_len);
    if (sdg->context_id_len + 1 > sizeof(sdg->context_id)) {
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_CONTEXT_ID_LEN, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_CONTEXT_ID_LEN));
      dec_fail = 1;
      goto DECODE_FINISH;
    }
    ber_value_dec(buf, sdg->context_id_len, ASN1_TAG_OCTSTR, &sdg->context_id);
    buf += sdg->context_id_len;
  }

  /* Context name */
  if (*buf++ != ASN1_TAG_OCTSTR) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_CONTEXT_NAME, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_CONTEXT_NAME));
    dec_fail = 1;
    goto DECODE_FINISH;
  }
  buf += ber_length_dec(buf, &sdg->context_name_len);
  if (sdg->context_name_len + 1 > sizeof(sdg->context_name)) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_CONTEXT_NAME_LEN, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_CONTEXT_NAME_LEN));
    dec_fail = 1;
    goto DECODE_FINISH;
  }
  ber_value_dec(buf, sdg->context_name_len, ASN1_TAG_OCTSTR, sdg->context_name);
  buf += sdg->context_name_len;

  /* PDU header */
  err = pdu_hdr_parse(sdg, &buf);
  if (err) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(snmp_err_msg, elem_num(snmp_err_msg), err));
    dec_fail = 1;
    goto DECODE_FINISH;
  }

  /* var bind */
  err = var_bind_parse(sdg, &buf);
  if (err) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(snmp_err_msg, elem_num(snmp_err_msg), err));
    dec_fail = 1;
    goto DECODE_FINISH;
  }

DECODE_FINISH:
  /* We should free received buffer here */
  free(sdg->recv_buf);

  /* If fail, do some clear things */
  if (dec_fail) {
    snmp_datagram_clear(sdg);
  }
}

/* SNMP request dispatch */
static void
snmp_request_dispatch(struct snmp_datagram *sdg)
{
  switch (sdg->pdu_hdr.pdu_type) {
    case MIB_REQ_GET:
      if (sdg->vb_in_cnt == 0) {
        sdg->pdu_hdr.pdu_type = MIB_REPO;
      } else {
        sdg->pdu_hdr.pdu_type = MIB_RESP;
      }
      snmp_get(sdg);
      break;
    case MIB_REQ_GETNEXT:
      if (sdg->vb_in_cnt == 0) {
        sdg->pdu_hdr.pdu_type = MIB_REPO;
      } else {
        sdg->pdu_hdr.pdu_type = MIB_RESP;
      }
      snmp_getnext(sdg);
      break;
    case MIB_RESP:
      break;
    case MIB_REQ_SET:
      if (sdg->vb_in_cnt == 0) {
        sdg->pdu_hdr.pdu_type = MIB_REPO;
      } else {
        sdg->pdu_hdr.pdu_type = MIB_RESP;
      }
      snmp_set(sdg);
      break;
    case MIB_REQ_BULKGET:
      if (sdg->vb_in_cnt == 0) {
        sdg->pdu_hdr.pdu_type = MIB_REPO;
      } else {
        sdg->pdu_hdr.pdu_type = MIB_RESP;
      }
      snmp_bulkget(sdg);
      break;
    case MIB_REQ_INF:
      break;
    case MIB_TRAP:
      break;
    case MIB_REPO:
      break;
    default:
      break;
  }
}

/* Receive snmp datagram from transport module */
void
snmpd_recv(uint8_t *buffer, int len)
{
  uint32_t len_len;
  const uint32_t tag_len = 1;

  assert(buffer != NULL && len > 0);

  /* Check PDU tag */
  if (buffer[0] != ASN1_TAG_SEQ) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_PDU_TYPE, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_PDU_TYPE));
    free(buffer);
    return;
  }

  /* Check PDU length */
  len_len = ber_length_dec(buffer + tag_len, &snmp_datagram.data_len);
  if (tag_len + len_len + snmp_datagram.data_len != len) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", SNMP_ERR_PDU_LEN, error_message(snmp_err_msg, elem_num(snmp_err_msg), SNMP_ERR_PDU_LEN));
    free(buffer);
    return;
  }

  /* Reset datagram */
  snmp_datagram_clear(&snmp_datagram);
  snmp_datagram.recv_buf = buffer;

  /* Decode snmp datagram */
  snmp_decode(&snmp_datagram);

  /* Dispatch request */
  snmp_request_dispatch(&snmp_datagram);
}
