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
#include <sys/epoll.h>

struct epoll_env {
  int epfd;
  struct epoll_event event[SNMP_MAX_EVENTS];
};

static struct epoll_env env;

static int
__ev_init(void)
{
  env.epfd = epoll_create(32);
  return env.epfd;
}

static void
__ev_done(void)
{
  close(env.epfd);  
}

static void
__ev_add(struct snmp_event *event, unsigned char flag)
{
  struct epoll_event ee;
  int op = event->flag == SNMP_EV_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  ee.events = 0;
  ee.data.fd = event->fd;
  if (flag & SNMP_EV_READ) {
    ee.events |= EPOLLIN;
  }
  if (flag & SNMP_EV_WRITE) {
    ee.events |= EPOLLOUT;
  }
  epoll_ctl(env.epfd, op, event->fd, &ee);
}
 
static void
__ev_remove(struct snmp_event *event, unsigned char flag)
{
  struct epoll_event ee;

  ee.events = EPOLLIN | EPOLLOUT;
  ee.data.fd = event->fd;
  if (flag & SNMP_EV_READ) {
    ee.events &= ~EPOLLIN;
  }
  if (flag & SNMP_EV_WRITE) {
    ee.events &= ~EPOLLOUT;
  } 
  if (ee.events == 0) {
    epoll_ctl(env.epfd, EPOLL_CTL_DEL, event->fd, &ee);
  } else {
    epoll_ctl(env.epfd, EPOLL_CTL_MOD, event->fd, &ee);
  }
}

static int
__ev_poll(struct snmp_event_loop *ev_loop)
{
  int i;

  int nfds = epoll_wait(env.epfd, env.event, SNMP_MAX_EVENTS, -1);
  if (nfds > 0) {
    for (i = 0; i < SNMP_MAX_EVENTS; i++) {
      struct epoll_event *ee = &env.event[i];
      struct snmp_event *event = &ev_loop->event[i];
      if (ee->events & EPOLLIN) {
        event->read = 1;
      }
      if (ee->events & EPOLLOUT) {
        event->write = 1;
      }
    }
  }

  return nfds;
}
