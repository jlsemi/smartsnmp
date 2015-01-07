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

#include <sys/types.h>
#include <sys/socket.h>

#include "agentx.h"
#include "util.h"

struct x_pdu_buf
agentx_open_pdu(struct agentx_datagram *xdg, const oid_t *oid, uint32_t oid_len, const char *descr, uint32_t descr_len)
{
  uint8_t i, *pdu, *buf;
  uint32_t *timeout, len;
  struct x_pdu_buf x_pdu;
  struct x_pdu_hdr *ph;
  struct x_objid_t *objid;
  struct x_octstr_t *octstr;

  assert(oid_len == 0 || (oid_len > 4 && oid_len + 5 <= MIB_OID_MAX_LEN && descr_len <= MIB_VALUE_MAX_LEN));
  descr_len = uint_sizeof(descr_len);

  /* PDU length */
  len = sizeof(*ph) + sizeof(*timeout) + 4;
  len += oid_len == 0 ? 0: (oid_len - 5) * sizeof(uint32_t);
  len += 4 + descr_len;
  pdu = buf = xmalloc(len);
  memset(buf, 0, len);

  /* PDU header */
  ph = (struct x_pdu_hdr *)buf;
  ph->version = 1;
  ph->type = AGENTX_PDU_OPEN;
#ifdef LITTLE_ENDIAN
  ph->flags = 0;
#else
  ph->flags = NETWORD_BYTE_ORDER;
#endif
  ph->session_id = xdg->pdu_hdr.session_id;
  ph->transaction_id = xdg->pdu_hdr.transaction_id;
  ph->packet_id = 1;
  ph->payload_length = len - sizeof(*ph);

  /* time out == 0 */
  buf += sizeof(*ph) + sizeof(*timeout);

  /* sub-oid */
  objid = (struct x_objid_t *)buf;
  objid->n_subid = oid_len == 0 ? 0 : oid_len - 5;
  objid->prefix = oid_len == 0 ? 0 : oid[4];
  objid->include = 0;
  for (i = 5; i < oid_len; i++) {
    objid->sub_id[i - 5] = oid[i];
  }
  buf += 4;
  buf += oid_len == 0 ? 0 : (oid_len - 5) * sizeof(uint32_t);

  /* octet string */
  octstr = (struct x_octstr_t *)buf;
  octstr->len = descr_len;
  memcpy(octstr->str, descr, strlen(descr));

  x_pdu.buf = pdu;
  x_pdu.len = len;
  return x_pdu;
}

struct x_pdu_buf
agentx_close_pdu(struct agentx_datagram *xdg, uint32_t reason)
{
  uint8_t *pdu, *buf;
  uint32_t len;
  struct x_pdu_buf x_pdu;
  struct x_pdu_hdr *ph;

  /* PDU length */
  len = sizeof(*ph) + sizeof(uint32_t);
  pdu = buf = xmalloc(len);
  memset(buf, 0, len);

  /* PDU header */
  ph = (struct x_pdu_hdr *)buf;
  ph->version = xdg->pdu_hdr.version;
  ph->type = AGENTX_PDU_CLOSE;
  ph->flags = xdg->pdu_hdr.flags;
  xdg->pdu_hdr.packet_id += 1;
  ph->session_id = xdg->pdu_hdr.session_id;
  ph->transaction_id = xdg->pdu_hdr.transaction_id;
  ph->packet_id = xdg->pdu_hdr.packet_id;
  ph->payload_length = len - sizeof(*ph);
  buf += sizeof(*ph);

  /* close reason */
  *(uint32_t *)buf = reason;

  x_pdu.buf = pdu;
  x_pdu.len = len;
  return x_pdu;
}

