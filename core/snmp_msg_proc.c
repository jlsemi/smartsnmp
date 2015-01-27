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
#include "util.h"

static void
mib_get(struct snmp_datagram *sdg, struct var_bind *vb_in, struct oid_search_res *ret_oid)
{
  struct mib_view *view = NULL;
  struct mib_community *community = NULL;
  struct mib_user *user = NULL;

  /* Access control */
  if (sdg->version >= 3) {
    user = mib_user_search(sdg->user_name);
    if (user == NULL) {
      ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
    }
  } else {
    community = mib_community_search(sdg->context_name);
    if (community == NULL) {
      ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
    }
  }

  /* Traverse all availble views according to community or user */
  for (; ;) {
    if (sdg->version >= 3) {
        view = mib_user_next_view(user, MIB_ACES_READ, view);
    } else {
        view = mib_community_next_view(community, MIB_ACES_READ, view);
    }

    /* End of mib view */
    if (view == NULL) {
      /* Duplicate original oid when result not found */
      ret_oid->oid = oid_dup(vb_in->oid, vb_in->oid_len);
      ret_oid->id_len = vb_in->oid_len;
      return;
    }

    mib_tree_search(view, vb_in->oid, vb_in->oid_len, ret_oid);
    if ((!ret_oid->err_stat && MIB_TAG_VALID(tag(&ret_oid->var))) || oid_cmp(vb_in->oid, vb_in->oid_len, view->oid, view->id_len) < 0) {
      /* Gotcha or given oid ahead of all views */
      return;
    }
    /* Free temporary result */
    free(ret_oid->oid);
  }
}

void
snmp_get(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len, val_len;
  uint32_t vb_in_cnt = 0;
  const uint32_t tag_len = 1;

  memset(&ret_oid, 0, sizeof(ret_oid));
  ret_oid.request = MIB_REQ_GET;

  list_for_each_safe(curr, next, &sdg->vb_in_list) {
    vb_in = list_entry(curr, struct var_bind, link);
    vb_in_cnt++;

    /* Decode vb_in value first */
    tag(&ret_oid.var) = vb_in->value_type;
    length(&ret_oid.var) = ber_value_dec(vb_in->value, vb_in->value_len, tag(&ret_oid.var), value(&ret_oid.var));

    /* Search at the input oid */
    mib_get(sdg, vb_in, &ret_oid);

    val_len = ber_value_enc_try(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var));
    vb_out = xmalloc(sizeof(*vb_out) + val_len);
    vb_out->oid = ret_oid.oid;
    vb_out->oid_len = ret_oid.id_len;
    vb_out->value_type = tag(&ret_oid.var);
    vb_out->value_len = ber_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);

    /* Error status */
    if (ret_oid.err_stat) {
      if (!sdg->pdu_hdr.err_stat) {
        /* Mark the first error varbind */
        sdg->pdu_hdr.err_stat = ret_oid.err_stat;
        sdg->pdu_hdr.err_idx = vb_in_cnt;
      }
    }

    /* OID length encoding */
    oid_len = ber_value_enc_try(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID);
    len_len = ber_length_enc_try(oid_len);
    vb_out->vb_len = tag_len + len_len + oid_len;

    /* Value length encoding */
    len_len = ber_length_enc_try(vb_out->value_len);
    vb_out->vb_len += tag_len + len_len + vb_out->value_len;

    /* Varbind length encoding */
    len_len = ber_length_enc_try(vb_out->vb_len);
    sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

    /* Add into list. */
    list_add_tail(&vb_out->link, &sdg->vb_out_list);
    sdg->vb_out_cnt++;
  }

  snmp_response(sdg);
}

static void
mib_getnext(struct snmp_datagram *sdg, struct var_bind *vb_in, struct oid_search_res *ret_oid)
{
  struct mib_view *view = NULL;
  struct mib_community *community = NULL;
  struct mib_user *user = NULL;

  /* Access control */
  if (sdg->version >= 3) {
    user = mib_user_search(sdg->user_name);
    if (user == NULL) {
      ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
    }
  } else {
    community = mib_community_search(sdg->context_name);
    if (community == NULL) {
      ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
    }
  }

  /* Traverse all availble views according to community or user */
  for (; ;) {
    if (sdg->version >= 3) {
        view = mib_user_next_view(user, MIB_ACES_READ, view);
    } else {
        view = mib_community_next_view(community, MIB_ACES_READ, view);
    }

    /* End of mib view */
    if (view == NULL) {
      /* Duplicate original oid when result not found */
      ret_oid->oid = oid_dup(vb_in->oid, vb_in->oid_len);
      ret_oid->id_len = vb_in->oid_len;
      return;
    }

    mib_tree_search_next(view, vb_in->oid, vb_in->oid_len, ret_oid);
    if (tag(&ret_oid->var) != ASN1_TAG_END_OF_MIB_VIEW) {
      /* Gotcha */
      break;
    }
    /* Free temporary result */
    free(ret_oid->oid);
  }
}

