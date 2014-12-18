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

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include <stdint.h>
#include "asn1.h" 

struct protocol_operation {
  const char *name;
  int (*init)(int port);
  int (*open)(void);
  int (*close)(void);
  void (*run)(void);
  int (*reg)(const oid_t *grp_id, int id_len, int grp_cb);
  int (*unreg)(const oid_t *grp_id, int id_len);
  void (*receive)(uint8_t *buf, int len);
  void (*send)(uint8_t *buf, int len);
};

extern struct protocol_operation snmp_prot_ops;
extern struct protocol_operation agentx_prot_ops;

#endif /* _PROTOCOL_H_ */
