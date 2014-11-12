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

#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#include <stdint.h>

#define TRANS_BUF_SIZ  (65536)

typedef void (*TRANSPORT_RECEIVER)(uint8_t *buf, int len);

struct smartsnmp_transport_ops {
  const char *name;
  void (*init)(int port, TRANSPORT_RECEIVER recv_cb);
  void (*running)(void);
  void (*send)(uint8_t *buf, int len);
};

extern struct smartsnmp_transport_ops *smartsnmp_trans_ops;
extern struct smartsnmp_transport_ops tcp_trans_ops;
extern struct smartsnmp_transport_ops udp_trans_ops;
extern struct smartsnmp_transport_ops le_trans_ops;
extern struct smartsnmp_transport_ops uloop_trans_ops;

#endif /* _TRANSPORT_H_ */
