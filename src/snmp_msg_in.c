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

static struct snmp_datagram snmp_datagram;

/* Embedded code is not funny at all... */
int
mib_instance_search(struct oid_search_res *ret_oid)
{
  int i;
  Variable *var = &ret_oid->var;
  lua_State *L = snmp_datagram.lua_state;

  /* Empty lua stack. */
  lua_pop(L, -1);
  /* Get function. */
  lua_getglobal(L, ret_oid->callback);
  /* op */
  lua_pushinteger(L, ret_oid->request);
  /* Community authorization */
  lua_pushstring(L, snmp_datagram.community);
  /* req_sub_oid */
  lua_newtable(L);
  for (i = 0; i < ret_oid->inst_id_len; i++) {
    lua_pushinteger(L, ret_oid->inst_id[i]);
    lua_rawseti(L, -2, i + 1);
  }

  if (ret_oid->request == SNMP_REQ_SET) {
    /* req_val */
    switch (tag(var)) {
      case BER_TAG_INT:
        lua_pushinteger(L, integer(var));
        break;
      case BER_TAG_OCTSTR:
        lua_pushlstring(L, octstr(var), length(var));
        break;
      case BER_TAG_CNT:
        lua_pushnumber(L, count(var));
        break;
      case BER_TAG_IPADDR:
        lua_pushlstring(L, (char *)ipaddr(var), length(var));
        break;
      case BER_TAG_OBJID:
        lua_newtable(L);
        for (i = 0; i < length(var); i++) {
          lua_pushnumber(L, oid(var)[i]);
          lua_rawseti(L, -2, i + 1);
        }
        break;
      case BER_TAG_GAU:
        lua_pushnumber(L, gauge(var));
        break;
      case BER_TAG_TIMETICKS:
        lua_pushnumber(L, timiticks(var));
        break;
      default:
        lua_pushnil(L);
        break;
    }
    /* req_val_type */
    lua_pushinteger(L, tag(var));
  } else {
    /* req_val */
    lua_pushnil(L);
    /* req_val_type */
    lua_pushnil(L);
  }

  if (lua_pcall(L, 5, 4, 0) != 0) {
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "Lua call function %s fail\n", ret_oid->callback);
    ret_oid->exist_state = BER_TAG_NO_SUCH_OBJ;
    return 0;
  }

  ret_oid->exist_state = lua_tointeger(L, -4);

  if (ret_oid->exist_state == 0) {
    if (ret_oid->request != SNMP_REQ_SET) {
      tag(var) = lua_tonumber(L, -1);
      switch (tag(var)) {
        case BER_TAG_INT:
          length(var) = 1;
          integer(var) = lua_tointeger(L, -2);
          break;
        case BER_TAG_OCTSTR:
          length(var) = lua_objlen(L, -2);
          memcpy(octstr(var), lua_tostring(L, -2), length(var));
          break;
        case BER_TAG_CNT:
          length(var) = 1;
          count(var) = lua_tonumber(L, -2);
          break;
        case BER_TAG_IPADDR:
          length(var) = lua_objlen(L, -2);
          for (i = 0; i < length(var); i++) {
            lua_rawgeti(L, -2, i + 1);
            ipaddr(var)[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
          }
          break;
        case BER_TAG_OBJID:
          length(var) = lua_objlen(L, -2);
          for (i = 0; i < length(var); i++) {
            lua_rawgeti(L, -2, i + 1);
            oid(var)[i] = lua_tointeger(L, -1);
            lua_pop(L, 1);
          }
          break;
        case BER_TAG_GAU:
          length(var) = 1;
          gauge(var) = lua_tonumber(L, -2);
          break;
        case BER_TAG_TIMETICKS:
          length(var) = 1;
          timiticks(var) = lua_tonumber(L, -2);
          break;
        default:
          assert(0);
          break;
      }
    }

    /* For GETNEXT request, return the new oid */
    if (ret_oid->request == SNMP_REQ_GETNEXT) {
      ret_oid->inst_id_len = lua_objlen(L, -3);
      for (i = 0; i < ret_oid->inst_id_len; i++) {
        lua_rawgeti(L, -3, i + 1);
        ret_oid->inst_id[i] = lua_tointeger(L, -1);
        lua_pop(L, 1);
      }
    }
  }

  return ret_oid->exist_state;
}

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
snmp_msg_clear(struct snmp_datagram *sdg)
{
  snmp_vb_list_free(&sdg->vb_in_list);
  snmp_vb_list_free(&sdg->vb_out_list);
  sdg->vb_in_cnt = 0;
  sdg->vb_out_cnt = 0;
  free(sdg->version);
  free(sdg->community);
}

