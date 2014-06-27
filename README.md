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

Build
-----

    cd smartsnmp
    scons

You may select different transport protocol by this command:

    scons --transport=T  # T can be 'libevent' or 'uloop' by now

Different transport modules need respective libraries. In libevent transport you
need to install libevent and in uloop you need to install libubox. Especially, 
on Ubuntu you should install liblua5.1 while on others liblua is needed.

Test
----

Server port:

    sudo ./snmpd example/init.lua

Client port:

    ./tests/test.sh

TODO
----

See `TODO.md`.

Authors
-------

See `AUTHORS`.
