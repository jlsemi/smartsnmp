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

#ifndef _MIB_H_
#define _MIB_H_

#include "asn1.h"

#define MIB_OBJ_UNKNOWN         0
#define MIB_OBJ_GROUP           1
#define MIB_OBJ_INSTANCE        2

#define INTERNET_PREFIX_LENGTH  4
#define NBL_STACK_SIZE          (MIB_OID_MAX_LEN - INTERNET_PREFIX_LENGTH + 1)

#define OID_ARRAY_SIZE(arr)     (sizeof(arr) / sizeof(arr[0]))

struct oid_search_res {
  /* Return oid */
  oid_t *oid;
  uint32_t id_len;
  /* Instance oid of return */
  oid_t *inst_id;
  uint32_t inst_id_len;
  /* Instance search callback in Lua */
  const char *callback;
  /* Request id */
  uint8_t request;
  /* Search return status */
  uint8_t exist_state;
  /* Search return value */
  Variable var;
};

struct mib_node {
  uint8_t type;
};

struct mib_group_node {
  uint8_t type;
  uint16_t sub_id_cap;
  uint16_t sub_id_cnt;
  oid_t *sub_id;
  void **sub_ptr;
};

struct mib_instance_node {
  uint8_t type;
  const char *callback;
};

oid_t * oid_dup(const oid_t *oid, uint32_t len);
oid_t *oid_cpy(oid_t *oid_dest, const oid_t *oid_src, uint32_t len);
int oid_cmp(const oid_t *src, uint32_t src_len, const oid_t *target, uint32_t tar_len);

int mib_instance_search(struct oid_search_res *ret_oid);
struct mib_node *mib_tree_search(const oid_t *oid, uint32_t id_len, struct oid_search_res *ret_oid);
struct mib_node *mib_tree_search_next(const oid_t *oid, uint32_t id_len, struct oid_search_res *ret_oid);

int mib_node_reg(const oid_t *oid, uint32_t id_len, const char *callback);
void mib_node_unreg(const oid_t *oid, uint32_t id_len);

void mib_init(void);

#endif /* _MIB_H_ */
