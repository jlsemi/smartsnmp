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

#include "mib.h"
#include "agentx.h"
#include "util.h"

static oid_t agentx_dummy_view[] = { 1, 3, 6, 1 };

static void
mib_get(struct agentx_datagram *xdg, struct x_search_range *sr_in, struct oid_search_res *ret_oid)
{
  struct mib_view view;
  
  view.oid = agentx_dummy_view;
  view.id_len = elem_num(agentx_dummy_view);

  mib_tree_search(&view, sr_in->start, sr_in->start_len, ret_oid);
}

void
agentx_get(struct agentx_datagram *xdg)
{
  uint32_t val_len, sr_in_cnt = 0;
  struct list_head *curr, *next;
  struct x_var_bind *vb_out;
  struct x_search_range *sr_in;
  struct oid_search_res ret_oid;

  memset(&ret_oid, 0, sizeof(ret_oid));
  ret_oid.request = MIB_REQ_GET; 

  list_for_each_safe(curr, next, &xdg->sr_in_list) {
    sr_in = list_entry(curr, struct x_search_range, link);
    sr_in_cnt++;

    /* Search at the input oid */
    mib_get(xdg, sr_in, &ret_oid);

    val_len = agentx_value_enc_try(length(&ret_oid.var), tag(&ret_oid.var));
    vb_out = xmalloc(sizeof(*vb_out) + val_len);
    vb_out->oid = ret_oid.oid;
    vb_out->oid_len = ret_oid.id_len;
    vb_out->val_type = tag(&ret_oid.var);
    vb_out->val_len = agentx_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);

    /* Error status */
    if (ret_oid.err_stat) {
      if (!xdg->u.response.error) {
        /* Report the first object error status in search range */
        xdg->u.response.error = ret_oid.err_stat;
        xdg->u.response.index = sr_in_cnt;
      }
    }

    /* Add into list. */
    list_add_tail(&vb_out->link, &xdg->vb_out_list);
    xdg->vb_out_cnt++;
  }

  agentx_response(xdg);
}

static void
mib_getnext(struct agentx_datagram *xdg, struct x_search_range *sr_in, struct oid_search_res *ret_oid)
{
  struct mib_view view;
  
  view.oid = agentx_dummy_view;
  view.id_len = elem_num(agentx_dummy_view);

  /* Search at the included start oid */
  if (sr_in->start_include) {
    mib_tree_search(&view, sr_in->start, sr_in->start_len, ret_oid);
    if (ret_oid->err_stat || !MIB_TAG_VALID(tag(&ret_oid->var))) {
      /* Invalid query */
      free(ret_oid->oid);
    }
  }

  /* If start oid not included or not exist, search the next one */
  if (!sr_in->start_include || ret_oid->err_stat || !MIB_TAG_VALID(tag(&ret_oid->var))) {
    mib_tree_search_next(&view, sr_in->start, sr_in->start_len, ret_oid);
    if (!ret_oid->err_stat && MIB_TAG_VALID(tag(&ret_oid->var))) {
      /* Check whether return oid exceeds end oid */
      if ((sr_in->end_include && oid_cmp(ret_oid->oid, ret_oid->id_len, sr_in->end, sr_in->end_len) > 0) ||
          (!sr_in->end_include && oid_cmp(ret_oid->oid, ret_oid->id_len, sr_in->end, sr_in->end_len) >= 0)) {
        /* Oid exceeds, end_of_mib_view */
        oid_cpy(ret_oid->oid, sr_in->start, sr_in->start_len);
        ret_oid->id_len = sr_in->start_len;
        ret_oid->inst_id = NULL;
        ret_oid->inst_id_len = 0;
        tag(&ret_oid->var) = ASN1_TAG_END_OF_MIB_VIEW;
      }
    }
  }
}

void
agentx_getnext(struct agentx_datagram *xdg)
{
  uint32_t val_len, sr_in_cnt = 0;
  struct list_head *curr, *next;
  struct x_var_bind *vb_out;
  struct x_search_range *sr_in;
  struct oid_search_res ret_oid;

  memset(&ret_oid, 0, sizeof(ret_oid));
  ret_oid.request = MIB_REQ_GETNEXT;

  list_for_each_safe(curr, next, &xdg->sr_in_list) {
    sr_in = list_entry(curr, struct x_search_range, link);
    sr_in_cnt++;

    /* Search at the input next oid */
    mib_getnext(xdg, sr_in, &ret_oid);

    val_len = agentx_value_enc_try(length(&ret_oid.var), tag(&ret_oid.var));
    vb_out = xmalloc(sizeof(*vb_out) + val_len);
    vb_out->oid = ret_oid.oid;
    vb_out->oid_len = ret_oid.id_len;
    vb_out->val_type = tag(&ret_oid.var);
    vb_out->val_len = agentx_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);

    /* Error status */
    if (ret_oid.err_stat) {
      if (!xdg->u.response.error) {
        /* Report the first object error status in search range */
        xdg->u.response.error = ret_oid.err_stat;
        xdg->u.response.index = sr_in_cnt;
      }
    }

    /* Add into list. */
    list_add_tail(&vb_out->link, &xdg->vb_out_list);
    xdg->vb_out_cnt++;
  }

  agentx_response(xdg);
}

static void
mib_set(struct agentx_datagram *xdg, struct x_var_bind *vb_in, struct oid_search_res *ret_oid)
{
  struct mib_view view;
  
  view.oid = agentx_dummy_view;
  view.id_len = elem_num(agentx_dummy_view);

  mib_tree_search(&view, vb_in->oid, vb_in->oid_len, ret_oid);
}

void
agentx_set(struct agentx_datagram *xdg)
{
  uint32_t val_len, vb_in_cnt = 0;
  struct list_head *curr, *next;
  struct x_var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;

  memset(&ret_oid, 0, sizeof(ret_oid));
  ret_oid.request = MIB_REQ_SET;

  list_for_each_safe(curr, next, &xdg->vb_in_list) {
    vb_in = list_entry(curr, struct x_var_bind, link);
    vb_in_cnt++;

    /* Decode the setting value ahead */
    tag(&ret_oid.var) = vb_in->val_type;
    length(&ret_oid.var) = vb_in->val_len;
    val_len = agentx_value_enc_try(length(&ret_oid.var), tag(&ret_oid.var));
    memcpy(value(&ret_oid.var), vb_in->value, val_len);

    /* Search at the input oid and set it */
    mib_set(xdg, vb_in, &ret_oid);
    
    val_len = agentx_value_enc_try(length(&ret_oid.var), tag(&ret_oid.var));
    vb_out = xmalloc(sizeof(*vb_out) + val_len);
    vb_out->oid = ret_oid.oid;
    vb_out->oid_len = ret_oid.id_len;
    vb_out->val_type = vb_in->val_type;
    vb_out->val_len = agentx_value_enc(value(&ret_oid.var), val_len, tag(&ret_oid.var), vb_out->value);

    /* Invalid tags convert to error status for snmpset */
    if (!ret_oid.err_stat && !MIB_TAG_VALID(tag(&ret_oid.var))) {
      ret_oid.err_stat = AGENTX_ERR_STAT_NOT_WRITABLE;
    }

    if (ret_oid.err_stat) {
      /* Something wrong */
      if (!xdg->u.response.error) {
        /* Report the first object error status in search range */
        xdg->u.response.error = ret_oid.err_stat;
        xdg->u.response.index = vb_in_cnt;
      }
    }

    /* Add into list. */
    list_add_tail(&vb_out->link, &xdg->vb_out_list);
    xdg->vb_out_cnt++;
  }

  agentx_response(xdg);
}