struct x_pdu_buf
agentx_register_pdu(struct agentx_datagram *xdg, const oid_t *oid, uint32_t oid_len, const char *context, uint32_t ctx_len,
                    uint8_t timeout, uint8_t priority, uint8_t range_subid, uint32_t upper_bound)
{
  uint8_t i, *pdu, *buf;
  uint32_t len;
  struct x_pdu_buf x_pdu;
  struct x_pdu_hdr *ph;
  struct x_objid_t *objid;
  struct x_octstr_t *octstr;

  assert(oid_len > 4 && oid_len + 5 <= MIB_OID_MAX_LEN && ctx_len <= 40);
  ctx_len = uint_sizeof(ctx_len);

  /* PDU length */
  len = sizeof(*ph);
  if (ctx_len) {
    len += 4 + ctx_len;
  }
  len += 4 + 4 + (oid_len - 5) * sizeof(uint32_t);
  if (range_subid) {
    len += 4;
  }
  pdu = buf = xmalloc(len);
  memset(buf, 0, len);

  /* PDU header */
  ph = (struct x_pdu_hdr *)buf;
  ph->version = xdg->pdu_hdr.version;
  ph->type = AGENTX_PDU_REG;
  ph->flags = xdg->pdu_hdr.flags | INSTANCE_REGISTRATION;
  xdg->pdu_hdr.packet_id += 1;
  ph->session_id = xdg->pdu_hdr.session_id;
  ph->transaction_id = xdg->pdu_hdr.transaction_id;
  ph->packet_id = xdg->pdu_hdr.packet_id;
  ph->payload_length = len - sizeof(*ph);
  buf = (uint8_t *)pdu + sizeof(*ph);

  /* context string */
  if (ctx_len > 0) {
    octstr = (struct x_octstr_t *)buf;
    octstr->len = ctx_len;
    memcpy(octstr->str, context, strlen(context));
    buf += 4 + ctx_len;
  }

  /* special fields */
  *buf++ = timeout;
  *buf++ = priority;
  *buf++ = range_subid;
  buf++;

  /* region */
  objid = (struct x_objid_t *)buf;
  objid->n_subid = oid_len - 5;
  objid->prefix = oid[4];
  objid->include = 0;
  for (i = 5; i < oid_len; i++) {
    objid->sub_id[i - 5] = oid[i];
  }
  buf += 4 + (oid_len - 5) * sizeof(uint32_t);

  /* upper bound */
  if (range_subid) {
    *(uint32_t *)buf = upper_bound;
  }

  x_pdu.buf = pdu;
  x_pdu.len = len;
  return x_pdu;
}

struct x_pdu_buf
agentx_unregister_pdu(struct agentx_datagram *xdg, const oid_t *oid, uint32_t oid_len, const char *context, uint32_t ctx_len,
                      uint8_t timeout, uint8_t priority, uint8_t range_subid, uint32_t upper_bound)
{
  uint8_t i, *pdu, *buf;
  uint32_t len;
  struct x_pdu_buf x_pdu;
  struct x_pdu_hdr *ph;
  struct x_objid_t *objid;
  struct x_octstr_t *octstr;

  assert(oid_len > 4 && oid_len + 5 <= MIB_OID_MAX_LEN && ctx_len <= 40);
  ctx_len = uint_sizeof(ctx_len);

  /* PDU length */
  len = sizeof(*ph);
  if (ctx_len) {
    len += 4 + ctx_len;
  }
  len += 4 + 4 + (oid_len - 5) * sizeof(uint32_t);
  if (range_subid) {
    len += 4;
  }
  pdu = buf = xmalloc(len);
  memset(buf, 0, len);

  /* PDU header */
  ph = (struct x_pdu_hdr *)buf;
  ph->version = xdg->pdu_hdr.version;
  ph->type = AGENTX_PDU_UNREG;
  ph->flags = xdg->pdu_hdr.flags | INSTANCE_REGISTRATION;
  xdg->pdu_hdr.packet_id += 1;
  ph->session_id = xdg->pdu_hdr.session_id;
  ph->transaction_id = xdg->pdu_hdr.transaction_id;
  ph->packet_id = xdg->pdu_hdr.packet_id;
  ph->payload_length = len - sizeof(*ph);
  buf += sizeof(*ph);

  /* context string */
  if (ctx_len) {
    octstr = (struct x_octstr_t *)buf;
    octstr->len = ctx_len;
    memcpy(octstr->str, context, strlen(context));
    buf += 4 + ctx_len;
  }

  /* special fields */
  buf++;
  *buf++ = priority; /* 127 */
  *buf++ = range_subid;
  buf++;

  /* region */
  objid = (struct x_objid_t *)buf;
  objid->n_subid = oid_len - 5;
  objid->prefix = oid[4];
  objid->include = 0;
  for (i = 5; i < oid_len; i++) {
    objid->sub_id[i - 5] = oid[i];
  }
  buf += 4 + (oid_len - 5) * sizeof(uint32_t);

  /* upper bound */
  if (range_subid) {
    *(uint32_t *)buf = upper_bound;
  }

  x_pdu.buf = pdu;
  x_pdu.len = len;
  return x_pdu;
}

