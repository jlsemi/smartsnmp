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

#include "transport.h"
#include "mib.h"
#include "snmp.h"
#include "agentx.h"

lua_State *mib_lua_state;
struct smartsnmp_transport_ops *smartsnmp_trans_ops;

static const luaL_Reg snmpd_func[] = {
  { "init", snmpd_init },
  { "open", snmpd_open },
  { "run", snmpd_run },
  { "mib_node_reg", snmpd_mib_node_reg },
  { "mib_node_unreg", snmpd_mib_node_unreg },
  { NULL, NULL }
};

static const luaL_Reg agentx_func[] = {
  { "init", agentx_init },
  { "open", agentx_open },
  { "run", agentx_run },
  { "mib_node_reg", agentx_mib_node_reg },
  { "mib_node_unreg", agentx_mib_node_unreg },
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

  /* Register snmpd_func into lua */
  luaL_register(L, "snmpd_lib", snmpd_func);
  //luaL_register(L, "agentx_lib", agentx_func);

  return 1;
}
