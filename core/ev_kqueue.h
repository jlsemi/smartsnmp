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

#include <unistd.h>
#include <sys/event.h>

struct kqueue_env {
  int kqfd;
  struct kevent event[SNMP_MAX_EVENTS];
};

static struct kqueue_env env;

static int
__ev_init(void)
{
  env.kqfd = kqueue();
  return env.kqfd;
}

static void
__ev_done(void)
{
  close(env.kqfd);  
}

static void
__ev_add(struct snmp_event *event, unsigned char flag)
{
  struct kevent ke;

  if (flag & SNMP_EV_READ) {
    EV_SET(&ke, event->fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  }
  if (flag & SNMP_EV_WRITE) {
    EV_SET(&ke, event->fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
  }
  kevent(env.kqfd, &ke, 1, NULL, 0, NULL);
}
 
static void
__ev_remove(struct snmp_event *event, unsigned char flag)
{
  struct kevent ke;

  if (flag & SNMP_EV_READ) {
    EV_SET(&ke, event->fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
  }
  if (flag & SNMP_EV_WRITE) {
    EV_SET(&ke, event->fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
  } 
  kevent(state->kqfd, &ke, 1, NULL, 0, NULL);
}

static int
__ev_poll(struct snmp_event_loop *ev_loop)
{
  int i;

  int nfds = kevent(kqfd, NULL, 0, env.event, SNMP_MAX_EVENTS, NULL);
  if (nfds > 0) {
    for (i = 0; i < SNMP_MAX_EVENTS; i++) {
      struct kevent *ke = &env.event[i];
      struct snmp_event *event = &ev_loop->event[i];
      if (ke->filter == EVFILT_READ) {
        event->read = 1;
      }
      if (ke->filter == EVFILT_WRITE) {
        event->write = 1;
      }
    }
  }

  return nfds;
}
