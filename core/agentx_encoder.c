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

/* Input:  value pointer, number of elements, value type
 * Output: none
 * Return: byte length.
 */
uint32_t
agentx_value_enc_try(uint32_t len, uint8_t type)
{
  uint32_t ret;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      ret = sizeof(uint32_t);
      break;
    case ASN1_TAG_CNT64:
      ret = sizeof(uint64_t);
      break;
    case ASN1_TAG_OBJID:
      ret = len * sizeof(uint32_t);
      break;
    case ASN1_TAG_OCTSTR:
    case ASN1_TAG_IPADDR:
      ret = len;
      break;
    default:
      ret = 0;
      break;
  }

  return ret;
}

/* Input:  value pointer, number of elements, value type
 * Output: buffer
 * Return: byte length.
 */
uint32_t
agentx_value_enc(const void *value, uint32_t len, uint8_t type, uint8_t *buf)
{
  uint32_t i, ret;
  uint8_t *str;
  oid_t *oid_src, *oid_dest;
  uint32_t *inter_src, *inter_dest;
  uint64_t *long_src, *long_dest;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      inter_src = (uint32_t *)value;
      inter_dest = (uint32_t *)buf;
      *inter_dest = *inter_src;
      ret = sizeof(uint32_t);
      break;
    case ASN1_TAG_CNT64:
      long_src = (uint64_t *)value;
      long_dest = (uint64_t *)buf;
      *long_dest = *long_src;
      ret = sizeof(uint64_t);
      break;
    case ASN1_TAG_OBJID:
      oid_src = (oid_t *)value;
      oid_dest = (oid_t *)buf;
      for (i = 0; i < len; i++) {
        oid_dest[i] = oid_src[i];
      }
      ret = len * sizeof(oid_t);
      break;
    case ASN1_TAG_OCTSTR:
    case ASN1_TAG_IPADDR:
      str = (uint8_t *)value;
      memcpy(buf, str, len);
      ret = len;
      break;
    default:
      ret = 0;
      break;
  }

  return ret;
}
