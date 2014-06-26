# This file is part of SmartSNMP
# Copyright (C) 2014, Credo Semiconductor Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import os

AddOption(
  '--transport',
  dest='transport',
  default = 'libevent',
  type='string',
  nargs=1,
  action='store',
  metavar='libevent OR uloop',
  help='choose a transport'
)

env = Environment(
  ENV = os.environ,
  LIBS = ['m', 'dl'],
  CFLAGS = '-std=c99 -Wall -Os -Iinclude ',
)

# Detect CC/CFLAGS/LINKFLAGS/LDFLAGS environments
if os.environ.has_key('CC'):
  env.Replace(CC = os.environ['CC'])

if os.environ.has_key('CFLAGS'):
  env.Append(CFLAGS = os.environ['CFLAGS'])

if os.environ.has_key('LINKFLAGS'):
  env.Replace(LINKFLAGS = os.environ['LINKFLAGS'])

if os.environ.has_key('LDFLAGS'):
  env.Replace(LINKFLAGS = os.environ['LDFLAGS'])

# Transport select
if GetOption("transport") == 'libevent':
  env.Append(LIBS = ['event'])
  transport_src = env.Glob("src/c/libevent_transport.c")
else:
  env.Append(LIBS = ['ubox'])
  transport_src = env.Glob("src/c/uloop_transport.c")

# autoconf

conf = Configure(env)

# find liblua
# on Ubuntu, liblua is named liblua5.1, so we need to check this.
if conf.CheckLib('lua'):
  env.Append(LIBS = ['lua'])
elif conf.CheckLib('lua5.1'):
  env.Append(LIBS = ['lua5.1'])
else:
  print "Can't find liblua or liblua5.1!"
  Exit(1)

# find lua header files
if conf.CheckCHeader('lua.h'):
  pass
elif conf.CheckCHeader('lua5.1/lua.h'):
  env.Append(CFLAGS = ' -I/usr/include/lua5.1 ')
else:
  print "Can't find lua.h"
  Exit(1)

env = conf.Finish()

src = env.Glob("src/c/asn1_*.c") + env.Glob("src/c/snmp_msg_*.c") + env.Glob("src/c/ans1_*.c") + env.Glob("src/c/snmpd.c") + env.Glob("src/c/mib.c") + transport_src
snmpd = env.Program('snmpd', src)
