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

#include "snmp.h"
#include "transport.h"
#include "protocol.h"
#include "mib.h"
#include "util.h"

struct snmp_datagram snmp_datagram;

/* Receive SNMP request datagram from transport layer */
static void
snmpd_receive(uint8_t *buf, int len)
{
  snmpd_recv(buf, len);
}

/* Send SNMP response datagram to transport layer */
static void
snmpd_send(uint8_t *buf, int len)
{
  snmp_trans_ops.send(buf, len);
}

/* Register mib group node */
static int
snmpd_mib_node_reg(const oid_t *grp_id, int id_len, int grp_cb)
{
  return mib_node_reg(grp_id, id_len, grp_cb);
}

/* Unregister mib group nodes */
static int
snmpd_mib_node_unreg(const oid_t *grp_id, int id_len)
{
  mib_node_unreg(grp_id, id_len);
  return 0;
}

static int
snmpd_init(int port)
{
  INIT_LIST_HEAD(&snmp_datagram.vb_in_list);
  INIT_LIST_HEAD(&snmp_datagram.vb_out_list);
  return snmp_trans_ops.init(port);
}

static int
snmpd_open(void)
{
  /* dummy */
  return 0;
}

static int
snmpd_close(void)
{
  snmp_trans_ops.stop();
  return 0;
}

static void
snmpd_run(void)
{
  return snmp_trans_ops.running();
}

struct protocol_operation snmp_prot_ops = {
  "snmp",
  snmpd_init,
  snmpd_open,
  snmpd_close,
  snmpd_run,
  snmpd_mib_node_reg,
  snmpd_mib_node_unreg,
  snmpd_receive,
  snmpd_send,
};
