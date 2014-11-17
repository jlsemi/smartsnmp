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

#include "mib.h"
#include "snmp.h"
#include "agentx.h"
#include "protocol.h"
#include "string.h"
#include "util.h"

static struct protocol_operation *prot_ops;

int
smartsnmp_init(lua_State *L)
{
  const char *protocol = luaL_checkstring(L, 1);
  int port = luaL_checkint(L, 2);

  mib_init();

  if (!strcmp(protocol, "snmp")) {
    prot_ops = &snmp_prot_ops;
  } else if (!strcmp(protocol, "agentx")) {
    prot_ops = &agentx_prot_ops;
  } else {
    lua_pushboolean(L, 0);
    return 1;  
  }

  prot_ops->init(port);

  lua_pushboolean(L, 1);
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
  lua_pushboolean(L, 1);
  return 1;  
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
  if (i < 0) {
    lua_pushboolean(L, 0);
  } else {
    lua_pushboolean(L, 1);
  }

  return 1;
}

static const luaL_Reg smartsnmp_func[] = {
  { "init", smartsnmp_init },
  { "open", smartsnmp_open },
  { "run", smartsnmp_run },
  { "mib_node_reg", smartsnmp_mib_node_reg },
  { "mib_node_unreg", smartsnmp_mib_node_unreg },
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