#if 0
struct x_pdu_buf
agentx_notify_pdu(struct agentx_datagram *xdg, const char *context, uint32_t context_len,
                  uint8_t type, const oid_t *oid, uint32_t oid_len,
                  uint8_t *data, uint32_t data_len)
{
  uint8_t i, *pdu, *buf;
  uint32_t *reason, len;
  struct x_pdu_buf x_pdu;
  struct x_pdu_hdr *ph;
  struct x_octstr_t *octstr;
  struct x_varbind_t *vb;

  assert(context_len < 40);
  context_len = uint_sizeof(context_len);

  /* PDU length */
  len = sizeof(*ph);
  if (context_len) {
    len += 4 + context_len;
  }
  pdu = buf = xmalloc(len);
  memset(buf, 0, len);

  /* PDU header */
  h = (struct x_pdu_hdr *)pdu;
  ph->version = 1;
  ph->type = AGENTX_PDU_NOTIFY;
  ph->flags = 0;
  xdg->pdu_hdr.packet_id += 1;
  ph->session_id = xdg->pdu_hdr.session_id;
  ph->transaction_id = xdg->pdu_hdr.transaction_id;
  ph->packet_id = xdg->pdu_hdr.packet_id;
  ph->payload_length = len - sizeof(*ph);
  buf = (uint8_t *)pdu + sizeof(*ph);

  /* context */
  if (context_len) {
    octstr = (struct x_octstr_t *)buf;
    octstr->len = context_len;
    memcpy(octstr->str, context, strlen(context));
    buf += 4 + context_len;
  }

  for (i = 0; i < len; i++) {
    vb = (struct x_varbind_t *)buf;
    vb->type = type;
  }

  x_pdu.buf = pdu;
  x_pdu.len = len;
  return x_pdu;
}
#endif

struct x_pdu_buf
agentx_ping_pdu(struct agentx_datagram *xdg, const char *context, uint32_t context_len)
{
  uint8_t *pdu, *buf;
  uint32_t len;
  struct x_pdu_buf x_pdu;
  struct x_pdu_hdr *ph;
  struct x_octstr_t *octstr;

  assert(context_len < 40);
  context_len = uint_sizeof(context_len);

  /* PDU length */
  len = sizeof(*ph);
  if (context_len) {
    len += 4 + context_len;
  }
  pdu = buf = xmalloc(len);
  memset(buf, 0, len);

  /* PDU header */
  ph = (struct x_pdu_hdr *)buf;
  ph->version = xdg->pdu_hdr.version;
  ph->type = AGENTX_PDU_PING;
  ph->flags = xdg->pdu_hdr.flags;
  xdg->pdu_hdr.packet_id += 1;
  ph->session_id = xdg->pdu_hdr.session_id;
  ph->transaction_id = xdg->pdu_hdr.transaction_id;
  ph->packet_id = xdg->pdu_hdr.packet_id;
  ph->payload_length = len - sizeof(*ph);
  buf += sizeof(*ph);

  /* context */
  if (context_len) {
    octstr = (struct x_octstr_t *)buf;
    octstr->len = context_len;
    memcpy(octstr->str, context, strlen(context));
  }

  x_pdu.buf = pdu;
  x_pdu.len = len;
  return x_pdu;
}

