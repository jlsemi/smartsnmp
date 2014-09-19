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

#include <stdio.h>
#include "ev_loop.h"

#define SNMP_MAX_EVENTS  5

struct snmp_event {
  int fd;
  transport_handler rcb;
  transport_handler wcb;
  void *rud;
  void *wud;
  unsigned char flag;
  unsigned char read;
  unsigned char write;
};

struct snmp_event_loop {
  int start;
  int ev_no;
  int max_fd;
  struct snmp_event event[SNMP_MAX_EVENTS];
};

static struct snmp_event_loop ev_loop;

#ifdef USE_EPOLL
#include "ev_epoll.h"
#else
    #ifdef USE_KQUEUE
    #include "ev_kqueue.h"
    #else
    #include "ev_select.h"
    #endif
#endif

void
snmp_event_init(void)
{
  int i;
  for (i = 0; i < SNMP_MAX_EVENTS; i++) {
    struct snmp_event *event = &ev_loop.event[i];
    event->fd = -1;
    event->flag = SNMP_EV_NONE;
    event->read = 0;
    event->write = 0;
  }
  ev_loop.start = 1;
  ev_loop.ev_no = 0;
  ev_loop.max_fd = -1;
  __ev_init();
}

void
snmp_event_done(void)
{
  int i;
  for (i = 0; i < SNMP_MAX_EVENTS; i++) {
    struct snmp_event *event = &ev_loop.event[i];
    event->fd = -1;
    event->flag = SNMP_EV_NONE;
    event->read = 0;
    event->write = 0;
  }
  ev_loop.start = 0;
  ev_loop.ev_no = 0;
  ev_loop.max_fd = -1;
  __ev_done();
}

int
snmp_event_add(int fd, unsigned char flag, transport_handler cb, void *ud)
{
  int i;

  for (i = 0; i < SNMP_MAX_EVENTS; i++) {
    struct snmp_event *event = &ev_loop.event[i];
    if (event->fd == fd) {
      __ev_add(event, flag);
      event->flag |= flag;
    } else if (event->fd == -1) {
      if (fd > ev_loop.max_fd) {
        ev_loop.max_fd = fd;
      }
      event->fd = fd;
      __ev_add(event, flag);
      event->flag = flag;
    } else {
      continue;
    }

    if (flag & SNMP_EV_READ) {
      event->rcb = cb;
      event->rud = ud;
    }
    if (flag & SNMP_EV_WRITE) {
      event->wcb = cb;
      event->wud = ud;
    }
    return 0;
  }

  return -1;
}

void
snmp_event_remove(int fd, unsigned char flag)
{
  int i;

  if (fd == ev_loop.max_fd) {
    ev_loop.max_fd = -1;
  }

  for (i = 0; i < SNMP_MAX_EVENTS; i++) {
    struct snmp_event *event = &ev_loop.event[i];
    if (event->fd == fd) {
      __ev_remove(event, flag);
      event->flag &= ~flag;
      if (flag & SNMP_EV_READ) {
        event->read = 0;
      }
      if (flag & SNMP_EV_WRITE) {
        event->write = 0;
      }
      if (event->flag == SNMP_EV_NONE) {
        event->fd = -1;
      }
    }
    if (fd > ev_loop.max_fd) {
      ev_loop.max_fd = fd;
    }
  }
}

static void
snmp_event_poll(void)
{
  int i;

  __ev_poll(&ev_loop);
  for (i = 0; i < SNMP_MAX_EVENTS; i++) {
    struct snmp_event *event = &ev_loop.event[i];
    if (event->read && event->rcb != NULL) {
      event->rcb(event->fd, SNMP_EV_READ, event->rud);
      event->read = 0;
    }
    if (event->write && event->wcb != NULL) {
      event->wcb(event->fd, SNMP_EV_WRITE, event->wud);
      event->write = 0;
    }
  }
}

void
snmp_event_run(void)
{
  while (ev_loop.start) {
    snmp_event_poll();
  }
}