void
snmp_getnext(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len, val_len;
  uint32_t vb_in_cnt = 0;
  const uint32_t tag_len = 1;

  memset(&ret_oid, 0, sizeof(ret_oid));
  ret_oid.request = MIB_REQ_GETNEXT;

  list_for_each_safe(curr, next, &sdg->vb_in_list) {
    vb_in = list_entry(curr, struct var_bind, link);
    vb_in_cnt++;

    /* Decode vb_in value first */
    tag(&ret_oid.var) = vb_in->value_type;
    length(&ret_oid.var) = ber_value_dec(vb_in->value, vb_in->value_len, tag(&ret_oid.var), value(&ret_oid.var));

    /* Search at the next input oid */
    mib_getnext(sdg, vb_in, &ret_oid);

    val_len = ber_value_enc_try(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var));
    vb_out = xmalloc(sizeof(*vb_out) + val_len);
    vb_out->oid = ret_oid.oid;
    vb_out->oid_len = ret_oid.id_len;
    vb_out->value_type = tag(&ret_oid.var);
    vb_out->value_len = ber_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);

    /* Error status */
    if (ret_oid.err_stat) {
      if (!sdg->pdu_hdr.err_stat) {
        /* Report the first error varbind */
        sdg->pdu_hdr.err_stat = ret_oid.err_stat;
        sdg->pdu_hdr.err_idx = vb_in_cnt;
      }
    }

    /* OID length encoding */
    oid_len = ber_value_enc_try(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID);
    len_len = ber_length_enc_try(oid_len);
    vb_out->vb_len = tag_len + len_len + oid_len;

    /* Value length encoding */
    len_len = ber_length_enc_try(vb_out->value_len);
    vb_out->vb_len += tag_len + len_len + vb_out->value_len;

    /* Varbind length encoding */
    len_len = ber_length_enc_try(vb_out->vb_len);
    sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

    /* Add into list. */
    list_add_tail(&vb_out->link, &sdg->vb_out_list);
    sdg->vb_out_cnt++;
  }

  snmp_response(sdg);
}

static void
mib_set(struct snmp_datagram *sdg, struct var_bind *vb_in, struct oid_search_res *ret_oid)
{
  struct mib_view *view = NULL;
  struct mib_community *community = NULL;
  struct mib_user *user = NULL;

  /* Access control */
  if (sdg->version >= 3) {
    user = mib_user_search(sdg->user_name);
    if (user != NULL) {
      /* Check mib write views */
      if (!mib_user_view_cover(user, MIB_ACES_WRITE, vb_in->oid, vb_in->oid_len)) {
        ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
        user = NULL;
      }
    } else {
      ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
    }
  } else {
    community = mib_community_search(sdg->context_name);
    if (community != NULL) {
      /* Check mib write views */
      if (!mib_community_view_cover(community, MIB_ACES_WRITE, vb_in->oid, vb_in->oid_len)) {
        ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
        community = NULL;
      }
    } else {
      ret_oid->err_stat = SNMP_ERR_STAT_NO_ACCESS;
    }
  }

  /* Traverse all availble views according to community or user */
  for (; ;) {
    if (sdg->version >= 3) {
        view = mib_user_next_view(user, MIB_ACES_WRITE, view);
    } else {
        view = mib_community_next_view(community, MIB_ACES_WRITE, view);
    }

    /* End of mib view */
    if (view == NULL) {
      /* Duplicate original oid when result not found */
      ret_oid->oid = oid_dup(vb_in->oid, vb_in->oid_len);
      ret_oid->id_len = vb_in->oid_len;
      return;
    }

    mib_tree_search(view, vb_in->oid, vb_in->oid_len, ret_oid);
    if ((!ret_oid->err_stat && MIB_TAG_VALID(tag(&ret_oid->var))) || oid_cmp(vb_in->oid, vb_in->oid_len, view->oid, view->id_len) < 0) {
      /* Gotcha or given oid ahead of all views */
      return;
    }
    /* Free temporary result */
    free(ret_oid->oid);
  }
}

