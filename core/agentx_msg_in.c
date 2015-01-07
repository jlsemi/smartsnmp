/*
 * This file is part of Smartagentx
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

#include "mib.h"
#include "agentx.h"
#include "util.h"

static struct err_msg_map agentx_err_msg[] = {
  { AGENTX_ERR_OK, "Every thing is OK!" },

  { AGENTX_ERR_PDU_CTX_LEN, "AgentX PDU context length exceeds!" },

  { AGENTX_ERR_VB_VAR, "AgentX varbind allocation fail!" },
  { AGENTX_ERR_VB_VALUE_LEN, "AgentX varbind value length exceeds!" },
  { AGENTX_ERR_VB_OID_LEN, "AgentX varbind oid length exceeds!" },

  { AGENTX_ERR_SR_VAR, "AgentX search range allocation fail!" },
  { AGENTX_ERR_SR_OID_LEN, "AgentX search range oid length exceeds!" },
};

static struct x_var_bind *
vb_new(uint32_t oid_len, uint32_t val_len)
{
  struct x_var_bind *vb = xmalloc(sizeof(*vb) + val_len);
  vb->oid = xmalloc(oid_len);
  return vb;
}

static void
vb_delete(struct x_var_bind *vb)
{
  free(vb->oid);
  free(vb);
}

static void
vb_list_free(struct list_head *vb_list)
{
  struct list_head *pos, *n;

  list_for_each_safe(pos, n, vb_list) {
    struct x_var_bind *vb = list_entry(pos, struct x_var_bind, link);
    list_del(&vb->link);
    vb_delete(vb);
  }
}

static struct x_search_range *
sr_new(uint32_t start_len, uint32_t end_len)
{
  struct x_search_range *sr = xmalloc(sizeof(*sr));
  sr->start = xmalloc(start_len * sizeof(uint32_t));
  sr->end = xmalloc(end_len * sizeof(uint32_t));
  return sr;
}

static void
sr_delete(struct x_search_range *sr)
{
  free(sr->start);
  free(sr->end);
  free(sr);
}

static void
sr_list_free(struct list_head *sr_list)
{
  struct list_head *pos, *n;

  list_for_each_safe(pos, n, sr_list) {
    struct x_search_range *sr = list_entry(pos, struct x_search_range, link);
    list_del(&sr->link);
    sr_delete(sr);
  }
}

static void
agentx_datagram_clear(struct agentx_datagram *xdg)
{
  /* free varbind list */
  vb_list_free(&xdg->vb_in_list);
  vb_list_free(&xdg->vb_out_list);
  xdg->vb_in_cnt = 0;
  xdg->vb_out_cnt = 0;
  /* free search range list */
  sr_list_free(&xdg->sr_in_list);
  sr_list_free(&xdg->sr_out_list);
  xdg->sr_in_cnt = 0;
  xdg->sr_out_cnt = 0;
  /* clear some fields */
  xdg->u.response.sys_up_time = 0;
  xdg->u.response.error = 0;
  xdg->u.response.index = 0;
  memset(xdg->context, 0, sizeof(xdg->context));
  xdg->ctx_len = 0;
}

/* Alloc buffer for var bind decoding */
static struct x_var_bind *
var_bind_alloc(uint8_t **buffer, uint8_t flag, enum agentx_err_code *err)
{
  struct x_var_bind *vb;
  uint16_t type;
  uint32_t oid_len, val_len;
  uint8_t *buf, *buf1;

  buf = *buffer;

  /* value type */
  if (flag & NETWORD_BYTE_ORDER) {
    type = NTOH16(*(uint16_t *)buf);
  } else {
    type = *(uint16_t *)buf;
  }
  buf += 4;

  /* oid length */
  buf1 = buf;
  oid_len = agentx_value_dec_try(buf1, flag, ASN1_TAG_OBJID);
  if (oid_len / sizeof(uint32_t) > MIB_OID_MAX_LEN) {
    *err = AGENTX_ERR_VB_OID_LEN;
    return NULL;
  }
  if (oid_len > 0) {
    buf += 4 + oid_len - 5 * sizeof(uint32_t);
  } else {
    buf += 4;
  }

  /* value length */
  val_len = agentx_value_dec_try(buf, flag, type);
  if (val_len > MIB_VALUE_MAX_LEN) {
    *err = AGENTX_ERR_VB_VALUE_LEN;
    return NULL;
  }

  /* varbind allocation */
  vb = vb_new(oid_len, val_len);
  if (vb == NULL) {
    *err = AGENTX_ERR_VB_VAR;
    return NULL;
  }

  /* OID assignment */
  vb->oid_len = agentx_value_dec(&buf1, flag, ASN1_TAG_OBJID, vb->oid);

  /* value assignment */
  vb->val_type = type;
  vb->val_len = agentx_value_dec(&buf, flag, type, vb->value);

  *buffer = buf;
  *err = AGENTX_ERR_OK;
  return vb;
}

