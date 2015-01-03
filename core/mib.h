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
#include "list.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define MIB_OBJ_UNKNOWN         0
#define MIB_OBJ_GROUP           1
#define MIB_OBJ_INSTANCE        2

/* MIB request type */
typedef enum mib_request {
  MIB_REQ_GET     = 0xA0,
  MIB_REQ_GETNEXT = 0xA1,
  MIB_RESP        = 0xA2,
  MIB_REQ_SET     = 0xA3,
  MIB_REQ_BULKGET = 0xA5,
  MIB_REQ_INF     = 0xA6,
  MIB_TRAP        = 0xA7,
  MIB_REPO        = 0xA8,
} MIB_REQ_E;

/* MIB access attribute */
typedef enum mib_aces_attr {
  MIB_ACES_READ = 1,
  MIB_ACES_WRITE
} MIB_ACES_ATTR_E;

struct oid_search_res {
  /* Return oid */
  oid_t *oid;
  uint32_t id_len;
  /* Instance oid of return */
  oid_t *inst_id;
  uint32_t inst_id_len;
  /* Instance search callback in Lua */
  int callback;
  /* Request id */
  int request;
  /* Error status */
  int err_stat;
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
  int callback;
};

struct mib_view {
  struct mib_view *next;
  const oid_t *oid;
  uint32_t id_len;
  /* head of all relevant communities */
  struct list_head communities;
  /* head of all relevant users */
  struct list_head users;
};

struct mib_community {
  struct mib_community *next;
  const char *name;
  /* head of relevant read only view */
  struct list_head ro_views;
  /* head of relevant read write view */
  struct list_head rw_views;
};

struct mib_user {
  struct mib_user *next;
  const char *name;
  /* head of relevant read only view */
  struct list_head ro_views;
  /* head of relevant read write view */
  struct list_head rw_views;
};

struct community_view {
  /* link to mib view */
  struct list_head vlink;
  /* link to mib community */
  struct list_head clink;
  struct mib_view *view;
  struct mib_community *community;
};

struct user_view {
  /* link to mib view */
  struct list_head vlink;
  /* link to mib user */
  struct list_head ulink;
  struct mib_view *view;
  struct mib_user *user;
};

extern lua_State *mib_lua_state;

oid_t *oid_dup(const oid_t *oid, uint32_t len);
oid_t *oid_cpy(oid_t *oid_dest, const oid_t *oid_src, uint32_t len);
int oid_cmp(const oid_t *src, uint32_t src_len, const oid_t *target, uint32_t tar_len);
int oid_cover(const oid_t *oid1, uint32_t len1, const oid_t *oid2, uint32_t len2);

void mib_handler_unref(int handler);
int mib_instance_search(struct oid_search_res *ret_oid);
struct mib_node *mib_tree_search(struct mib_view *view, const oid_t *oid, uint32_t id_len, struct oid_search_res *ret_oid);
void mib_tree_search_next(struct mib_view *view, const oid_t *oid, uint32_t id_len, struct oid_search_res *ret_oid);

int mib_node_reg(const oid_t *oid, uint32_t id_len, int callback);
void mib_node_unreg(const oid_t *oid, uint32_t id_len);
void mib_community_reg(const oid_t *oid, uint32_t len, const char *community, MIB_ACES_ATTR_E attribute);
void mib_community_unreg(const char *community, MIB_ACES_ATTR_E attribute);
void mib_user_reg(const oid_t *oid, uint32_t len, const char *community, MIB_ACES_ATTR_E attribute);
void mib_user_unreg(const char *user, MIB_ACES_ATTR_E attribute);
struct mib_community *mib_community_search(const char *community);
struct mib_view *mib_community_next_view(struct mib_community *c, MIB_ACES_ATTR_E attribute, struct mib_view *v);
int mib_community_view_cover(struct mib_community *c, MIB_ACES_ATTR_E attribute, const oid_t *oid, uint32_t id_len);
struct mib_user *mib_user_search(const char *user);
struct mib_view *mib_user_next_view(struct mib_user *u, MIB_ACES_ATTR_E attribute, struct mib_view *v);
int mib_user_view_cover(struct mib_user *u, MIB_ACES_ATTR_E attribute, const oid_t *oid, uint32_t id_len);

void mib_init(void);

#endif /* _MIB_H_ */
