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

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <sys/queue.h>
#include <event.h>

#include "transport.h"
#include "protocol.h"
#include "util.h"

static struct event_base *event_base;
static struct event *snmp_recv_event;
static struct event *snmp_send_event;
static int sock;
struct sockaddr_in *client_sin;

struct send_data_entry {
  int len;
  uint8_t * buf;
  struct sockaddr_in * client_sin;
  TAILQ_ENTRY(send_data_entry) entries;
};

TAILQ_HEAD(, send_data_entry) send_queue_head;

static void
snmp_write_cb(const int sock, short int which, void *arg)
{
  struct send_data_entry * entry;

  if (!TAILQ_EMPTY(&send_queue_head)) {
    entry = TAILQ_FIRST(&send_queue_head);

    /* Send the data back to the client */
    if (sendto(sock, entry->buf, entry->len, 0, (struct sockaddr *) entry->client_sin, sizeof(struct sockaddr_in)) == -1) {
      perror("sendto()");
      event_loopbreak();
    }
    TAILQ_REMOVE(&send_queue_head, entry, entries);

    free(entry->buf);
    free(entry->client_sin);
    free(entry);

    /* Free send event */
    event_free(snmp_send_event);

    /* if there is other data to be sent, register another EV_WRITE event */
    if (!TAILQ_EMPTY(&send_queue_head)) {
      snmp_send_event = event_new(event_base, sock, EV_WRITE, snmp_write_cb, NULL);
      event_add(snmp_send_event, NULL);
    }
  }
}

static void
snmp_read_cb(const int sock, short int which, void *arg)
{
  socklen_t server_sz = sizeof(struct sockaddr_in);
  int len;
  uint8_t * buf;

  buf = xmalloc(TRANS_BUF_SIZ);
  client_sin = xmalloc(server_sz);

  /* Receive UDP data, store the address of the sender in client_sin */
  len = recvfrom(sock, buf, TRANS_BUF_SIZ, 0, (struct sockaddr *)client_sin, &server_sz);
  if (len == -1) {
    perror("recvfrom()");
    event_loopbreak();
  }

  /* Parse SNMP PDU in decoder */
  snmp_prot_ops.receive(buf, len);
}

static void
transport_send(uint8_t * buf, int len)
{
  struct send_data_entry * entry;

  entry = xmalloc(sizeof(struct send_data_entry));
  entry->buf = buf;
  entry->len = len;
  entry->client_sin = client_sin;

  /* Insert to tail */
  TAILQ_INSERT_TAIL(&send_queue_head, entry, entries);

  /* Send event comes with UPD packet */
  snmp_send_event = event_new(event_base, sock, EV_WRITE, snmp_write_cb, NULL);
  event_add(snmp_send_event, NULL);
}

static void
transport_running(void)
{
  /* Initialize libevent */
  event_base = event_base_new();

  /* Receive event can be added only once. */
  snmp_recv_event = event_new(event_base, sock, EV_READ | EV_PERSIST, snmp_read_cb, NULL);
  event_add(snmp_recv_event, NULL);

  /* Enter the event loop; does not return. */
  event_base_dispatch(event_base);

  event_base_free(event_base);

  close(sock);
}

static void
transport_stop(void)
{
  event_base_loopexit(event_base, 0);
}

static int
transport_init(int port)
{
  struct sockaddr_in sin;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("usock");
    return -1;
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  /* Initialize send queue */
  TAILQ_INIT(&send_queue_head);

  if (bind(sock, (struct sockaddr *) &sin, sizeof(sin))) {
    perror("bind()");
    close(sock);
    return -1;
  }

  return 0;
}

struct transport_operation snmp_trans_ops = {
  "snmp_libevent",
  transport_init,
  transport_running,
  transport_stop,
  transport_send,
};