/* Alloc buffer for search range decoding */
static struct x_search_range *
search_range_alloc(uint8_t **buffer, uint8_t flag, enum agentx_err_code *err)
{
  uint8_t *buf, *buf1;
  uint8_t start_include, end_include;
  uint32_t start_len, end_len;
  struct x_search_range *sr;

  buf1 = buf = *buffer;

  /* start oid length */
  start_len = agentx_value_dec_try(buf, flag, ASN1_TAG_OBJID);
  if (start_len / sizeof(uint32_t) > MIB_OID_MAX_LEN) {
    *err = AGENTX_ERR_SR_OID_LEN;
    return NULL;
  }
  start_include = *(buf + 2);
  if (start_len > 0) {
    buf += 4 + start_len - 5 * sizeof(uint32_t);
  } else {
    buf += 4;
  }

  /* end oid length */
  end_len = agentx_value_dec_try(buf, flag, ASN1_TAG_OBJID);
  if (end_len / sizeof(uint32_t) > MIB_OID_MAX_LEN) {
    *err = AGENTX_ERR_SR_OID_LEN;
    return NULL;
  }
  end_include = *(buf + 2);

  /* search range allocation */
  sr = sr_new(start_len, end_len);
  if (sr == NULL) {
    *err = AGENTX_ERR_SR_VAR;
    return NULL;
  }

  /* search range decoding */
  sr->start_include = start_include;
  sr->end_include = end_include;
  sr->start_len = agentx_value_dec(&buf1, flag, ASN1_TAG_OBJID, sr->start);
  sr->end_len = agentx_value_dec(&buf, flag, ASN1_TAG_OBJID, sr->end);

  *buffer = buf;
  return sr;
}

/* Parse varbind */
static AGENTX_ERR_CODE_E
var_bind_parse(struct agentx_datagram *xdg, uint8_t **buffer)
{
  AGENTX_ERR_CODE_E err;
  uint8_t *buf;

  err = AGENTX_ERR_OK;
  buf = *buffer;

  while (xdg->pdu_hdr.payload_length > 0) {
    /* Alloc a new var_bind and add into var_bind list. */
    struct x_var_bind *vb = var_bind_alloc(&buf, xdg->pdu_hdr.flags, &err);
    if (vb == NULL) {
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(agentx_err_msg, elem_num(agentx_err_msg), err));
      *buffer = buf;
      break;
    }
    list_add_tail(&vb->link, &xdg->vb_in_list);
    xdg->vb_in_cnt++;
    xdg->pdu_hdr.payload_length -= buf - *buffer;
    *buffer = buf;
  }

  return err;
}

/* Parse search range */
static AGENTX_ERR_CODE_E
search_range_parse(struct agentx_datagram *xdg, uint8_t **buffer)
{
  AGENTX_ERR_CODE_E err;
  uint8_t *buf;

  err = AGENTX_ERR_OK;
  buf = *buffer;

  while (xdg->pdu_hdr.payload_length > 0) {
    /* Alloc a new search range and add into search range list. */
    struct x_search_range *sr = search_range_alloc(&buf, xdg->pdu_hdr.flags, &err);
    if (sr == NULL) {
      SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(agentx_err_msg, elem_num(agentx_err_msg), err));
      *buffer = buf;
      break;
    }
    list_add_tail(&sr->link, &xdg->sr_in_list);
    xdg->sr_in_cnt++;
    xdg->pdu_hdr.payload_length -= buf - *buffer;
    *buffer = buf;
  }

  return err;
}

