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
#include <signal.h>
#include "mib.h"
#include "snmp.h"
#include "agentx.h"
#include "protocol.h"
#include "util.h"

static struct protocol_operation *prot_ops;

void sig_int_handler(int dummy)
{
  prot_ops->close();
}

int
smartsnmp_init(lua_State *L)
{
  int ret;
  const char *protocol = luaL_checkstring(L, 1);
  int port = luaL_checkint(L, 2);

  signal(SIGINT, sig_int_handler);

  mib_init();

  if (!strcmp(protocol, "snmp")) {
    prot_ops = &snmp_prot_ops;
  } else if (!strcmp(protocol, "agentx")) {
    prot_ops = &agentx_prot_ops;
  } else {
    lua_pushboolean(L, 0);
    return 1;  
  }

  ret = prot_ops->init(port);

  if (ret < 0) {
    lua_pushboolean(L, 0);
  } else {
    lua_pushboolean(L, 1);
  }
  return 1;  
}

int
smartsnmp_open(lua_State *L)
{
  int ret = prot_ops->open();
  if (ret < 0) {
    lua_pushboolean(L, 0);
  } else {
    lua_pushboolean(L, 1);
  }

  return 1;  
}

int
smartsnmp_run(lua_State *L)
{
  prot_ops->run();
  return 0;  
}

int
smartsnmp_exit(lua_State *L)
{
  prot_ops->close();
  return 0;
}

/* Register mib nodes from Lua */
int
smartsnmp_mib_node_reg(lua_State *L)
{
  oid_t *grp_id;
  int i, grp_id_len, grp_cb;

  /* Check if the first argument is a table. */
  luaL_checktype(L, 1, LUA_TTABLE);
  /* Get oid length */
  grp_id_len = lua_objlen(L, 1);
  /* Get oid */
  grp_id = xmalloc(grp_id_len * sizeof(oid_t));
  for (i = 0; i < grp_id_len; i++) {
    lua_rawgeti(L, 1, i + 1);
    grp_id[i] = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }
  /* Get lua callback of grpance node */
  if (!lua_isfunction(L, -1)) {
    lua_pushstring(L, "Handler is not a function!");
    lua_error(L);
  }
  grp_cb = luaL_ref(L, LUA_ENVIRONINDEX);

  /* Register node */
  i = prot_ops->reg(grp_id, grp_id_len, grp_cb);
  free(grp_id);

  /* Return value */
  lua_pushnumber(L, i);
  return 1;
}

/* Unregister mib nodes from Lua */
int
smartsnmp_mib_node_unreg(lua_State *L)
{
  oid_t *grp_id;
  int i, grp_id_len;

  /* Check if the first argument is a table. */
  luaL_checktype(L, 1, LUA_TTABLE);
  /* Get oid length */
  grp_id_len = lua_objlen(L, 1);
  /* Get oid */
  grp_id = xmalloc(grp_id_len * sizeof(oid_t));
  for (i = 0; i < grp_id_len; i++) {
    lua_rawgeti(L, 1, i + 1);
    grp_id[i] = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }

  /* Unregister group node */
  i = prot_ops->unreg(grp_id, grp_id_len);
  free(grp_id);

  /* Return value */
  lua_pushnumber(L, i);
  return 1;
}

/* Register community string from Lua */
int
smartsnmp_mib_community_reg(lua_State *L)
{
  oid_t *oid;
  int i, id_len, attribute;
  const char *community;

  /* Check if the first argument is a table. */
  luaL_checktype(L, 1, LUA_TTABLE);
  /* Get oid length */
  id_len = lua_objlen(L, 1);
  /* Get oid */
  oid = xmalloc(id_len * sizeof(oid_t));
  for (i = 0; i < id_len; i++) {
    lua_rawgeti(L, 1, i + 1);
    oid[i] = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }

  /* Community string and RW attribute */
  community = luaL_checkstring(L, 2);
  attribute = luaL_checkint(L, 3);

  /* Register community string */
  mib_community_reg(oid, id_len, community, attribute);
  free(oid);

  return 0;
}

/* Unregister mib community string from Lua */
int
smartsnmp_mib_community_unreg(lua_State *L)
{
  /* Unregister community string */
  const char *community = luaL_checkstring(L, 1);
  int attribute = luaL_checkint(L, 2);
  mib_community_unreg(community, attribute);

  return 0;
}

/* Register mib user from Lua */
int
smartsnmp_mib_user_reg(lua_State *L)
{
  oid_t *oid;
  int i, id_len, attribute;
  const char *user;

  /* Check if the first argument is a table. */
  luaL_checktype(L, 1, LUA_TTABLE);
  /* Get oid length */
  id_len = lua_objlen(L, 1);
  /* Get oid */
  oid = xmalloc(id_len * sizeof(oid_t));
  for (i = 0; i < id_len; i++) {
    lua_rawgeti(L, 1, i + 1);
    oid[i] = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }

  /* User string and RW attribute */
  user = luaL_checkstring(L, 2);
  attribute = luaL_checkint(L, 3);

  /* Register user string */
  mib_user_reg(oid, id_len, user, attribute);
  free(oid);

  return 0;
}

/* Unregister mib user string from Lua */
int
smartsnmp_mib_user_unreg(lua_State *L)
{
  /* Unregister user string */
  const char *user = luaL_checkstring(L, 1);
  int attribute = luaL_checkint(L, 2);
  mib_user_unreg(user, attribute);

  return 0;
}

static const luaL_Reg smartsnmp_func[] = {
  { "init", smartsnmp_init },
  { "open", smartsnmp_open },
  { "run", smartsnmp_run },
  { "exit", smartsnmp_exit },
  { "mib_node_reg", smartsnmp_mib_node_reg },
  { "mib_node_unreg", smartsnmp_mib_node_unreg },
  { "mib_community_reg", smartsnmp_mib_community_reg },
  { "mib_community_unreg", smartsnmp_mib_community_unreg },
  { "mib_user_reg", smartsnmp_mib_user_reg },
  { "mib_user_unreg", smartsnmp_mib_user_unreg },
  { NULL, NULL }
};

/* Init smartsnmp agent from lua */
int
luaopen_smartsnmp_core(lua_State *L)
{
  /* Store lua environment */
  mib_lua_state = L;
  lua_newtable(L);
  lua_replace(L, LUA_ENVIRONINDEX);

  /* Register smartsnmp_func into lua */
  luaL_register(L, "smartsnmp_lib", smartsnmp_func);

  return 1;
}
