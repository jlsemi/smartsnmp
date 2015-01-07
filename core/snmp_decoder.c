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

/* Input:  buffer, byte length;
 * Output: none
 * Return: number of elements
 */
static uint32_t
ber_oid_dec_try(const uint8_t *buf, uint32_t len)
{
  uint32_t i, j;

  for (j = 2, i = 1; i < len; i++) {
    if (!(buf[i] & 0x80)) {
      j++;
    }
  }

  return j;
}

/* Input:  buffer, byte length, value type;
 * Output: none
 * Return: number of elements
 */
uint32_t
ber_value_dec_try(const uint8_t *buf, uint32_t len, uint8_t type)
{
  uint32_t ret;

  if (len == 0)
    return 0;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      ret = 1;
      break;
    case ASN1_TAG_OBJID:
      ret = ber_oid_dec_try(buf, len);
      break;
    case ASN1_TAG_OCTSTR:
    case ASN1_TAG_IPADDR:
      ret = len;
      break;
    case ASN1_TAG_SEQ:
    case ASN1_TAG_NUL:
    default:
      ret = 0;
      break;
  }

  return ret;
}

/* Input:  buffer, byte length;
 * Output: interger pointer
 * Return: number of elements
 */
static uint32_t
ber_int_dec(const uint8_t *buf, uint32_t len, int *value)
{
  int i, j;

  *value = 0;

  if (buf[0] & 0x80) {
    for (i = 0, j = 0; j < sizeof(int) - len; j++) {
      *value = (*value << 8) | 0xff;
    }
  } else {
    i = 0;
    if (!buf[0]) {
      i++;
    }
  }

  while (i < len) {
    *value = (*value << 8) | buf[i++];
  }

  return 1;
}

/* Input:  buffer, byte length;
 * Output: unsigned interger pointer
 * Return: number of elements
 */
static uint32_t
ber_uint_dec(const uint8_t *buf, uint32_t len, unsigned int *value)
{
  int i;

  i = 0;
  if (!buf[0]) {
    i++;
  }

  *value = 0;
  while (i < len) {
    *value = (*value << 8) | buf[i++];
  }

  return 1;
}


/* Input:  buffer, byte length;
 * Output: oid pointer
 * Return: number of elements
 */
static uint32_t
ber_oid_dec(const uint8_t *buf, uint32_t len, oid_t *oid)
{
  uint32_t i, j;
  oid_t id;

  oid[0] = buf[0] / 40;
  oid[1] = buf[0] % 40;

  for (id = 0, j = 2, i = 1; i < len; i++) {
    id = (id << 7) | (buf[i] & 0x7f);
    if (!(buf[i] & 0x80)) {
      oid[j++] = id;
      id = 0;
    }
  }

  return j;
}

/* Input:  buffer, byte length, value type;
 * Output: value pointer
 * Return: number of elements
 */
uint32_t
ber_value_dec(const uint8_t *buf, uint32_t len, uint8_t type, void *value)
{
  uint32_t ret;

  if (len == 0)
    return 0;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
      ret = ber_int_dec(buf, len, value);
      break;
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      ret = ber_uint_dec(buf, len, value);
      break;
    case ASN1_TAG_OBJID:
      ret = ber_oid_dec(buf, len, value);
      break;
    case ASN1_TAG_OCTSTR:
    case ASN1_TAG_IPADDR:
      memcpy(value, buf, len);
      ret = len;
      break;
    case ASN1_TAG_SEQ:
    case ASN1_TAG_NUL:
    default:
      ret = 0;
      break;
  }

  return ret;
}

/* Input:  buffer
 * Output: none
 * Return: length value
 */
uint32_t
ber_length_dec_try(const uint8_t *buf)
{
  uint32_t len = 1;

  if (buf[0] & 0x80) {
    len += buf[0] & 0x7f;
  }

  return len;
}

/* Input:  buffer
 * Output: value pointer
 * Return: length value
 */
uint32_t
ber_length_dec(const uint8_t *buf, uint32_t *value)
{
  uint32_t i = 0, len = 1;

  if (buf[0] & 0x80) {
    len += buf[0] & 0x7f;
    i++;
  }

  *value = 0;
  while (i < len)
    *value = (*value << 8) | buf[i++];

  return len;
}