/* Parse PDU header */
static AGENTX_ERR_CODE_E
pdu_hdr_parse(struct agentx_datagram *xdg, uint8_t **buffer)
{
  AGENTX_ERR_CODE_E err;
  uint8_t *buf;

  err = AGENTX_ERR_OK;
  buf = *buffer;

  /* PDU header */
  xdg->pdu_hdr.version = *buf++;
  xdg->pdu_hdr.type = *buf++;
  xdg->pdu_hdr.flags = *buf++;
  xdg->pdu_hdr.reserved = *buf++;
  if (xdg->pdu_hdr.flags & NETWORD_BYTE_ORDER) {
    xdg->pdu_hdr.session_id = NTOH32(*(uint32_t *)buf);
    buf += sizeof(uint32_t);
    xdg->pdu_hdr.transaction_id = NTOH32(*(uint32_t *)buf);
    buf += sizeof(uint32_t);
    xdg->pdu_hdr.packet_id = NTOH32(*(uint32_t *)buf);
    buf += sizeof(uint32_t);
    xdg->pdu_hdr.payload_length = NTOH32(*(uint32_t *)buf);
    buf += sizeof(uint32_t);
  } else {
    xdg->pdu_hdr.session_id = *(uint32_t *)buf;
    buf += sizeof(uint32_t);
    xdg->pdu_hdr.transaction_id = *(uint32_t *)buf;
    buf += sizeof(uint32_t);
    xdg->pdu_hdr.packet_id = *(uint32_t *)buf;
    buf += sizeof(uint32_t);
    xdg->pdu_hdr.payload_length = *(uint32_t *)buf;
    buf += sizeof(uint32_t);
  }

  /* Optinal context */
  if (xdg->pdu_hdr.flags & NON_DEFAULT_CONTEXT) {
    if (xdg->pdu_hdr.flags & NETWORD_BYTE_ORDER) {
      xdg->ctx_len = NTOH32(*(uint32_t *)buf);
    } else {
      xdg->ctx_len = *(uint32_t *)buf;
    }
    if (xdg->ctx_len + 1 > sizeof(xdg->context)) {
      err = AGENTX_ERR_PDU_CTX_LEN;
      *buffer = buf;
      return err;
    }
    buf += sizeof(uint32_t);
    memcpy(xdg->context, buf, xdg->ctx_len);
    buf += uint_sizeof(xdg->ctx_len);
    xdg->pdu_hdr.payload_length -= 4 + uint_sizeof(xdg->ctx_len);
  }

  /* additional data */
  switch (xdg->pdu_hdr.type) {
    case AGENTX_PDU_CLOSE:
      xdg->u.close.reason = *buf;
      buf += sizeof(uint32_t);
      xdg->pdu_hdr.payload_length -= sizeof(uint32_t);
      break;
    case AGENTX_PDU_GETBULK:
      if (xdg->pdu_hdr.flags & NETWORD_BYTE_ORDER) {
        xdg->u.getbulk.non_rep = NTOH16(*(uint16_t *)buf);
      } else {
        xdg->u.getbulk.non_rep = *(uint16_t *)buf;
      }
      buf += sizeof(uint16_t);
      if (xdg->pdu_hdr.flags & NETWORD_BYTE_ORDER) {
        xdg->u.getbulk.max_rep = NTOH16(*(uint16_t *)buf);
      } else {
        xdg->u.getbulk.max_rep = *(uint16_t *)buf;
      }
      buf += sizeof(uint16_t);
      xdg->pdu_hdr.payload_length -= 2 * sizeof(uint16_t);
      break;
    case AGENTX_PDU_RESPONSE:
      if (xdg->pdu_hdr.flags & NETWORD_BYTE_ORDER) {
        xdg->u.response.sys_up_time = NTOH32(*(uint32_t *)buf);
      } else {
        xdg->u.response.sys_up_time = *(uint32_t *)buf;
      }
      buf += sizeof(uint32_t);
      if (xdg->pdu_hdr.flags & NETWORD_BYTE_ORDER) {
        xdg->u.response.error = NTOH16(*(uint16_t *)buf);
      } else {
        xdg->u.response.error = *(uint16_t *)buf;
      }
      buf += sizeof(uint16_t);
      xdg->u.response.index = *(uint16_t *)buf;
      if (xdg->pdu_hdr.flags & NETWORD_BYTE_ORDER) {
        xdg->u.response.index = NTOH16(*(uint16_t *)buf);
      } else {
        xdg->u.response.index = *(uint16_t *)buf;
      }
      buf += sizeof(uint16_t);
      xdg->pdu_hdr.payload_length -= sizeof(uint32_t) + 2 * sizeof(uint16_t);
    default:
      break;
  }

  *buffer = buf;
  return err;
}

