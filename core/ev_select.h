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

#include <string.h>
#include <sys/select.h>

struct select_env {
  fd_set rfds, wfds;
  fd_set rfds_, wfds_;
};

static struct select_env env;

static int
__ev_init(void)
{
  FD_ZERO(&env.rfds);
  FD_ZERO(&env.wfds);
  return 0;
}

static void
__ev_done(void)
{
  return;
}

static void
__ev_add(struct snmp_event *event, unsigned char flag)
{
  if (flag & SNMP_EV_READ) {
    FD_SET(event->fd, &env.rfds);
  }
  if (flag & SNMP_EV_WRITE) {
    FD_SET(event->fd, &env.wfds);
  }
}
 
static void
__ev_remove(struct snmp_event *event, unsigned char flag)
{
  if (flag & SNMP_EV_READ) {
    FD_CLR(event->fd, &env.rfds);
  }
  if (flag & SNMP_EV_WRITE) {
    FD_CLR(event->fd, &env.wfds);
  } 
}

static int
__ev_poll(struct snmp_event_loop *ev_loop)
{
  int i;

  memcpy(&env.rfds_, &env.rfds, sizeof(fd_set));
  memcpy(&env.wfds_, &env.wfds, sizeof(fd_set));

  int nfds = select(ev_loop->max_fd + 1, &env.rfds_, &env.wfds_, NULL, NULL);
  if (nfds > 0) {
    for (i = 0; i < SNMP_MAX_EVENTS; i++) {
      struct snmp_event *event = &ev_loop->event[i];
      if (event->flag == SNMP_EV_NONE) {
        continue;
      }
      if (event->flag & SNMP_EV_READ && FD_ISSET(event->fd, &env.rfds_)) {
        event->read = 1;
      }
      if (event->flag & SNMP_EV_WRITE && FD_ISSET(event->fd, &env.wfds_)) {
        event->write = 1;
      }
    }
  }

  return nfds;
}
