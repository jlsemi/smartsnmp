SmartSNMP - A Smart SNMP Agent
==============================

[![Build Status](https://travis-ci.org/credosemi/smartsnmp.svg?branch=master)](https://travis-ci.org/credosemi/smartsnmp)

**SmartSNMP** is a minimal easy-config agent for network management based on SNMPv2c
and written in C99 and Lua5.1. It can run on both PC system like Linux and 
FreeBSD and embedded system such as OpenWRT and also on both platform of 32-bit 
and 64-bit. It can be compiled to a file with size of 20K in terms of option 
'-Os'.

License
-------

**SmartSNMP** is licensed under GPLv2, see `LICENSE` file for more details.

Configure and Interfaces
------------------------

One of the biggest bonuses (aka smartest features) of this agent is that you can
write your own private mib and loaded by it only if you learn to write a lua 
file as shown at files in the example directory.

There are two interfaces for the register and unregister of you mib nodes as
shown at example/init.lua

Operation
---------

SmartSNMP is based on SNMPv2c so it can response 5 kinds of request operations 
from client: snmpget, snmpgetnext, snmpset, snmpbulkget and snmpwalk. Revelant 
test samples are shown at tests/test.sh.

Dependencies
------------

- Lua 5.1
- One of transports
  - libevent
  - libubox/uloop (for OpenWrt)

Build
-----

Different transport modules need respective libraries. In libevent transport you
need to install libevent and in uloop you need to install libubox. Especially, 
on Ubuntu you should install liblua5.1 while on others liblua is needed.

Assume you are using Ubuntu and the transport is libevent

    # lua5.1
    sudo apt-get install -y lua5.1 liblua5.1-0-dev

    # for libevent transport
    sudo apt-get install -y libevent-dev

    # scons & git
    sudo apt-get install -y scons git

    # clone with git
    git clone https://github.com/credosemi/smartsnmp.git
    
    # build
    cd smartsnmp
    scons

For more build options, type:

    scons --help

You will get:

    ... SCons Options ...
    Local Options:
      --transport=[libevent|uloop]
                                  transport you want to use
      --with-cflags=CFLAGS        use CFLAGS as compile time arguments (will
                                    ignore CFLAGS env)
      --with-ldflags=LDFLAGS      use LDFLAGS as link time arguments to ld (will
                                    ignore LDFLAGS env)
      --with-libs=LIBS            use LIBS as link time arguments to ld
      --with-liblua=DIR           use liblua in DIR
      --with-libubox=DIR          use libubox in DIR (only for transport is uloop)
      --with-libevent=DIR         use libevent in DIR (only for transport is
                                    libevent)

You can specify any options you need to build the project.

_Installing scripts will coming soon._

Test script
-----------

Net-SNMP utils need be installed before you run test scripts (on Ubuntu).

    sudo apt-get install -y snmp

To run SNMP agent:

    sodo ./run_snmpd

Test agent in another terminal:

    ./tests/test.sh

TODO
----

See `TODO.md`.

Authors
-------

See `AUTHORS`.
