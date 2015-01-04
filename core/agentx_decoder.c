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
#include <stdlib.h>
#include <string.h>

#include "asn1.h"
#include "util.h"

/* Input:  buffer, byte length, value type;
 * Output: none
 * Return: value bytes
 */
uint32_t
agentx_value_dec_try(const uint8_t *buf, uint8_t flag, uint8_t type)
{
  uint32_t ret;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      ret = sizeof(uint32_t);
    case ASN1_TAG_CNT64:
      ret = sizeof(uint64_t);
      break;
    case ASN1_TAG_OCTSTR:
    case ASN1_TAG_IPADDR:
      ret = *(uint32_t *)buf;
      break;
    case ASN1_TAG_OBJID:
      ret = *buf;
      uint8_t prefix = *(buf + 1);
      if (ret == 0 && prefix == 0) {
        return 0;
      }
      ret = (ret + 5) * sizeof(uint32_t);
      break;
    default:
      ret = 0;
      break;
  }

  return ret;
}

/* Input:  buffer, flag, value type;
 * Output: value pointer
 * Return: number of elements
 */
uint32_t
agentx_value_dec(uint8_t **buffer, uint8_t flag, uint8_t type, void *value)
{
  uint8_t *buf;
  uint32_t i, ret;
  uint32_t *oid_src, *oid_dest;
  uint32_t *inter_src, *inter_dest;
  uint64_t *long_src, *long_dest;

  buf = *buffer;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      inter_src = (uint32_t *)buf;
      inter_dest = (uint32_t *)value;
      *inter_dest = *inter_src;
      buf += sizeof(uint32_t);
      ret = 1;
      break;
    case ASN1_TAG_CNT64:
      long_src = (uint64_t *)buf;
      long_dest = (uint64_t *)value;
      *long_dest = *long_src;
      buf += sizeof(uint64_t);
      ret = 1;
      break;
    case ASN1_TAG_OCTSTR:
    case ASN1_TAG_IPADDR:
      ret = *(uint32_t *)buf;
      buf += 4;
      memcpy(value, buf, ret);
      buf += uint_sizeof(ret);
      break;
    case ASN1_TAG_OBJID:
      ret = *buf++;
      uint8_t prefix = *buf++;
      buf += 2;
      if (ret == 0 && prefix == 0) {
        break;
      }
      ret += 5;
      oid_src = (uint32_t *)buf;
      oid_dest = (uint32_t *)value;
      oid_dest[0] = 1;
      oid_dest[1] = 3;
      oid_dest[2] = 6;
      oid_dest[3] = 1;
      oid_dest[4] = prefix;
      for (i = 5; i < ret; i++) {
        oid_dest[i] = oid_src[i - 5];
        buf += sizeof(uint32_t);
      }
      break;
    default:
      ret = 0;
      break;
  }

  *buffer = buf;
  return ret;
}
