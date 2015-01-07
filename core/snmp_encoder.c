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

/* Input:  integer value
 * Output: none
 * Return: byte length.
 */
static uint32_t
ber_int_enc_try(int value)
{
  uint32_t i, len;
  union anonymous {
    uint8_t buf[sizeof(int)];
    int tmp;
  } a;

  a.tmp = value;
  len = 0;

#ifdef LITTLE_ENDIAN
  i = sizeof(int) - 1;

  if (value >= 0) {
    while (i > 0 && !a.buf[i]) {
      i--;
    }
    if (a.buf[i] & 0x80) {
      len += 1;
    }
  } else {
    while (i > 0 && a.buf[i] == 0xff) {
      i--;
    }
    if (!(a.buf[i] & 0x80)) {
      len += 1;
    }
  }

  len += i + 1;
#else
  i = 0;

  if (value >= 0) {
    while (i < sizeof(int) && !a.buf[i]) {
      i++;
    }
    if (a.buf[i] & 0x80) {
      len += 1;
    }
  } else {
    while (i < sizeof(int) && a.buf[i] == 0xff) {
      i++;
    }
    if (!(a.buf[i] & 0x80)) {
      len += 1;
    }
  }

  len += sizeof(int) - i;
#endif

  return len;
}

/* Input:  unsigned integer value
 * Output: none
 * Return: byte length.
 */
static uint32_t
ber_uint_enc_try(unsigned int value)
{
  uint32_t i, len;
  union anonymous {
    uint8_t buf[sizeof(unsigned int)];
    unsigned int tmp;
  } a;

  a.tmp = value;
  len = 0;

#ifdef LITTLE_ENDIAN
  i = sizeof(unsigned int) - 1;

  while (i > 0 && !a.buf[i]) {
    i--;
  }
  if (a.buf[i] & 0x80) {
    len += 1;
  }

  len += i + 1;
#else
  i = 0;

  while (i < sizeof(unsigned int) && !a.buf[i]) {
    i++;
  }
  if (a.buf[i] & 0x80) {
    len += 1;
  }

  len += sizeof(unsigned int) - i;
#endif

  return len;
}


/* Input:  oid pointer, number of elements
 * Output: none
 * Return: byte length.
 */
static uint32_t
ber_oid_enc_try(const oid_t *oid, uint32_t len)
{
  uint8_t i, j, tmp[10];

  if (len < 2)
    return len;

  for (j = 1, i = 2; i < len; i++) {
    uint32_t k = 0;
    oid_t id = oid[i];
    do {
      tmp[k++] = (id & 0x7f) | 0x80;
      id /= 128;
    } while (id);
    tmp[0] &= 0x7f;
    do {
      j++;
      k--;
    } while (k);
  }

  return j;
}

/* Input:  value pointer, number of elements, value type
 * Output: none
 * Return: byte length.
 */