static void
snmp_response(struct snmp_datagram *sdg)
{
  if (sdg->pdu_hdr.err_stat) {
    sdg->pdu_hdr.err_idx = sdg->vb_in_cnt;
  } else {
    sdg->pdu_hdr.err_idx = 0;
  }

  snmp_vb_list_free(&sdg->vb_in_list);
  sdg->vb_in_cnt = 0;

  snmp_send_response(sdg);

  snmp_vb_list_free(&sdg->vb_out_list);
  sdg->vb_out_cnt = 0;
}

/* GET request function */
static void
snmp_get(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len, val_len;
  const uint32_t tag_len = 1;

  ret_oid.request = SNMP_REQ_GET;

  list_for_each_safe(curr, next, &sdg->vb_in_list) {
    vb_in = list_entry(curr, struct var_bind, link);

    /* Search at the input oid */
    mib_tree_search(vb_in->oid, vb_in->oid_len, &ret_oid);

    if (ret_oid.exist_state) {
      /* Community authorization */
      if (ret_oid.exist_state == SNMP_ERR_STAT_AUTHORIZATION) {
        CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: Community authorization failure\n");
        snmp_msg_clear(sdg);
        return;
      }
      /* not exist */
      vb_out = xmalloc(sizeof(*vb_out));
      vb_out->oid = ret_oid.oid;
      vb_out->oid_len = ret_oid.id_len;
      vb_out->value_type = ret_oid.exist_state;
      vb_out->value_len = 0;
    } else {
      /* Gotcha */
      val_len = ber_value_enc_test(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var));
      vb_out = xmalloc(sizeof(*vb_out) + val_len);
      vb_out->oid = ret_oid.oid;
      vb_out->oid_len = ret_oid.id_len;
      vb_out->value_type = tag(&ret_oid.var);
      vb_out->value_len = ber_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);
    }

    /* Test OID encoding length. */
    oid_len = ber_value_enc_test(vb_out->oid, vb_out->oid_len, BER_TAG_OBJID);
    len_len = ber_length_enc_test(oid_len);
    vb_out->vb_len = tag_len + len_len + oid_len;

    /* Test value encoding length. */
    len_len = ber_length_enc_test(vb_out->value_len);
    vb_out->vb_len += tag_len + len_len + vb_out->value_len;

    /* Test varbind encoding length. */
    len_len = ber_length_enc_test(vb_out->vb_len);
    sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

    /* Add into list. */
    list_add_tail(&vb_out->link, &sdg->vb_out_list);
    sdg->vb_out_cnt++;
  }

  snmp_response(sdg);
}

/* GETNEXT request function */
static void
snmp_getnext(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len, val_len;
  const uint32_t tag_len = 1;

  ret_oid.request = SNMP_REQ_GETNEXT;

  list_for_each_safe(curr, next, &sdg->vb_in_list) {
    vb_in = list_entry(curr, struct var_bind, link);

    /* Search at the input oid */
    mib_tree_search_next(vb_in->oid, vb_in->oid_len, &ret_oid);

    if (ret_oid.exist_state) {
      /* Community authorization */
      if (ret_oid.exist_state == SNMP_ERR_STAT_AUTHORIZATION) {
        CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: Community authorization failure\n");
        snmp_msg_clear(sdg);
        return;
      }
      /* This situation is only for traversal when end-of-mib-tree */
      vb_out = xmalloc(sizeof(*vb_out));
      vb_out->oid = ret_oid.oid;
      vb_out->oid_len = ret_oid.id_len;
      vb_out->value_type = ret_oid.exist_state;
      vb_out->value_len = 0;
    } else {
      val_len = ber_value_enc_test(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var));
      vb_out = xmalloc(sizeof(*vb_out) + val_len);
      vb_out->oid = ret_oid.oid;
      vb_out->oid_len = ret_oid.id_len;
      vb_out->value_type = tag(&ret_oid.var);
      vb_out->value_len = ber_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);
    }

    /* Test OID encoding length. */
    oid_len = ber_value_enc_test(vb_out->oid, vb_out->oid_len, BER_TAG_OBJID);
    len_len = ber_length_enc_test(oid_len);
    vb_out->vb_len = tag_len + len_len + oid_len;

    /* Test value encoding length. */
    len_len = ber_length_enc_test(vb_out->value_len);
    vb_out->vb_len += tag_len + len_len + vb_out->value_len;

    /* Test varbind encoding length. */
    len_len = ber_length_enc_test(vb_out->vb_len);
    sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

    /* Add into list. */
    list_add_tail(&vb_out->link, &sdg->vb_out_list);
    sdg->vb_out_cnt++;
  }

  snmp_response(sdg);
}

