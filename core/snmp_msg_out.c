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

#include "mib.h"
#include "snmp.h"
#include "protocol.h"
#include "util.h"

static octstr_t snmpv3_engine_id[] = {
  0x80, 0x00, 0x00, 0x00,
  /* Text */
  0x04,
  /* 'S', 'm', 'a', 'r', 't', 'S', 'N', 'M', 'P' */
  0x53, 0x6d, 0x61, 0x72, 0x74, 0x53, 0x4e, 0x4d, 0x50, 
};

static uint32_t
global_data_encode_try(struct snmp_datagram *sdg)
{
  const uint32_t tag_len = 1;
  uint32_t len_len, msg_len;
 
  len_len = ber_length_enc_try(sdg->msg_id_len);
  msg_len = tag_len + len_len + sdg->msg_id_len;

  len_len = ber_length_enc_try(sdg->msg_size_len);
  msg_len += tag_len + len_len + sdg->msg_size_len;
 
  len_len = ber_length_enc_try(sdg->msg_flags_len);
  msg_len += tag_len + len_len + sdg->msg_flags_len;
 
  len_len = ber_length_enc_try(sdg->msg_model_len);
  msg_len += tag_len + len_len + sdg->msg_model_len;

  return msg_len;
}

static uint8_t *
global_data_encode(struct snmp_datagram *sdg, uint8_t *buf)
{
  /* Global data sequence */
  *buf++ = ASN1_TAG_SEQ;
  buf += ber_length_enc(sdg->msg_len, buf);

  /* Messege ID */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(sdg->msg_id_len, buf);
  buf += ber_value_enc(&sdg->msg_id, 1, ASN1_TAG_INT, buf);

  /* Messege max size */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(sdg->msg_size_len, buf);
  buf += ber_value_enc(&sdg->msg_max_size, 1, ASN1_TAG_INT, buf);

  /* Messege flags */
  *buf++ = ASN1_TAG_OCTSTR;
  buf += ber_length_enc(sdg->msg_flags_len, buf);
  sdg->msg_flags = 0;
  buf += ber_value_enc(&sdg->msg_flags, sdg->msg_flags_len, ASN1_TAG_OCTSTR, buf);

  /* Messege security model */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(sdg->msg_model_len, buf);
  buf += ber_value_enc(&sdg->msg_security_model, 1, ASN1_TAG_INT, buf);

  return buf;
}

static uint32_t
security_parameter_encode_try(struct snmp_datagram *sdg)
{
  const uint32_t tag_len = 1;
  uint32_t len_len, secur_para_len;

  sdg->engine_id_len = sizeof(snmpv3_engine_id);
  len_len = ber_length_enc_try(sdg->engine_id_len);
  secur_para_len = tag_len + len_len + sdg->engine_id_len;

  len_len = ber_length_enc_try(sdg->engine_boots_len);
  secur_para_len += tag_len + len_len + sdg->engine_boots_len;

  len_len = ber_length_enc_try(sdg->engine_time_len);
  secur_para_len += tag_len + len_len + sdg->engine_time_len;

  len_len = ber_length_enc_try(sdg->user_name_len);
  secur_para_len += tag_len + len_len + sdg->user_name_len;

  len_len = ber_length_enc_try(sdg->auth_para_len);
  secur_para_len += tag_len + len_len + sdg->auth_para_len;

  len_len = ber_length_enc_try(sdg->priv_para_len);
  secur_para_len += tag_len + len_len + sdg->priv_para_len;

  return secur_para_len;
}

static uint8_t *
security_parameter_encode(struct snmp_datagram *sdg, uint8_t *buf)
{
  /* Security parameter sequence */
  *buf++ = ASN1_TAG_SEQ;
  buf += ber_length_enc(sdg->secur_para_len, buf);

  /* Engine ID */
  *buf++ = ASN1_TAG_OCTSTR;
  buf += ber_length_enc(sdg->engine_id_len, buf);
  buf += ber_value_enc(snmpv3_engine_id, sdg->engine_id_len, ASN1_TAG_OCTSTR, buf);

  /* Engine boots */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(sdg->engine_boots_len, buf);
  buf += ber_value_enc(&sdg->engine_boots, 1, ASN1_TAG_INT, buf);

  /* Engine time */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(sdg->engine_time_len, buf);
  buf += ber_value_enc(&sdg->engine_time, 1, ASN1_TAG_INT, buf);

  /* User name */
  *buf++ = ASN1_TAG_OCTSTR;
  buf += ber_length_enc(sdg->user_name_len, buf);
  buf += ber_value_enc(&sdg->user_name, sdg->user_name_len, ASN1_TAG_OCTSTR, buf);

  /* Authotative parameter */
  *buf++ = ASN1_TAG_OCTSTR;
  buf += ber_length_enc(sdg->auth_para_len, buf);
  buf += ber_value_enc(&sdg->auth_para, sdg->auth_para_len, ASN1_TAG_OCTSTR, buf);

  /* Privative parameter */
  *buf++ = ASN1_TAG_OCTSTR;
  buf += ber_length_enc(sdg->priv_para_len, buf);
  buf += ber_value_enc(&sdg->priv_para, sdg->priv_para_len, ASN1_TAG_OCTSTR, buf);

  return buf; 
}

