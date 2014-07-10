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

static lua_State *l_state;

/* Receive SNMP request datagram from transport layer */
void
snmpd_receive(uint8_t *buf, int len)
{
  snmp_recv(buf, len, l_state);
}

/* Send SNMP response datagram to transport layer */
void
snmpd_send(uint8_t *buf, int len)
{
  transport_send(buf, len);
}

/* Register mib nodes from Lua */
static int
snmpd_mib_node_reg(lua_State *L)
{
  oid_t *inst_id;
  int inst_id_len;
  const char *inst_cb;
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

  /* Get lua callback of instance node */
  inst_cb = luaL_checkstring(L, 2);
  /* Register node */
  i = mib_node_reg(inst_id, inst_id_len, inst_cb);
  /* Get returned value */
  lua_pushnumber(L, i);

  free(inst_id);

  return 1;
}

/* Unregister mib nodes from Lua */
static int
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

static int
snmpd_init(lua_State *L)
{
  int port;
  
  mib_init();

  port = luaL_checkint(L, 1);
  transport_init(port, snmpd_receive);

  lua_pushboolean(L, 1);

  return 1;  
}

static int
snmpd_run(lua_State *L)
{
  transport_running();

  lua_pushboolean(L, 1);

  return 1;  
}

static const luaL_Reg snmpd_func[] = {
  { "init", snmpd_init },
  { "run", snmpd_run },
  { "mib_node_reg", snmpd_mib_node_reg },
  { "mib_node_unreg", snmpd_mib_node_unreg },
  { NULL, NULL }
};

/* Init snmp agent from lua */
int
luaopen_lualib_core(lua_State *L)
{
  /* Store lua environment */
  l_state = L;
  /* Register snmpd_func into lua */
  luaL_register(L, "snmpd_lib", snmpd_func);

  return 1;
}