/* Decode agentx datagram */
static AGENTX_ERR_CODE_E
agentx_decode(struct agentx_datagram *xdg)
{
  AGENTX_ERR_CODE_E err;
  uint8_t *buf, dec_fail = 0;

  buf = xdg->recv_buf;

  /* PDU header */
  err = pdu_hdr_parse(xdg, &buf);
  if (err) {
    SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(agentx_err_msg, elem_num(agentx_err_msg), err));
    dec_fail = 1;
    goto DECODE_FINISH;
  }

  /* varbind or search range */
  switch (xdg->pdu_hdr.type) {
    case AGENTX_PDU_GET:
    case AGENTX_PDU_GETNEXT:
    case AGENTX_PDU_GETBULK:
      /* search range */
      err = search_range_parse(xdg, &buf);
      if (err) {
        SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(agentx_err_msg, elem_num(agentx_err_msg), err));
        dec_fail = 1;
      }
      break;
    case AGENTX_PDU_TESTSET:
    case AGENTX_PDU_RESPONSE:
      /* var bind */
      err = var_bind_parse(xdg, &buf);
      if (err) {
        SMARTSNMP_LOG(L_ERROR, "ERR(%d): %s\n", err, error_message(agentx_err_msg, elem_num(agentx_err_msg), err));
        dec_fail = 1;
      }
      break;
    default:
      break;
  }

DECODE_FINISH:
  /* If fail, do some clear things */
  if (dec_fail) {
    agentx_datagram_clear(xdg);
  }

  /* We should free received buf here */
  free(xdg->recv_buf);

  return err;
}

/* AgentX request dispatch */
static void
agentx_request_dispatch(struct agentx_datagram *xdg)
{
  switch (xdg->pdu_hdr.type) {
    case AGENTX_PDU_GET:
      agentx_get(xdg);
      break;
    case AGENTX_PDU_GETNEXT:
    case AGENTX_PDU_GETBULK:
      agentx_getnext(xdg);
      break;
    case AGENTX_PDU_TESTSET:
      agentx_set(xdg);
      break;
    case AGENTX_PDU_COMMITSET:
    case AGENTX_PDU_UNDOSET:
    case AGENTX_PDU_CLEANUPSET:
      agentx_response(xdg);
      break;
    case AGENTX_PDU_INDEXALLOC:
    case AGENTX_PDU_INDEXDEALLOC:
    case AGENTX_PDU_ADDAGENTCAP:
    case AGENTX_PDU_REMOVEAGENTCAP:
      break;
    default:
      break;
  }
}

/* Receive agentx datagram from transport module */
int
agentx_recv(uint8_t *buffer, int len)
{
  int ret;

  assert(buffer != NULL && len > 0);

  /* Reset agentX datagram */
  agentx_datagram_clear(&agentx_datagram);
  agentx_datagram.recv_buf = buffer;

  /* Decode agentX datagram */
  ret = agentx_decode(&agentx_datagram);

  if (!ret) {
    /* Dispatch agentX request */
    agentx_request_dispatch(&agentx_datagram);
  }

  return ret;
}