struct x_pdu_buf
agentx_response_pdu(struct agentx_datagram *xdg)
{
  uint8_t *pdu, *buf;
  uint32_t i, len;
  struct x_var_bind *vb_out;
  struct x_pdu_buf x_pdu;
  struct x_pdu_hdr *ph;
  struct x_objid_t *objid;
  struct x_octstr_t *octstr;
  struct list_head *curr, *next;
  uint32_t *p_tmp32;
  uint64_t *p_tmp64;
  oid_t *oid;

  /* PDU length */
  len = sizeof(*ph) + sizeof(uint32_t) + 2 * sizeof(uint16_t);
  list_for_each_safe(curr, next, &xdg->vb_out_list) {
    vb_out = list_entry(curr, struct x_var_bind, link);
    if (vb_out->oid_len > 5) {
      len += 4 + 4 + (vb_out->oid_len - 5) * sizeof(uint32_t);
    } else {
      len += 4 + 4;
    }
    switch (vb_out->val_type) {
      case ASN1_TAG_INT:
      case ASN1_TAG_CNT:
      case ASN1_TAG_GAU:
      case ASN1_TAG_TIMETICKS:
        len += sizeof(uint32_t);
        break;
      case ASN1_TAG_CNT64:
        len += sizeof(uint64_t);
        break;
      case ASN1_TAG_OCTSTR:
      case ASN1_TAG_IPADDR:
        len += sizeof(uint32_t) + uint_sizeof(vb_out->val_len);
        break;
      case ASN1_TAG_OBJID:
        if (vb_out->val_len > 5 * sizeof(uint32_t)) {
          len += 4 + vb_out->val_len - 5 * sizeof(uint32_t);
        } else {
          len += 4;
        }
        break;
      default:
        break;
    }
  }
  pdu = buf = xmalloc(len);
  memset(buf, 0, len);

  /* PDU header */
  ph = (struct x_pdu_hdr *)buf;
  ph->version = xdg->pdu_hdr.version;
  ph->type = AGENTX_PDU_RESPONSE;
  ph->flags = xdg->pdu_hdr.flags;
  ph->session_id = xdg->pdu_hdr.session_id;
  ph->transaction_id = xdg->pdu_hdr.transaction_id;
  ph->packet_id = xdg->pdu_hdr.packet_id;
  ph->payload_length = len - sizeof(*ph);
  buf += sizeof(*ph);

  /* special fields */
  buf += sizeof(uint32_t); /* timeout will be ignored from subagent to master */
  *(uint16_t *)buf = xdg->u.response.error;
  buf += sizeof(uint16_t);
  *(uint16_t *)buf = xdg->u.response.index;
  buf += sizeof(uint16_t);

  /* var binds */
  list_for_each_safe(curr, next, &xdg->vb_out_list) {
    vb_out = list_entry(curr, struct x_var_bind, link);

    /* type */
    *(uint16_t *)buf = vb_out->val_type;
    buf += 2 * sizeof(uint16_t);

    /* oid */
    objid = (struct x_objid_t *)buf;
    objid->n_subid = vb_out->oid_len > 5 ? vb_out->oid_len - 5 : 0;
    objid->prefix = vb_out->oid_len > 4 ? vb_out->oid[4] : 0;
    objid->include = 0;
    for (i = 5; i < vb_out->oid_len; i++) {
      objid->sub_id[i - 5] = vb_out->oid[i];
    }
    if (vb_out->oid_len > 5) {
      buf += sizeof(*objid) + (vb_out->oid_len - 5) * sizeof(uint32_t);
    } else {
      buf += sizeof(*objid);
    }

    /* data */
    switch (vb_out->val_type) {
      case ASN1_TAG_INT:
      case ASN1_TAG_CNT:
      case ASN1_TAG_GAU:
      case ASN1_TAG_TIMETICKS:
        p_tmp32 = (uint32_t *)vb_out->value;
        *(uint32_t *)buf = *p_tmp32;
        buf += sizeof(uint32_t);
        break;
      case ASN1_TAG_CNT64:
        p_tmp64 = (uint64_t *)vb_out->value;
        *(uint64_t *)buf = *p_tmp64;
        buf += sizeof(uint64_t);
        break;
      case ASN1_TAG_OCTSTR:
      case ASN1_TAG_IPADDR:
        octstr = (struct x_octstr_t *)buf;
        octstr->len = vb_out->val_len;
        memcpy(octstr->str, vb_out->value, vb_out->val_len);
        buf += sizeof(uint32_t) + uint_sizeof(vb_out->val_len);
        break;
      case ASN1_TAG_OBJID:
        objid = (struct x_objid_t *)buf;
        oid = (oid_t *)vb_out->value;
        objid->n_subid = vb_out->val_len > 5 * sizeof(uint32_t) ? (vb_out->val_len - 5 * sizeof(uint32_t)) / sizeof(uint32_t) : 0;
        objid->prefix = vb_out->val_len > 4 * sizeof(uint32_t) ? oid[4] : 0;
        objid->include = 0;
        for (i = 5; i < vb_out->val_len / sizeof(uint32_t); i++) {
          objid->sub_id[i - 5] = oid[i];
        }
        if (vb_out->val_len > 5 * sizeof(uint32_t)) {
          buf += 4 + vb_out->val_len - 5 * sizeof(uint32_t);
        } else {
          buf += 4;
        }
        break;
      default:
        break;
    }
  }

  x_pdu.buf = pdu;
  x_pdu.len = len;
  return x_pdu;
}

/* Send AgentX response PDU */
void
agentx_response(struct agentx_datagram *xdg)
{
  /* Send response PDU */
  struct x_pdu_buf x_pdu = agentx_response_pdu(xdg);
  if (send(xdg->sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    SMARTSNMP_LOG(L_ERROR, "ERR: Send response PDU failure!\n");
  }
  free(x_pdu.buf);
}
