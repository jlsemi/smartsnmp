SmartSNMP - A Smart SNMP Agent
==============================

[![Build Status](https://travis-ci.org/credosemi/smartsnmp.svg?branch=master)](https://travis-ci.org/credosemi/smartsnmp)

**SmartSNMP** is a minimal easy-config agent for network management supporting
SNMPv2c and AgentX. It is written in C99 and Lua5.1. It can run both on PC platforms
like Linux and embedded systems such as OpenWRT.

License
-------

**SmartSNMP** is licensed under GPLv2 with "PRIVATE MIB" EXCEPTION, see `LICENSE` file for more details.

Configure and Interfaces
------------------------

One of the biggest bonuses (aka smartest features) of this agent is that you can
write your own private mibs and loaded by it only if you learn to write lua
files as shown in `mibs` directory.

Operation
---------

**SmartSNMP** can run in two modes: the **SNMP mode** and the **AgentX mode**. In SNMP
mode the agent will run as an independent SNMP agent and process SNMP datagram
from the client, while in AgentX mode the agent will run as an sub-agent against
Net-SNMP as the master agent and process AgentX datagram from the master.

Revelant test samples are shown respectively as `tests/snmpd_test.sh` and `tests/agentx_test.sh`

Dependencies
------------

- Lua 5.1
- One of transports
  - built-in
    - select
    - kqueue (not tested yet)
    - epoll
  - libevent
  - libubox/uloop (for OpenWrt)

Build
-----

Different event-driven mechanisms and transport options need respective libraries.
For expample, in libevent transport you need to install libevent, in uloop
transport you need to install libubox. Especially, on Ubuntu you should install
liblua5.1 while liblua is needed on other platforms.

Assume **SmartSNMP** is running on Ubuntu in libevent transport, you shall install
libraries such as:

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
      --transport=[built-in|libevent|uloop]
                                  transport you want to use
      --evloop=[select|kqueue|epoll]
                                  built-in event loop type
      --with-cflags=CFLAGS        use CFLAGS as compile time arguments (will
                                    ignore CFLAGS env)
      --with-ldflags=LDFLAGS      use LDFLAGS as link time arguments to ld (will
                                    ignore LDFLAGS env)
      --with-libs=LIBS            use LIBS as link time arguments to ld
      --with-liblua=DIR           use liblua in DIR
      --with-libubox=DIR          use libubox in DIR (only for transport is uloop)
      --with-libevent=DIR         use libevent in DIR (only for transport is
                                    libevent)

You can specify options above you need to build the project.

_Installation scripts is coming soon._

Test script
-----------

Net-SNMP utils should be installed before you run all test scripts (on Ubuntu).

    sudo apt-get install -y snmp snmpd

To run in **SNMP mode**:

    sudo /etc/init.d/snmpd stop
    cd smartsnmp
    sudo ./tests/snmp_daemon.sh

Test samples at another terminal:

    ./tests/snmpd_test.sh

To run in **AgentX mode**:

    sudo mv /etc/snmp/snmpd.conf /etc/snmp/snmpd.conf.save
    cd smartsnmp
    sudo cp config/snmpd.conf /etc/snmp/snmpd.conf
    sudo /etc/init.d/snmpd restart
    ./tests/agentx_deamon.sh

Test samples at another terminal:

    ./tests/agentx_test.sh

TODO
----

See `TODO.md`.

Authors
-------

See `AUTHORS`.
