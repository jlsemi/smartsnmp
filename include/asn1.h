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

#ifndef _ANS1_H_
#define _ANS1_H_

#include <stdint.h>

#define HTON16(x) NTOH16(x)
#define HTON32(x) NTOH32(x)

#define NTOH16(x) \
  ((uint16_t)((((uint16_t)(x) & 0x00ff) << 8) | \
  (((uint16_t)(x) & 0xff00) >> 8)))

#define NTOH32(x) \
  ((uint32_t)((((uint32_t)(x) & 0x000000ffU) << 24) | \
  (((uint32_t)(x) & 0x0000ff00U) << 8) | \
  (((uint32_t)(x) & 0x00ff0000U) >> 8) | \
  (((uint32_t)(x) & 0xff000000U) >> 24)))

/* BER tags */
#define BER_TAG_BOOL            0x01
#define BER_TAG_INT             0x02
#define BER_TAG_BITSTR          0x03
#define BER_TAG_OCTSTR          0x04
#define BER_TAG_NUL             0x05
#define BER_TAG_OBJID           0x06
#define BER_TAG_SEQ             0x30
#define BER_TAG_IPADDR          0x40
#define BER_TAG_CNT             0x41
#define BER_TAG_GAU             0x42
#define BER_TAG_TIMETICKS       0x43
#define BER_TAG_OPAQ            0x44
#define BER_TAG_NO_SUCH_OBJ     0x80
#define BER_TAG_NO_SUCH_INST    0x81
#define BER_TAG_END_OF_MIB_VIEW 0x82

#define MIB_OID_MAX_LEN       64
#define MIB_VALUE_MAX_LEN     256

uint32_t ber_value_enc_test(const void *value, uint32_t len, uint8_t type);
uint32_t ber_value_enc(const void *value, uint32_t len, uint8_t type, uint8_t *buf);
uint32_t ber_length_enc_test(uint32_t value);
uint32_t ber_length_enc(uint32_t value, uint8_t *buf);

uint32_t ber_value_dec_test(const uint8_t *buf, uint32_t len, uint8_t type);
uint32_t ber_value_dec(const uint8_t *buf, uint32_t len, uint8_t type, void *value);
uint32_t ber_length_dec_test(const uint8_t *buf);
uint32_t ber_length_dec(const uint8_t *buf, uint32_t *value);

/* A map of simple data type syntax between ANSI C and ASN.1 */
typedef int integer_t;
typedef char octstr_t;
typedef unsigned int count_t;
typedef unsigned int count32_t;
typedef unsigned long count64_t;
typedef unsigned short oid_t;
typedef unsigned char ipaddr_t;
typedef unsigned int gauge_t;
typedef unsigned int timeticks_t;

/* variable as TLV */
typedef struct {
  uint8_t tag;
  uint16_t len;
  union {
    integer_t i;
    count_t c;
    ipaddr_t ip[6];
    octstr_t s[MIB_VALUE_MAX_LEN];
    oid_t o[MIB_OID_MAX_LEN];
    gauge_t g;
    timeticks_t t;
  } value;
} Variable;

#define tag(var) ((var)->tag)
#define length(var) ((var)->len)
#define value(var) ((void *)&((var)->value))
#define integer(var) ((var)->value.i)
#define octstr(var) ((var)->value.s)
#define count(var) ((var)->value.c)
#define oid(var) ((var)->value.o)
#define ipaddr(var) ((var)->value.ip)
#define gauge(var) ((var)->value.g)
#define timiticks(var) ((var)->value.t)

#endif /* _ANS1_H_ */
