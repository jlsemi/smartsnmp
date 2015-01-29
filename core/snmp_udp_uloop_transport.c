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
#include "protocol.h"
#include "libubox/uloop.h"
#include "util.h"

static struct uloop_fd server;
static struct sockaddr_in *client_sin;

struct send_data_entry {
  int len;
  uint8_t *buf;
  struct sockaddr_in *client_sin;
};

static void
server_cb(struct uloop_fd *fd, unsigned int events)
{
  socklen_t server_sz = sizeof(struct sockaddr_in);
  int len;
  uint8_t * buf;

  buf = xmalloc(TRANS_BUF_SIZ);
  client_sin = xmalloc(server_sz);

  /* Receive UDP data, store the address of the sender in client_sin */
  len = recvfrom(server.fd, buf, TRANS_BUF_SIZ, 0, (struct sockaddr *)client_sin, &server_sz);
  if (len == -1) {
    perror("recvfrom()");
    uloop_done();
  }

  /* Parse SNMP PDU in decoder */
  snmp_prot_ops.receive(buf, len);
}

/* Send snmp datagram as a UDP packet to the remote */
static void
transport_send(uint8_t *buf, int len)
{
  struct send_data_entry *entry;

  entry = xmalloc(sizeof(struct send_data_entry));
  entry->buf = buf;
  entry->len = len;
  entry->client_sin = client_sin;

  /* Send the data back to the client */
  if (sendto(server.fd, entry->buf, entry->len, 0, (struct sockaddr *)entry->client_sin, sizeof(struct sockaddr_in)) == -1) {
    perror("sendto()");
    uloop_done();
  }

  free(entry->buf);
  free(entry->client_sin);
  free(entry);
}

static void
transport_running(void)
{
  uloop_init();
  uloop_fd_add(&server, ULOOP_READ);
  uloop_run();
}

static void
transport_stop(void)
{
  uloop_end();
}

static void
transport_init(int port)
{
  struct sockaddr_in sin;

  server.cb = server_cb;
  server.fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (server.fd < 0) {
    perror("usock");
    return -1;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  if (bind(server.fd, (struct sockaddr *)&sin, sizeof(sin))) {
    perror("bind()");
    return -1;
  }

  return 0;
}

struct transport_operation snmp_trans_ops = {
  "snmp_uloop",
  transport_init,
  transport_running,
  transport_stop,
  transport_send,
};
