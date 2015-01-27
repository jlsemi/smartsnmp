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

#include "agentx.h"
#include "transport.h"
#include "protocol.h"
#include "mib.h"
#include "util.h"

struct agentx_datagram agentx_datagram;

/* Receive agentX request datagram from transport layer */
static void
agentx_receive(uint8_t *buf, int len)
{
  agentx_recv(buf, len);
}

/* Send agentX response datagram to transport layer */
static void
agentx_send(uint8_t *buf, int len)
{
  agentx_trans_ops.send(buf, len);
}

/* Register mib group node */
static int
agentx_mib_node_reg(const oid_t *grp_id, int id_len, int grp_cb)
{
  struct x_pdu_buf x_pdu;

  /* Check oid prefix */
  if (id_len < 4 || grp_id[0] != 1 || grp_id[1] != 3 || grp_id[2] != 6 || grp_id[3] != 1) {
    SMARTSNMP_LOG(L_ERROR, "Oid prefix must be .1.3.6.1!\n");
    return -1;
  }

  /* Send agentX register PDU */
  x_pdu = agentx_register_pdu(&agentx_datagram, grp_id, id_len, NULL, 0, 0, 127, 0, 0);
  if (send(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    SMARTSNMP_LOG(L_ERROR, "Send agentX register PDU failure!\n");
    return -1;
  }

  /* Receive agentX resoponse PDU */
  x_pdu.len = TRANS_BUF_SIZ;
  x_pdu.buf = xrealloc(x_pdu.buf, x_pdu.len);
  x_pdu.len = recv(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0);
  if (x_pdu.len == -1) {
    SMARTSNMP_LOG(L_ERROR, "Receive agentX register response PDU failure!\n");
    return -1;
  }

  /* Verify register response PDU */
  if (agentx_recv(x_pdu.buf, x_pdu.len) != AGENTX_ERR_OK) {
    SMARTSNMP_LOG(L_ERROR, "Parse agentX rigister response PDU error!\n");
    return -1;
  }

  /* Register node */
  return mib_node_reg(grp_id, id_len, grp_cb);
}

/* Unregister mib group node */
static int
agentx_mib_node_unreg(const oid_t *grp_id, int id_len)
{
  struct x_pdu_buf x_pdu;

  /* Check oid prefix */
  if (id_len < 4 || grp_id[0] != 1 || grp_id[1] != 3 || grp_id[2] != 6 || grp_id[3] != 1) {
    SMARTSNMP_LOG(L_ERROR, "Oid prefix must be .1.3.6.1!");
    return -1;
  }

  /* Send angentX register PDU */
  x_pdu = agentx_unregister_pdu(&agentx_datagram, grp_id, id_len, NULL, 0, 0, 127, 0, 0);
  if (send(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    SMARTSNMP_LOG(L_ERROR, "Send agentX unregister PDU failure!");
    return -1;
  }

  /* Receive agentX response PDU */
  x_pdu.len = TRANS_BUF_SIZ;
  x_pdu.buf = xrealloc(x_pdu.buf, x_pdu.len);
  x_pdu.len = recv(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0);
  if (x_pdu.len == -1) {
    SMARTSNMP_LOG(L_ERROR, "Receive agentX unregister response PDU failure!\n");
    return -1;
  }

  /* Verify register response PDU */
  if (agentx_recv(x_pdu.buf, x_pdu.len) != AGENTX_ERR_OK) {
    SMARTSNMP_LOG(L_ERROR, "Unregister response error!");
    return -1;
  }

  /* Unregister node */
  mib_node_unreg(grp_id, id_len);
  return 0;
}

static int
agentx_init(int port)
{
  INIT_LIST_HEAD(&agentx_datagram.vb_in_list);
  INIT_LIST_HEAD(&agentx_datagram.vb_out_list);
  INIT_LIST_HEAD(&agentx_datagram.sr_in_list);
  INIT_LIST_HEAD(&agentx_datagram.sr_out_list);
  return agentx_trans_ops.init(port);
}

static int
agentx_open(void)
{
  struct x_pdu_buf x_pdu;
  const char *descr = "SmartSNMP AgentX sub-agent";

  /* Send agentX open PDU */
  x_pdu = agentx_open_pdu(&agentx_datagram, NULL, 0, descr, strlen(descr));
  if (send(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    SMARTSNMP_LOG(L_ERROR, "Send agentX open PDU failure!\n");
    return -1;
  }

  /* Receive agentX open response PDU */
  x_pdu.len = TRANS_BUF_SIZ;
  x_pdu.buf = xrealloc(x_pdu.buf, x_pdu.len);
  x_pdu.len = recv(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0);
  if (x_pdu.len == -1) {
    SMARTSNMP_LOG(L_ERROR, "Receive agentX open response PDU failure!\n");
    return -1;
  }

  /* Verify open response PDU */
  if (agentx_recv(x_pdu.buf, x_pdu.len) != AGENTX_ERR_OK) {
    SMARTSNMP_LOG(L_ERROR, "Parse agentX open response PDU error!\n");
    return -1;
  }

  return 0;
}

static int
agentx_close(void)
{
  struct x_pdu_buf x_pdu;

  /* Send agentX close PDU */
  x_pdu = agentx_close_pdu(&agentx_datagram, R_SHUTDOWN);
  if (send(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0) == -1) {
    SMARTSNMP_LOG(L_ERROR, "Send agentX close PDU failure!\n");
    return -1;
  }

  /* Receive agentX close response PDU */
  x_pdu.len = TRANS_BUF_SIZ;
  x_pdu.buf = xrealloc(x_pdu.buf, x_pdu.len);
  x_pdu.len = recv(agentx_datagram.sock, x_pdu.buf, x_pdu.len, 0);
  if (x_pdu.len == -1) {
    SMARTSNMP_LOG(L_ERROR, "Receive agentX close response PDU failure!\n");
    return -1;
  }

  /* Verify close response PDU */
  if (agentx_recv(x_pdu.buf, x_pdu.len) != AGENTX_ERR_OK) {
    SMARTSNMP_LOG(L_ERROR, "Parse agentX close response PDU error!\n");
    return -1;
  }
  
  agentx_trans_ops.stop();
  return 0;
}

static void
agentx_run(void)
{
  return agentx_trans_ops.running();
}

struct protocol_operation agentx_prot_ops = {
  "agentx",
  agentx_init,
  agentx_open,
  agentx_close,
  agentx_run,
  agentx_mib_node_reg,
  agentx_mib_node_unreg,
  agentx_receive,
  agentx_send,
};