/* SET request function */
void
snmp_set(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len, val_len;
  uint32_t vb_in_cnt = 0;
  const uint32_t tag_len = 1;

  memset(&ret_oid, 0, sizeof(ret_oid));
  ret_oid.request = MIB_REQ_SET;

  list_for_each_safe(curr, next, &sdg->vb_in_list) {
    vb_in = list_entry(curr, struct var_bind, link);
    vb_in_cnt++;

    /* Decode vb_in value first */
    tag(&ret_oid.var) = vb_in->value_type;
    length(&ret_oid.var) = ber_value_dec(vb_in->value, vb_in->value_len, tag(&ret_oid.var), value(&ret_oid.var));

    /* Search at the input oid and set it */
    mib_set(sdg, vb_in, &ret_oid);

    val_len = ber_value_enc_try(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var));
    vb_out = xmalloc(sizeof(*vb_out) + val_len);
    vb_out->oid = ret_oid.oid;
    vb_out->oid_len = ret_oid.id_len;
    vb_out->value_type = vb_in->value_type;
    vb_out->value_len = ber_value_enc(value(&ret_oid.var), val_len, tag(&ret_oid.var), vb_out->value);

    /* Invalid tags convert to error status for snmpset */
    if (!ret_oid.err_stat && !MIB_TAG_VALID(tag(&ret_oid.var))) {
      ret_oid.err_stat = SNMP_ERR_STAT_NOT_WRITABLE;
    }

    /* Error status */
    if (ret_oid.err_stat) {
      if (!sdg->pdu_hdr.err_stat) {
        /* Report the first error varbind */
        sdg->pdu_hdr.err_stat = ret_oid.err_stat;
        sdg->pdu_hdr.err_idx = vb_in_cnt;
      }
    }

    /* OID length encoding */
    oid_len = ber_value_enc_try(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID);
    len_len = ber_length_enc_try(oid_len);
    vb_out->vb_len = tag_len + len_len + oid_len;

    /* Value length encoding */
    len_len = ber_length_enc_try(vb_out->value_len);
    vb_out->vb_len += tag_len + len_len + vb_out->value_len;

    /* Varbind length encoding */
    len_len = ber_length_enc_try(vb_out->vb_len);
    sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

    /* Add into list. */
    list_add_tail(&vb_out->link, &sdg->vb_out_list);
    sdg->vb_out_cnt++;
  }

  snmp_response(sdg);
}

void
snmp_bulkget(struct snmp_datagram *sdg)
{
  struct list_head *curr, *next;
  struct var_bind *vb_in, *vb_out;
  struct oid_search_res ret_oid;
  uint32_t oid_len, len_len, val_len;
  uint32_t vb_in_cnt = 0;
  uint32_t repeat;
  const uint32_t tag_len = 1;

  memset(&ret_oid, 0, sizeof(ret_oid));
  ret_oid.request = MIB_REQ_GETNEXT;
  repeat = sdg->pdu_hdr.err_idx;
  sdg->pdu_hdr.err_idx = 0;

  while (repeat-- > 0) {
    list_for_each_safe(curr, next, &sdg->vb_in_list) {
      vb_in = list_entry(curr, struct var_bind, link);
      vb_in_cnt++;

      /* Decode vb_in value first */
      tag(&ret_oid.var) = vb_in->value_type;
      length(&ret_oid.var) = ber_value_dec(vb_in->value, vb_in->value_len, tag(&ret_oid.var), value(&ret_oid.var));

      /* Search at the next input oid */
      mib_getnext(sdg, vb_in, &ret_oid);

      /* Return oid for the next query. */
      free(vb_in->oid);
      vb_in->oid = oid_dup(ret_oid.oid, ret_oid.id_len);
      vb_in->oid_len = ret_oid.id_len;

      val_len = ber_value_enc_try(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var));
      vb_out = xmalloc(sizeof(*vb_out) + val_len);
      vb_out->oid = ret_oid.oid;
      vb_out->oid_len = ret_oid.id_len;
      vb_out->value_type = tag(&ret_oid.var);
      vb_out->value_len = ber_value_enc(value(&ret_oid.var), length(&ret_oid.var), tag(&ret_oid.var), vb_out->value);

      /* Error status */
      if (ret_oid.err_stat) {
        if (!sdg->pdu_hdr.err_stat) {
          /* Report the first error varbind */
          sdg->pdu_hdr.err_stat = ret_oid.err_stat;
          sdg->pdu_hdr.err_idx = vb_in_cnt;
        }
      }

      /* OID length encoding */
      oid_len = ber_value_enc_try(vb_out->oid, vb_out->oid_len, ASN1_TAG_OBJID);
      len_len = ber_length_enc_try(oid_len);
      vb_out->vb_len = tag_len + len_len + oid_len;

      /* Value length encoding */
      len_len = ber_length_enc_try(vb_out->value_len);
      vb_out->vb_len += tag_len + len_len + vb_out->value_len;

      /* Varbind length encoding */
      len_len = ber_length_enc_try(vb_out->vb_len);
      sdg->vb_list_len += tag_len + len_len + vb_out->vb_len;

      /* Add into list. */
      list_add_tail(&vb_out->link, &sdg->vb_out_list);
      sdg->vb_out_cnt++;
    }
  }

  snmp_response(sdg);
}