uint32_t
ber_value_enc_try(const void *value, uint32_t len, uint8_t type)
{
  uint32_t ret;
  const oid_t *oid;
  const int *inter;
  const unsigned int *uinter;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
      inter = (const int *)value;
      ret = ber_int_enc_try(*inter);
      break;
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      uinter = (const unsigned int *)value;
      ret = ber_uint_enc_try(*uinter);
      break;
    case ASN1_TAG_OBJID:
      oid = (const oid_t *)value;
      ret = ber_oid_enc_try(oid, len);
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

/* Input:  integer value
 * Output: buffer
 * Return: byte length.
 */
static uint32_t
ber_int_enc(int value, uint8_t *buf)
{
  int i, j;
  union anonymous {
    uint8_t buf[sizeof(int)];
    int tmp;
  } a;

  a.tmp = value;
  j = 0;

#ifdef LITTLE_ENDIAN
  i = sizeof(int) - 1;

  if (value >= 0) {
    while (i > 0 && !a.buf[i]) {
      i--;
    }
    if (a.buf[i] & 0x80) {
      buf[j++] = 0x0;
    }
  } else {
    while (i > 0 && a.buf[i] == 0xff) {
      i--;
    }
    if (!(a.buf[i] & 0x80)) {
      buf[j++] = 0xff;
    }
  }

  while (i >= 0) {
    buf[j++] = a.buf[i--];
  }
#else
  i = 0;

  if (value >= 0) {
    while (i < sizeof(int) && !a.buf[i]) {
      i++;
    }
    if (a.buf[i] & 0x80) {
      buf[j++] = 0x0;
    }
  } else {
    while (i < sizeof(int) && a.buf[i] == 0xff) {
      i++;
    }
    if (!(a.buf[i] & 0x80)) {
      buf[j++] = 0xff;
    }
  }

  while (i < sizeof(int)) {
    buf[j++] = a.buf[i++];
  }
#endif

  return j;
}

/* Input:  unsigned integer value
 * Output: buffer
 * Return: byte length.
 */
static uint32_t
ber_uint_enc(int value, uint8_t *buf)
{
  int i, j;
  union anonymous {
    uint8_t buf[sizeof(unsigned int)];
    unsigned int tmp;
  } a;

  a.tmp = value;
  j = 0;

#ifdef LITTLE_ENDIAN
  i = sizeof(unsigned int) - 1;

  while (i > 0 && !a.buf[i]) {
    i--;
  }
  if (a.buf[i] & 0x80) {
    buf[j++] = 0x0;
  }

  while (i >= 0) {
    buf[j++] = a.buf[i--];
  }
#else
  i = 0;

  while (i < sizeof(unsigned int) && !a.buf[i]) {
    i++;
  }
  if (a.buf[i] & 0x80) {
    buf[j++] = 0x0;
  }

  while (i < sizeof(unsigned int)) {
    buf[j++] = a.buf[i++];
  }
#endif

  return j;
}


/* Input:  oid pointer, number of elements
 * Output: buffer
 * Return: byte length.
 */
static uint32_t
ber_oid_enc(const oid_t *oid, uint32_t len, uint8_t *buf)
{
  uint32_t i, j;
  uint8_t tmp[10];

  if (len == 0) {
    return 0;
  } else if (len == 1) {
    buf[0] = oid[0];
    return 1;
  }

  buf[0] = oid[0] * 40 + oid[1];

  for (j = 1, i = 2; i < len; i++) {
    uint32_t k = 0;
    oid_t id = oid[i];
    do {
      tmp[k++] = (id & 0x7f) | 0x80;
      id /= 128;
    } while (id);
    tmp[0] &= 0x7f;
    do {
      buf[j++] = tmp[--k];
    } while (k);
  }

  return j;
}

/* Input:  value pointer, number of elements, value type
 * Output: buffer
 * Return: byte length.
 */
uint32_t
ber_value_enc(const void *value, uint32_t len, uint8_t type, uint8_t *buf)
{
  uint32_t ret;
  const uint8_t *str;
  const oid_t *oid;
  const int *inter;
  const unsigned int *uinter;

  switch (type) {
    case ASN1_TAG_INT:
    case ASN1_TAG_CNT:
      inter = (const int *)value;
      ret = ber_int_enc(*inter, buf);
      break;
    case ASN1_TAG_GAU:
    case ASN1_TAG_TIMETICKS:
      uinter = (const unsigned int *)value;
      ret = ber_uint_enc(*uinter, buf);
      break;
    case ASN1_TAG_OBJID:
      oid = (const oid_t *)value;
      ret = ber_oid_enc(oid, len, buf);
      break;
    case ASN1_TAG_OCTSTR:
    case ASN1_TAG_IPADDR:
      str = (const uint8_t *)value;
      memcpy(buf, str, len);
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

/* Input:  length value
 * Output: none
 * Return: byte length.
 */
uint32_t
ber_length_enc_try(uint32_t value)
{
  uint32_t i, len;
  union anonymous {
    uint8_t buf[sizeof(uint32_t)];
    uint32_t tmp;
  } a;

  a.tmp = value;
  len = 0;

#ifdef LITTLE_ENDIAN
  i = sizeof(uint32_t) - 1;
  while (i > 0 && !a.buf[i]) {
    i--;
  }

  if (a.tmp > 127) {
    len += 1;
  }

  len += i + 1;
#else
  i = 0;
  while (i < sizeof(uint32_t) && !a.buf[i]) {
    i++;
  }

  if (a.tmp > 127) {
    len += 1;
  }

  len += sizeof(uint32_t) - i;
#endif

  return len;
}

/* Input:  length value
 * Output: buffer
 * Return: byte length.
 */
uint32_t
ber_length_enc(uint32_t value, uint8_t *buf)
{
  int i, j;
  union anonymous {
    uint8_t buf[sizeof(uint32_t)];
    uint32_t tmp;
  } a;

  a.tmp = value;
  j = 0;

#ifdef LITTLE_ENDIAN
  i = sizeof(uint32_t) - 1;
  while (i > 0 && !a.buf[i]) {
    i--;
  }

  if (a.tmp > 127) {
    buf[j++] = 0x80 | (i + 1);
  }

  while (i >= 0) {
    buf[j++] = a.buf[i--];
  }
#else
  i = 0;
  while (i < sizeof(uint32_t) && !a.buf[i]) {
    i++;
  }

  if (a.tmp > 127) {
    buf[j++] = 0x80 | (sizeof(uint32_t) - i);
  }

  while (i < sizeof(uint32_t)) {
    buf[j++] = a.buf[i++];
  }
#endif

  return j;
}
