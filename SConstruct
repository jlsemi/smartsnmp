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

# options 
AddOption(
  '--transport',
  dest='transport',
  default = 'built-in',
  type='string',
  nargs=1,
  action='store',
  metavar='[built-in|libevent|uloop]',
  help='transport you want to use'
)

AddOption(
  '--evloop',
  dest='evloop',
  default = 'select',
  type='string',
  nargs=1,
  action='store',
  metavar='[select|epoll|kqueue]',
  help='transport you want to use'
)

AddOption(
  '--with-cflags',
  dest='cflags',
  default = '',
  type='string',
  nargs=1,
  action='store',
  metavar='CFLAGS',
  help='use CFLAGS as compile time arguments (will ignore CFLAGS env)'
)

AddOption(
  '--with-ldflags',
  dest='ldflags',
  default = '',
  type='string',
  nargs=1,
  action='store',
  metavar='LDFLAGS',
  help='use LDFLAGS as link time arguments to ld (will ignore LDFLAGS env)'
)

AddOption(
  '--with-libs',
  dest='libs',
  default = '',
  type='string',
  nargs=1,
  action='store',
  metavar='LIBS',
  help='use LIBS as link time arguments to ld'
)

AddOption(
  '--with-liblua',
  dest='liblua_dir',
  default = '',
  type='string',
  nargs=1,
  action='store',
  metavar='DIR',
  help='use liblua in DIR'
)

AddOption(
  '--with-libubox',
  dest='libubox_dir',
  default = '',
  type='string',
  nargs=1,
  action='store',
  metavar='DIR',
  help='use libubox in DIR (only for transport is uloop)'
)

AddOption(
  '--with-libevent',
  dest='libevent_dir',
  default = '',
  type='string',
  nargs=1,
  action='store',
  metavar='DIR',
  help='use libevent in DIR (only for transport is libevent)'
)

env = Environment(
  ENV = os.environ,
  LIBS = ['m', 'dl'],
  CFLAGS = ['-std=c99', '-Wall', '-Os']
)

# handle options/environment varibles.
if os.environ.has_key('CC'):
  env.Replace(CC = os.environ['CC'])

# CFLAGS
if GetOption("cflags") != "":
  env.Append(CFLAGS = GetOption("cflags"))
elif os.environ.has_key('CFLAGS'):
  env.Append(CFLAGS = os.environ['CFLAGS'])

# LDFLAGS
if GetOption("ldflags") != "":
  env.Replace(LINKFLAGS = GetOption("ldflags"))
elif os.environ.has_key('LDFLAGS'):
  env.Replace(LINKFLAGS = os.environ['LDFLAGS'])
elif os.environ.has_key('LINKFLAGS'):
  env.Replace(LINKFLAGS = os.environ['LINKFLAGS'])

# LIBS
if GetOption("libs") != "":
  env.Append(LIBS = GetOption("libs"))

# liblua
if GetOption("liblua_dir") != "":
  env.Append(LIBPATH = [GetOption("liblua_dir")])

# libevent
if GetOption("libevent_dir") != "":
  env.Append(LIBPATH = [GetOption("libevent_dir")])

# libubox
if GetOption("libubox_dir") != "":
  env.Append(LIBPATH = [GetOption("libubox_dir")])

# transport select
if GetOption("transport") == 'libevent':
  env.Append(LIBS = ['event'])
  transport_src = env.Glob("core/libevent_transport.c")
elif GetOption("transport") == 'uloop':
  env.Append(LIBS = ['ubox'])
  transport_src = env.Glob("core/uloop_transport.c")
elif GetOption("transport") == 'built-in' or GetOption("transport") == '':
  transport_src = env.Glob("core/transport.c") + env.Glob("core/ev_loop.c")
else:
  print "Error: Transport not found!"
  Exit(1)

# built-in event loop
if GetOption("evloop") == 'epoll':
  env.Append(CFLAGS = ["-DUSE_EPOLL"])
elif GetOption("evloop") == 'kqueue':
  env.Append(CFLAGS = ["-DUSE_KQUEUE"])
elif GetOption("evloop") == 'select' or GetOption("evloop") == '':
  print ""
else:
  print "Error: Not the right event driving type"
  Exit(1)

# autoconf
conf = Configure(env)

# find liblua
# on Ubuntu, liblua is named liblua5.1, so we need to check this.
if conf.CheckLib('lua'):
  env.Append(LIBS = ['lua'])
elif conf.CheckLib('lua5.1'):
  env.Append(LIBS = ['lua5.1'])
else:
  print "Error: liblua or liblua5.1 not found!"
  Exit(1)

# find lua header files
if conf.CheckCHeader('lua.h'):
  pass
elif conf.CheckCHeader('lua5.1/lua.h'):
  env.Append(CFLAGS = ['-I/usr/include/lua5.1'])
else:
  print "Error: lua.h not found"
  Exit(1)

env = conf.Finish()

src = env.Glob("core/asn1_*.c") + env.Glob("core/snmp_msg_*.c") + env.Glob("core/ans1_*.c") + env.Glob("core/smartsnmp.c") + env.Glob("core/mib_tree.c") + transport_src

# generate lua c module
libsmartsnmp_core = env.SharedLibrary('build/smartsnmp/core', src, SHLIBPREFIX = '')
