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

static uint8_t *
asn1_encode(struct snmp_datagram *sdg)
{
  struct pdu_hdr *ph;
  uint8_t *buf, len_len;
  const uint8_t tag_len = 1;

  ph = &sdg->pdu_hdr;

  len_len = ber_length_enc_test(ph->req_id_len);
  ph->pdu_len = tag_len + len_len + ph->req_id_len;

  len_len = ber_length_enc_test(ph->err_stat_len);
  ph->pdu_len += tag_len + len_len + ph->err_stat_len;

  len_len = ber_length_enc_test(ph->err_idx_len);
  ph->pdu_len += tag_len + len_len + ph->err_idx_len;

  len_len = ber_length_enc_test(sdg->vb_list_len);
  ph->pdu_len += tag_len + len_len + sdg->vb_list_len;

  len_len = ber_length_enc_test(ph->pdu_len);
  sdg->data_len = tag_len + len_len + ph->pdu_len;

  len_len = ber_length_enc_test(sdg->ver_len);
  sdg->data_len += tag_len + len_len + sdg->ver_len;

  len_len = ber_length_enc_test(sdg->comm_len);
  sdg->data_len += tag_len + len_len + sdg->comm_len;

  len_len = ber_length_enc_test(sdg->data_len);
  sdg->send_buf = xmalloc(tag_len + len_len + sdg->data_len);

  buf = sdg->send_buf;

  /* Datagram type and length */
  *buf++ = ASN1_TAG_SEQ;
  buf += ber_length_enc(sdg->data_len, buf);

  /* Version */
  *buf++ = ASN1_TAG_INT;
  buf += ber_length_enc(sdg->ver_len, buf);
  buf += ber_value_enc(&sdg->version, 1, ASN1_TAG_INT, buf);

  /* Community */
  *buf++ = ASN1_TAG_OCTSTR;
  buf += ber_length_enc(sdg->comm_len, buf);
  buf += ber_value_enc(sdg->community, sdg->comm_len, ASN1_TAG_OCTSTR, buf);

  /* PDU header */
  *buf++ = MIB_RESP;
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

  /* var bind list */
  *buf++ = ASN1_TAG_SEQ;
  buf += ber_length_enc(sdg->vb_list_len, buf);

  return buf;
}

void
snmpd_send_response(struct snmp_datagram *sdg)
{
  struct var_bind *vb_out;
  struct list_head *curr, *next;
  uint8_t *buf, oid_len;
  uint32_t len_len;
  const uint8_t tag_len = 1;

  buf = asn1_encode(sdg);
  if (buf == NULL)
    return;

  list_for_each_safe(curr, next, &sdg->vb_out_list) {

    vb_out = list_entry(curr, struct var_bind, link);

    *buf++ = ASN1_TAG_SEQ;
    buf += ber_length_enc(vb_out->vb_len, buf);

    /* oid */
    *buf++ = ASN1_TAG_OBJID;
    oid_len = ber_value_enc_test(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID);
    buf += ber_length_enc(oid_len, buf);
    buf += ber_value_enc(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID, buf);

    /* value */
    *buf++ = vb_out->value_type;
    buf += ber_length_enc(vb_out->value_len, buf);
    memcpy(buf, vb_out->value, vb_out->value_len);
    buf += vb_out->value_len;
  }

  len_len = ber_length_enc_test(sdg->data_len);
  /* This callback will free send_buf */
  snmp_prot_ops.send(sdg->send_buf, tag_len + len_len + sdg->data_len);
}
