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

#include <sys/socket.h>
#include <sys/queue.h>
#include <netinet/in.h>

#include <assert.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "transport.h"
#include "snmp.h"
#include "mib.h"
#include "util.h"

/* Receive SNMP request datagram from transport layer */
void
snmpd_receive(uint8_t *buf, int len)
{
  snmpd_recv(buf, len);
}

/* Send SNMP response datagram to transport layer */
void
snmpd_send(uint8_t *buf, int len)
{
  smartsnmp_trans_ops->send(buf, len);
}

/* Register mib nodes from Lua */
int
snmpd_mib_node_reg(lua_State *L)
{
  oid_t *inst_id;
  int inst_id_len;
  int i, inst_cb;

  /* Check if the first argument is a table. */
  luaL_checktype(L, 1, LUA_TTABLE);

  /* Get oid length */
  inst_id_len = lua_objlen(L, 1);
  /* Get oid */
  inst_id = xmalloc(inst_id_len * sizeof(oid_t));
  for (i = 0; i < inst_id_len; i++) {
    lua_rawgeti(L, 1, i + 1);
    inst_id[i] = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }

  /* Get lua callback of instance node */
  if (!lua_isfunction(L, -1)) {
    lua_pushstring(L, "Handler is not a function!");
    lua_error(L);
  }

  inst_cb = luaL_ref(L, LUA_ENVIRONINDEX);
  /* Register node */
  i = mib_node_reg(inst_id, inst_id_len, inst_cb);
  /* Get returned value */
  lua_pushnumber(L, i);

  free(inst_id);

  return 1;
}

/* Unregister mib nodes from Lua */
int
snmpd_mib_node_unreg(lua_State *L)
{
  oid_t *inst_id;
  int inst_id_len;
  int i;

  /* Check if the first argument is a table. */
  luaL_checktype(L, 1, LUA_TTABLE);

  /* Get oid length */
  inst_id_len = lua_objlen(L, 1);

  /* Get oid */
  inst_id = xmalloc(inst_id_len * sizeof(oid_t));
  for (i = 0; i < inst_id_len; i++) {
    lua_rawgeti(L, 1, i + 1);
    inst_id[i] = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }

  /* Unregister node */
  mib_node_unreg(inst_id, inst_id_len);

  free(inst_id);

  return 0;
}

int
snmpd_init(lua_State *L)
{
  int port;
  
  mib_init();

  port = luaL_checkint(L, 1);

  smartsnmp_trans_ops = &udp_trans_ops;
  smartsnmp_trans_ops->init(port, snmpd_receive);

  lua_pushboolean(L, 1);

  return 1;  
}

int
snmpd_open(lua_State *L)
{
  lua_pushboolean(L, 1);
  return 1;  
}

int
snmpd_run(lua_State *L)
{
  smartsnmp_trans_ops->running();

  lua_pushboolean(L, 1);

  return 1;  
}
