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
#include "string.h"
#include "util.h"

static const char *protocol;

int
smartsnmp_init(lua_State *L)
{
  int port;
  
  protocol = luaL_checkstring(L, 1);
  port = luaL_checkint(L, 2);

  if (!strcmp(protocol, "snmp")) {
    snmpd_init(port);
  } else if (!strcmp(protocol, "agentx")) {
    agentx_init(port);
  }

  lua_pushboolean(L, 1);

  return 1;  
}

int
smartsnmp_open(lua_State *L)
{
  int ret = -1;

  if (!strcmp(protocol, "snmp")) {
    ret = snmpd_open();
  } else if (!strcmp(protocol, "agentx")) {
    ret = agentx_open();
  }
  
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
  if (!strcmp(protocol, "snmp")) {
    snmpd_run();
  } else if (!strcmp(protocol, "agentx")) {
    agentx_run();
  }

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
  if (!strcmp(protocol, "snmp")) {
    i = snmpd_mib_node_reg(grp_id, grp_id_len, grp_cb);
  } else if (!strcmp(protocol, "agentx")) {
    i = agentx_mib_node_reg(grp_id, grp_id_len, grp_cb);
  }
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
  if (!strcmp(protocol, "snmp")) {
    i = snmpd_mib_node_unreg(grp_id, grp_id_len);
  } else if (!strcmp(protocol, "agentx")) {
    i = agentx_mib_node_unreg(grp_id, grp_id_len);
  }
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
