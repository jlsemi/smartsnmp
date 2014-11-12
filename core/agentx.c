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
#include "agentx.h"
#include "mib.h"
#include "util.h"

/* Receive agentx request datagram from transport layer */
void
agentx_receive(uint8_t *buf, int len)
{
  agentx_recv(buf, len);
}

/* Send agentx response datagram to transport layer */
void
agentx_send(uint8_t *buf, int len)
{
  smartsnmp_trans_ops->send(buf, len);
}

int
agentx_open(lua_State *L)
{
  struct x_pdu_buf x_pdu;
  oid_t open_oid[] = { 1, 3, 6, 1, 4, 1, 8072, 3, 2, 10 };
  const char *descr = "SmartSNMP AgentX sub-agent";

  x_pdu = agentx_open_pdu(&agentx_datagram, open_oid, OID_ARRAY_SIZE(open_oid), descr, strlen(descr));

  /* Send open PDU as TCP packet */
  if (send(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    lua_pushstring(L, "Send open PDU failure!");
    lua_error(L);
  }

  x_pdu.len = TRANS_BUF_SIZ;
  x_pdu.buf = xrealloc(x_pdu.buf, x_pdu.len);

  /* Receive TCP data */
  x_pdu.len = recv(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0);
  if (x_pdu.len == -1) {
    lua_pushstring(L, "Receive open PDU response failure!");
    lua_error(L);
  }

  /* Testify open PDU response */
  if (agentx_recv(x_pdu.buf, x_pdu.len) != AGENTX_ERR_OK) {
    lua_pushstring(L, "Open response error!");
    lua_error(L);
  }

  lua_pushboolean(L, 1);

  return 1;
}

/* Register mib nodes from Lua */
int
agentx_mib_node_reg(lua_State *L)
{
  oid_t *inst_id;
  int inst_id_len;
  int i, inst_cb;
  struct x_pdu_buf x_pdu;

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

  /* Check oid prefix */
  if (inst_id_len < 4 || inst_id[0] != 1 || inst_id[1] != 3 || inst_id[2] != 6 || inst_id[3] != 1) {
    lua_pushstring(L, "Oid prefix must be .1.3.6.1!");
    lua_error(L);
  }

  x_pdu = agentx_register_pdu(&agentx_datagram, inst_id, inst_id_len, NULL, 0, 0, 127, 0, 0);

  /* Send register PDU as TCP packet */
  if (send(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    lua_pushstring(L, "Send register PDU failure!");
    lua_error(L);
  }

  x_pdu.len = TRANS_BUF_SIZ;
  x_pdu.buf = xrealloc(x_pdu.buf, x_pdu.len);

  /* Receive TCP data */
  x_pdu.len = recv(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0);
  if (x_pdu.len == -1) {
    lua_pushstring(L, "Receive register PDU response failure!");
    lua_error(L);
  }

  /* Testify register PDU response */
  if (agentx_recv(x_pdu.buf, x_pdu.len) != AGENTX_ERR_OK) {
    lua_pushstring(L, "Rigister response error!");
    lua_error(L);
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
  /* Free registered oid */
  free(inst_id);

  return 1;
}

/* Unregister mib nodes from Lua */
int
agentx_mib_node_unreg(lua_State *L)
{
  oid_t *inst_id;
  int i, inst_id_len;
  struct x_pdu_buf x_pdu;

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

  /* Check oid prefix */
  if (inst_id_len < 4 || inst_id[0] != 1 || inst_id[1] != 3 || inst_id[2] != 6 || inst_id[3] != 1) {
    lua_pushstring(L, "Oid prefix must be .1.3.6.1!");
    lua_error(L);
  }

  x_pdu = agentx_unregister_pdu(&agentx_datagram, inst_id, inst_id_len, NULL, 0, 0, 127, 0, 0);

  /* Send register PDU as TCP packet */
  if (send(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    lua_pushstring(L, "Send unregister PDU failure!");
    lua_error(L);
  }

  x_pdu.len = TRANS_BUF_SIZ;
  x_pdu.buf = xrealloc(x_pdu.buf, x_pdu.len);

  /* Receive TCP data */
  x_pdu.len = recv(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0);
  if (x_pdu.len == -1) {
    lua_pushstring(L, "Receive unregister PDU response failure!");
    lua_error(L);
  }

  /* Testify register PDU response */
  if (agentx_recv(x_pdu.buf, x_pdu.len) != AGENTX_ERR_OK) {
    lua_pushstring(L, "Unregister response error!");
    lua_error(L);
  }

  /* Unregister node */
  mib_node_unreg(inst_id, inst_id_len);
  /* Free unregistered oid */
  free(inst_id);

  return 0;
}

int
agentx_init(lua_State *L)
{
  int port;

  mib_init();
  port = luaL_checkint(L, 1);
  smartsnmp_trans_ops = &tcp_trans_ops;
  smartsnmp_trans_ops->init(port, agentx_receive);
  lua_pushboolean(L, 1);

  return 1;  
}

int
agentx_run(lua_State *L)
{
  smartsnmp_trans_ops->running();
  lua_pushboolean(L, 1);
  return 1;
}