/* SET request function */
static void
snmp_set(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len;
  const uint32_t tag_len = 1;

  ret_oid.request = SNMP_REQ_SET;

  list_for_each_safe(curr, next, &sdg->vb_in_list) {
    vb_in = list_entry(curr, struct var_bind, link);

    /* Decode the setting value ahead */
    tag(&ret_oid.var) = vb_in->value_type;
    length(&ret_oid.var) = ber_value_dec(vb_in->value, vb_in->value_len, tag(&ret_oid.var), value(&ret_oid.var));

    /* Search at the input oid and set it */
    mib_tree_search(vb_in->oid, vb_in->oid_len, &ret_oid);

    /* Community authorization */
    if (ret_oid.exist_state == SNMP_ERR_STAT_AUTHORIZATION) {
      CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: Community authorization failure\n");
      snmp_msg_clear(sdg);
      return;
    }

    vb_out = xmalloc(sizeof(*vb_out) + vb_in->value_len);
    vb_out->oid = ret_oid.oid;
    vb_out->oid_len = ret_oid.id_len;
    vb_out->value_type = vb_in->value_type;
    vb_out->value_len = vb_in->value_len;
    memcpy(vb_out->value, vb_in->value, vb_out->value_len);

    if (ret_oid.exist_state >= BER_TAG_NO_SUCH_OBJ) {
      /* Object not found */
      vb_out->value_type = ret_oid.exist_state;
    } else {
      /* 0 means success, while others mean some failure */
      sdg->pdu_hdr.err_stat = ret_oid.exist_state;
    }

    /* Test OID encoding length. */
    oid_len = ber_value_enc_test(vb_out->oid, vb_out->oid_len, BER_TAG_OBJID);
    len_len = ber_length_enc_test(oid_len);
    vb_out->vb_len = tag_len + len_len + oid_len;

    /* Test value encoding length. */
    len_len = ber_length_enc_test(vb_out->value_len);
    vb_out->vb_len += tag_len + len_len + vb_out->value_len;

    /* Test varbind encoding length. */
    len_len = ber_length_enc_test(vb_out->vb_len);
    sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

    /* Add into list. */
    list_add_tail(&vb_out->link, &sdg->vb_out_list);
    sdg->vb_out_cnt++;
  }

  snmp_response(sdg);
}

/* BULKGET request function */
void
snmp_bulkget(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len, val_len, id_len;
  const uint32_t tag_len = 1;
  const oid_t *oid;

  ret_oid.request = SNMP_REQ_GETNEXT;

  while (sdg->pdu_hdr.err_idx-- > 0) {

    list_for_each_safe(curr, next, &sdg->vb_in_list) {
      vb_in = list_entry(curr, struct var_bind, link);

      oid = vb_in->oid;
      id_len = vb_in->oid_len;

      /* Search at the input oid */
      mib_tree_search_next(oid, id_len, &ret_oid);

      /* Return oid for the next query. */
      free(vb_in->oid);
      vb_in->oid = oid_dup(ret_oid.oid, ret_oid.id_len);
      vb_in->oid_len = ret_oid.id_len;

      if (ret_oid.exist_state) {
        /* Community authorization */
        if (ret_oid.exist_state == SNMP_ERR_STAT_AUTHORIZATION) {
          CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: Community authorization failure\n");
          snmp_msg_clear(sdg);
          return;
        }
        /* This situation is only for traversal when end-of-mib-tree */
        vb_out = xmalloc(sizeof(*vb_out));
        vb_out->oid = ret_oid.oid;
        vb_out->oid_len = ret_oid.id_len;
        vb_out->value_type = ret_oid.exist_state;
        vb_out->value_len = 0;
      } else {
        val_len = ber_value_enc_test(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var));
        vb_out = xmalloc(sizeof(*vb_out) + val_len);
        vb_out->oid = ret_oid.oid;
        vb_out->oid_len = ret_oid.id_len;
        vb_out->value_type = tag(&ret_oid.var);
        vb_out->value_len = ber_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);
      }

      /* Test OID encoding length. */
      oid_len = ber_value_enc_test(vb_out->oid, vb_out->oid_len, BER_TAG_OBJID);
      len_len = ber_length_enc_test(oid_len);
      vb_out->vb_len = tag_len + len_len + oid_len;

      /* Test value encoding length. */
      len_len = ber_length_enc_test(vb_out->value_len);
      vb_out->vb_len += tag_len + len_len + vb_out->value_len;

      /* Test varbind encoding length. */
      len_len = ber_length_enc_test(vb_out->vb_len);
      sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

      /* Add into list. */
      list_add_tail(&vb_out->link, &sdg->vb_out_list);
      sdg->vb_out_cnt++;
    }
  }

  snmp_response(sdg);
}

