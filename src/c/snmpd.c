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

static lua_State *L;

/* Register mib nodes from Lua */
static int
lua_mib_node_reg(lua_State *L)
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
  mib_node_reg(inst_id, inst_id_len, inst_cb);

  free(inst_id);

  return 1;
}

/* Unregister mib nodes from Lua */
static int
lua_mib_node_unreg(lua_State *L)
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

  return 1;
}

static const luaL_Reg mib_lib[] = {
  { "mib_node_reg", lua_mib_node_reg },
  { "mib_node_unreg", lua_mib_node_unreg },
  { NULL, NULL },
};

/* Receive SNMP request datagram from transport layer */
void
snmpd_receive(uint8_t *buf, int len)
{
  snmp_recv(buf, len, L);
}

/* Send SNMP response datagram to transport layer */
void
snmpd_send(uint8_t *buf, int len)
{
  transport_send(buf, len);
}

/* Calling debug.traceback() from lua */
static int
traceback(lua_State *L)
{
  if (!lua_isstring(L, 1))
    return 1;

  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }

  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }

  /* pass error message */
  lua_pushvalue(L, 1);
  /* skip this function and traceback */
  lua_pushinteger(L, 2);
  /* call debug.traceback */
  lua_call(L, 2, 1);

  fprintf(stderr, "%s\n", lua_tostring(L, -1));

  return 1;
}

static int
verify_path(const char *path)
{
	char c;

	goto inside;
	for (; ;) {
		if (!c)
			return 1;
		if (c == '/') {
inside:
			c = *path++;
			if (c != '/' && c != '.' && c != '\0')
				continue;
			return 0;
		}
		c = *path++;
	}
}

int
main(int argc, char **argv)
{
  /* Defalut port */
  int port = 161;
  const char *conf = NULL;

  while (--argc > 0) {
    ++argv;

    if (verify_path(*argv)) {
      const char *str = &((*argv)[strlen(*argv) - 4]); 
      if (!strcmp(str, ".lua"))
        conf = *argv;
    }

    if (!strcmp(*argv, "-p") || !strcmp(*argv, "--port")) {
      if (--argc == 0)
        usage("snmpd [-p|--port 161]");
      port = atoi(*(++argv));
      if (port == 0)
        perror("What's the port number on earth?");
    }
  }

  if (conf == NULL) {
    usage("snmpd init.lua");
  }

  /* Init MIB tree */
  mib_init();

  /* Open Lua environment */
  L = luaL_newstate();
  luaL_openlibs(L);

  /* Open Lua environment */
  /* Register mib_lib into lua */
  luaL_register(L, "mib_lib", mib_lib);

  lua_pushcfunction(L, traceback);

  /* Open Lua environment */
  /* Load mib-node config file */
  if (luaL_loadfile(L, conf)) {
    fprintf(stderr, "Lua do file fail: %s\n", lua_tostring(L, -1));
    exit(-1);
  } else if (lua_pcall(L, 0, 0, lua_gettop(L) - 1)) {
    exit(-1);
  }

  /* Init and run transport module */
  transport_init(port, snmpd_receive);
  transport_running();

  lua_close(L);

  return 0;
}
