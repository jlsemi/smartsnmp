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

#ifndef _SNMP_EVENT_LOOP_H_
#define _SNMP_EVENT_LOOP_H_

#define SNMP_EV_NONE  0
#define SNMP_EV_READ  1
#define SNMP_EV_WRITE 2

typedef void (*transport_handler)(int sock, unsigned char flag, void *ud);

void snmp_event_init(void);
void snmp_event_done(void);
void snmp_event_run(void);
int snmp_event_add(int fd, unsigned char flag, transport_handler cb, void *ud);
void snmp_event_remove(int fd, unsigned char flag);

#endif /* _SNMP_EVENT_LOOP_H_ */
