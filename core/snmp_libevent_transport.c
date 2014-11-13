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

static TRANSPORT_RECEIVER le_receiver;

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
udp_write_cb(const int sock, short int which, void *arg)
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
      snmp_send_event = event_new(event_base, sock, EV_WRITE, udp_write_cb, NULL);
      event_add(snmp_send_event, NULL);
    }
  }
}

static void
udp_read_cb(const int sock, short int which, void *arg)
{
  socklen_t server_sz = sizeof(struct sockaddr_in);
  int len;
  uint8_t * buf;

  buf = malloc(TRANS_BUF_SIZ);
  if (buf == NULL) {
    perror("malloc()");
    exit(EXIT_FAILURE);
  }

  client_sin = malloc(server_sz);
  if (client_sin == NULL) {
    perror("malloc()");
    exit(EXIT_FAILURE);
  }

  /* Receive UDP data, store the address of the sender in client_sin */
  len = recvfrom(sock, buf, TRANS_BUF_SIZ, 0, (struct sockaddr *)client_sin, &server_sz);
  if (len == -1) {
    perror("recvfrom()");
    event_loopbreak();
  }

  if (le_receiver != NULL) {
    le_receiver(buf, len);
  }
}

static void
snmp_le_transport_send(uint8_t * buf, int len)
{
  struct send_data_entry * entry;

  entry = malloc(sizeof(struct send_data_entry));
  if (entry == NULL) {
    perror("malloc()");
    exit(EXIT_FAILURE);
  }

  entry->buf = buf;
  entry->len = len;
  entry->client_sin = client_sin;

  /* Insert to tail */
  TAILQ_INSERT_TAIL(&send_queue_head, entry, entries);

  /* Send event comes with UPD packet */
  snmp_send_event = event_new(event_base, sock, EV_WRITE, udp_write_cb, NULL);
  event_add(snmp_send_event, NULL);
}

static void
snmp_le_transport_running(void)
{
  /* Initialize libevent */
  event_base = event_base_new();

  /* Receive event can be added only once. */
  snmp_recv_event = event_new(event_base, sock, EV_READ | EV_PERSIST, udp_read_cb, NULL);
  event_add(snmp_recv_event, NULL);

  /* Enter the event loop; does not return. */
  event_base_dispatch(event_base);

  close(sock);
}

static void
snmp_le_transport_init(int port, TRANSPORT_RECEIVER recv_cb)
{
  struct sockaddr_in sin;

  le_receiver = recv_cb;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("usock");
    exit(EXIT_FAILURE);
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  /* Initialize send queue */
  TAILQ_INIT(&send_queue_head);

  if (bind(sock, (struct sockaddr *) &sin, sizeof(sin))) {
    perror("bind()");
    exit(EXIT_FAILURE);
  }
}

struct transport_operation snmp_le_trans_ops = {
  "snmp_libevent",
  snmp_le_transport_init,
  snmp_le_transport_running,
  snmp_le_transport_send,
};
