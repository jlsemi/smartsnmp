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
#include "ev_loop.h"

#define BUF_SIZE     (65536)

struct snmp_data_entry {
  int sock;
  uint8_t *buf;
  int len;
  struct sockaddr_in *client_sin;
};

static struct snmp_data_entry snmp_entry;
static TRANSPORT_RECEIVER snmp_receiver;

static void
snmp_write_handler(int sock, unsigned char flag, void *ud)
{
  struct snmp_data_entry *entry = ud;

  if (sendto(sock, entry->buf, entry->len, 0, (struct sockaddr *)entry->client_sin, sizeof(struct sockaddr_in)) == -1) {
    perror("sendto()");
    snmp_event_done();
  }

  free(entry->buf);
  free(entry->client_sin);

  snmp_event_remove(sock, flag);
}


/* Send snmp datagram as a UDP packet to the remote */
  void
transport_send(uint8_t *buf, int len)
{
  snmp_entry.buf = buf;
  snmp_entry.len = len;
  snmp_event_add(snmp_entry.sock, SNMP_EV_WRITE, snmp_write_handler, &snmp_entry);
}

static void
snmp_read_handler(int sock, unsigned char flag, void *ud)
{
  socklen_t server_sz = sizeof(struct sockaddr_in);
  int len;
  uint8_t *buf;

  buf = malloc(BUF_SIZE);
  if (buf == NULL) {
    perror("malloc()");
    exit(EXIT_FAILURE);
  }

  snmp_entry.client_sin = malloc(server_sz);
  if (snmp_entry.client_sin == NULL) {
    perror("malloc()");
    exit(EXIT_FAILURE);
  }

  /* Receive UDP data, store the address of the sender in client_sin */
  len = recvfrom(sock, buf, BUF_SIZE - 1, 0, (struct sockaddr *)snmp_entry.client_sin, &server_sz);
  if (len == -1) {
    perror("recvfrom()");
    snmp_event_done();
  }

  if (snmp_receiver != NULL) {
    snmp_receiver(buf, len);
  }
}

void
transport_running(void)
{
  snmp_event_init();
  snmp_event_add(snmp_entry.sock, SNMP_EV_READ, snmp_read_handler, NULL);
  snmp_event_run();
}

void
transport_init(int port, TRANSPORT_RECEIVER recv_cb)
{
  struct sockaddr_in sin;

  snmp_receiver = recv_cb;

  snmp_entry.sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (snmp_entry.sock < 0) {
    perror("usock");
    exit(EXIT_FAILURE);
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(port);

  if (bind(snmp_entry.sock, (struct sockaddr *)&sin, sizeof(sin))) {
    perror("bind()");
    exit(EXIT_FAILURE);
  }
}