/* Request callback */
static void
event_invoke(struct snmp_datagram *sdg)
{
  switch (sdg->pdu_hdr.pdu_type) {
    case SNMP_REQ_GET:
      snmp_get(sdg);
      break;
    case SNMP_REQ_GETNEXT:
      snmp_getnext(sdg);
      break;
    case SNMP_RESP:
      break;
    case SNMP_REQ_SET:
      snmp_set(sdg);
      break;
    case SNMP_REQ_BULKGET:
      snmp_bulkget(sdg);
      break;
    case SNMP_REQ_INF:
      break;
    case SNMP_TRAP:
      break;
    case SNMP_REPO:
      break;
    default:
      break;
  }
}

/* Alloc buffer for var bind decoding */
static struct var_bind *
var_bind_alloc(uint8_t *buf, enum snmp_err_code *err)
{
  struct var_bind *vb;
  uint8_t oid_t, val_t;
  uint32_t oid_len, oid_dec_len, val_len;
  uint8_t *buf1;

  /* OID */
  oid_t = *buf++;
  if (oid_t != BER_TAG_OBJID) {
    *err = SNMP_ERR_VB_TYPE;
    return NULL;
  }
  buf += ber_length_dec(buf, &oid_len);
  buf1 = buf;
  buf += oid_len;

  /* OID decoding length test, keep from overflow. */
  oid_dec_len = ber_value_dec_test(buf1, oid_len, BER_TAG_OBJID);
  if (oid_dec_len > MIB_OID_MAX_LEN) {
    *err = SNMP_ERR_VB_OID_LEN;
    return NULL;
  }

  /* Value */
  val_t = *buf++;
  buf += ber_length_dec(buf, &val_len);
  if (val_len > MIB_VALUE_MAX_LEN) {
    *err = SNMP_ERR_VB_VALUE_LEN;
    return NULL;
  }

  /* varbind allocation */
  vb = vb_new(oid_dec_len, val_len);
  if (vb == NULL) {
    *err = SNMP_ERR_VB_VAR;
    return NULL;
  }

  /* vb->oid_len is the actually length of oid */
  vb->oid_len = ber_value_dec(buf1, oid_len, BER_TAG_OBJID, vb->oid);

  /* Value */
  vb->value_type = val_t;
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
  if (*buf++ != BER_TAG_INT) {
    err = SNMP_ERR_PDU_REQID;
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
    return err;
  }
  buf += ber_length_dec(buf, &ph->req_id_len);
  ber_value_dec(buf, ph->req_id_len, BER_TAG_INT, &ph->req_id);
  buf += ph->req_id_len;

  /* Error status */
  if (*buf++ != BER_TAG_INT) {
    err = SNMP_ERR_PDU_ERRSTAT;
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
    return err;
  }
  buf += ber_length_dec(buf, &ph->err_stat_len);
  ber_value_dec(buf, ph->err_stat_len, BER_TAG_INT, &ph->err_stat);
  buf += ph->err_stat_len;

  /* Error index */
  if (*buf++ != BER_TAG_INT) {
    err = SNMP_ERR_PDU_ERRIDX;
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
    return err;
  }
  buf += ber_length_dec(buf, &ph->err_idx_len);
  ber_value_dec(buf, ph->err_idx_len, BER_TAG_INT, &ph->err_idx);
  buf += ph->err_idx_len;

  *buffer = buf;
  return err;
}

