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
#include <netinet/in.h>

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "agentx.h"
#include "transport.h"
#include "protocol.h"
#include "ev_loop.h"
#include "util.h"

struct agentx_data_entry {
  int sock;
  uint8_t *buf;
  int len;
};

static struct agentx_data_entry agentx_entry;

static void
agentx_write_handler(int sock, unsigned char flag, void *ud)
{
  struct agentx_data_entry *entry = ud;

  if (send(sock, entry->buf, entry->len, 0) == -1) {
    perror("send()");
    snmp_event_done();
  }

  free(entry->buf);

  snmp_event_remove(sock, flag);
}

static void
agentx_read_handler(int sock, unsigned char flag, void *ud)
{
  uint8_t *buf;
  int len;

  buf = xmalloc(TRANS_BUF_SIZ);

  /* Receive agentx PDU */
  len = recv(sock, buf, TRANS_BUF_SIZ - 1, 0);
  if (len == -1) {
    perror("recv()");
    snmp_event_done();
  }

  /* Parse agentX PDU in decoder */
  agentx_prot_ops.receive(buf, len);
}

/* Send angentX PDU to the remote */
static void
transport_send(uint8_t *buf, int len)
{
  agentx_entry.buf = buf;
  agentx_entry.len = len;
  snmp_event_add(agentx_entry.sock, SNMP_EV_WRITE, agentx_write_handler, &agentx_entry);
}

static void
transport_running(void)
{
  snmp_event_init();
  snmp_event_add(agentx_entry.sock, SNMP_EV_READ, agentx_read_handler, NULL);
  snmp_event_run();
}

static void
transport_stop(void)
{
  snmp_event_done();
  close(agentx_entry.sock);
}

static int
transport_init(int port)
{
  struct sockaddr_in sin;

  agentx_datagram.sock = agentx_entry.sock = socket(AF_INET, SOCK_STREAM, 0);
  if (agentx_entry.sock < 0) {
    perror("usock");
    return -1;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  sin.sin_port = htons(port);

  if (connect(agentx_entry.sock, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
    perror("connect()");
    close(agentx_datagram.sock);
    return -1;
  }

  return 0;
}

struct transport_operation agentx_trans_ops = {
  "agentx_tcp",
  transport_init,
  transport_running,
  transport_stop,
  transport_send,
};