static uint8_t *
asn1_encode(struct snmp_datagram *sdg)
{
  struct pdu_hdr *ph;
  uint8_t *buf;
  const uint32_t tag_len = 1;
  uint32_t len_len;

  ph = &sdg->pdu_hdr;
  sdg->data_len = 0;

  len_len = ber_length_enc_try(sdg->vb_list_len);
  ph->pdu_len = tag_len + len_len + sdg->vb_list_len;

  len_len = ber_length_enc_try(ph->req_id_len);
  ph->pdu_len += tag_len + len_len + ph->req_id_len;

  len_len = ber_length_enc_try(ph->err_stat_len);
  ph->pdu_len += tag_len + len_len + ph->err_stat_len;

  len_len = ber_length_enc_try(ph->err_idx_len);
  ph->pdu_len += tag_len + len_len + ph->err_idx_len;

  len_len = ber_length_enc_try(ph->pdu_len);
  sdg->scope_len = tag_len + len_len + ph->pdu_len;

  len_len = ber_length_enc_try(sdg->context_name_len);
  sdg->scope_len += tag_len + len_len + sdg->context_name_len;

  if (sdg->version >= 3) {
    /* SNMPv3 */
    sdg->context_id_len = sizeof(snmpv3_engine_id);
    len_len = ber_length_enc_try(sdg->context_id_len);
    sdg->scope_len += tag_len + len_len + sdg->context_id_len;

    sdg->secur_para_len = security_parameter_encode_try(sdg);

    len_len = ber_length_enc_try(sdg->secur_para_len);
    sdg->secur_str_len = tag_len + len_len + sdg->secur_para_len;

    sdg->msg_len = global_data_encode_try(sdg);

    len_len = ber_length_enc_try(sdg->msg_len);
    sdg->data_len += tag_len + len_len + sdg->msg_len;

    len_len = ber_length_enc_try(sdg->secur_str_len);
    sdg->data_len += tag_len + len_len + sdg->secur_str_len;

    len_len = ber_length_enc_try(sdg->scope_len);
    sdg->data_len += tag_len + len_len;
  }

  sdg->data_len += sdg->scope_len;

  len_len = ber_length_enc_try(sdg->ver_len);
  sdg->data_len += tag_len + len_len + sdg->ver_len;

  len_len = ber_length_enc_try(sdg->data_len);
  sdg->send_buf = xmalloc(tag_len + len_len + sdg->data_len);

  buf = sdg->send_buf;

  /* Datagram sequence */
  *buf++ = ASN1_TAG_SEQ;
  buf += ber_length_enc(sdg->data_len, buf);

  /* Version */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(sdg->ver_len, buf);
  buf += ber_value_enc(&sdg->version, 1, ASN1_TAG_INT, buf);

  if (sdg->version >= 3) {
    /* Global data */
    buf = global_data_encode(sdg, buf);

    /* Security string */
    *buf++ = ASN1_TAG_OCTSTR;
    buf += ber_length_enc(sdg->secur_str_len, buf);

    /* Security parameter */
    buf = security_parameter_encode(sdg, buf);

    /* Context sequence */
    *buf++ = ASN1_TAG_SEQ;
    buf += ber_length_enc(sdg->scope_len, buf);

    /* Context ID */
    *buf++ = ASN1_TAG_OCTSTR;
    buf += ber_length_enc(sizeof(snmpv3_engine_id), buf);
    buf += ber_value_enc(snmpv3_engine_id, sizeof(snmpv3_engine_id), ASN1_TAG_OCTSTR, buf);
  }

  /* Context_name */
  *buf++ = ASN1_TAG_OCTSTR;
  buf += ber_length_enc(sdg->context_name_len, buf);
  buf += ber_value_enc(sdg->context_name, sdg->context_name_len, ASN1_TAG_OCTSTR, buf);

  /* PDU header */
  *buf++ = ph->pdu_type;
  buf += ber_length_enc(ph->pdu_len, buf);

  /* Request ID */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(ph->req_id_len, buf);
  buf += ber_value_enc(&ph->req_id, 1, ASN1_TAG_INT, buf);

  /* Error status */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(ph->err_stat_len, buf);
  buf += ber_value_enc(&ph->err_stat, 1, ASN1_TAG_INT, buf);

  /* Error index */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(ph->err_idx_len, buf);
  buf += ber_value_enc(&ph->err_idx, 1, ASN1_TAG_INT, buf);

  /* Varbind list */
  *buf++ = ASN1_TAG_SEQ;
  buf += ber_length_enc(sdg->vb_list_len, buf);

  return buf;
}

void
snmp_response(struct snmp_datagram *sdg)
{
  struct var_bind *vb_out;
  struct list_head *curr, *next;
  uint8_t *buf;
  uint32_t oid_len, len_len;
  const uint32_t tag_len = 1;

  buf = asn1_encode(sdg);

  list_for_each_safe(curr, next, &sdg->vb_out_list) {
    vb_out = list_entry(curr, struct var_bind, link);

    *buf++ = ASN1_TAG_SEQ;
    buf += ber_length_enc(vb_out->vb_len, buf);

    /* oid */
    *buf++ = ASN1_TAG_OBJID;
    oid_len = ber_value_enc_try(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID);
    buf += ber_length_enc(oid_len, buf);
    buf += ber_value_enc(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID, buf);

    /* value */
    *buf++ = vb_out->value_type;
    buf += ber_length_enc(vb_out->value_len, buf);
    memcpy(buf, vb_out->value, vb_out->value_len);
    buf += vb_out->value_len;
  }

  len_len = ber_length_enc_try(sdg->data_len);
  /* This callback will free send_buf */
  snmp_prot_ops.send(sdg->send_buf, tag_len + len_len + sdg->data_len);
}