/* Parse varbind */
static SNMP_ERR_CODE_E
var_bind_parse(struct snmp_datagram *sdg, uint8_t **buffer)
{
  SNMP_ERR_CODE_E err;
  struct var_bind *vb = NULL;
  uint8_t *buf;
  uint32_t vb_len, len_len;
  const uint32_t tag_len = 1;

  err = SNMP_ERR_OK;
  buf = *buffer;

  if (*buf++ != BER_TAG_SEQ) {
    err = SNMP_ERR_VB_TYPE;
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
    return err;
  }
  buf += ber_length_dec(buf, &sdg->vb_list_len);

  while (sdg->vb_list_len > 0) {
    /* check vb_list type */
    if (*buf++ != BER_TAG_SEQ) {
      err = SNMP_ERR_VB_TYPE;
      CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
      break;
    }
    len_len = ber_length_dec(buf, &vb_len);
    buf += len_len;

    /* Alloc a new var_bind and add into var_bind list. */
    vb = var_bind_alloc(buf, &err);
    if (vb == NULL) {
      CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
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

/* Decode snmp datagram */
static void
asn1_decode(struct snmp_datagram *sdg)
{
  SNMP_ERR_CODE_E err;
  uint8_t *buffer, dec_fail = 0;
  const uint32_t tag_len = 1;

  /* Skip tag and length */
  buffer = sdg->recv_buf + tag_len;
  buffer += ber_length_dec(buffer, &snmp_datagram.data_len);

  /* version */
  if (*buffer++ != BER_TAG_INT) {
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", SNMP_ERR_PDU_TYPE);
    dec_fail = 1;
    goto DECODE_FINISH;
  }
  buffer += ber_length_dec(buffer, &sdg->ver_len);
  sdg->version = xmalloc(sizeof(integer_t));
  ber_value_dec(buffer, sdg->ver_len, BER_TAG_INT, sdg->version);
  buffer += sdg->ver_len;

  /* community */
  if (*buffer++ != BER_TAG_OCTSTR) {
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", SNMP_ERR_PDU_TYPE);
    dec_fail = 1;
    goto DECODE_FINISH;
  }
  buffer += ber_length_dec(buffer, &sdg->comm_len);
  sdg->community = xcalloc(sdg->comm_len + 1, sizeof(octstr_t));
  ber_value_dec(buffer, sdg->comm_len, BER_TAG_OCTSTR, sdg->community);
  buffer += sdg->comm_len;

  /* PDU header */
  err = pdu_hdr_parse(sdg, &buffer);
  if (err) {
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
    dec_fail = 1;
    goto DECODE_FINISH;
  }

  /* var bind */
  err = var_bind_parse(sdg, &buffer);
  if (err) {
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", err);
    dec_fail = 1;
    goto DECODE_FINISH;
  }

DECODE_FINISH:
  /* If fail, do some clear things */
  if (dec_fail) {
    snmp_msg_clear(sdg);
  }

  /* We should free receive buffer here */
  free(sdg->recv_buf);
}

/* Receive snmp datagram from transport module */
void snmp_recv(uint8_t *buffer, int len, void *arg)
{
  uint32_t len_len;
  const uint32_t tag_len = 1;

  assert(buffer != NULL && len > 0 && arg != NULL);

  /* Check buffer type and length. */
  len_len = ber_length_dec(buffer + tag_len, &snmp_datagram.data_len);
  if (buffer[0] != BER_TAG_SEQ || tag_len + len_len + snmp_datagram.data_len != len) {
    CREDO_SNMP_LOG(SNMP_LOG_ERROR, "ERR: %d\n", SNMP_ERR_PDU_LEN);
    free(buffer);
    return;
  }

  memset(&snmp_datagram, 0, sizeof(snmp_datagram));
  snmp_datagram.lua_state = arg;
  snmp_datagram.recv_buf = buffer;
  INIT_LIST_HEAD(&snmp_datagram.vb_in_list);
  INIT_LIST_HEAD(&snmp_datagram.vb_out_list);

  /* Decode asn.1 data */
  asn1_decode(&snmp_datagram);

  /* Invoke relavant events. */
  event_invoke(&snmp_datagram);
}
