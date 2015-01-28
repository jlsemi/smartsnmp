SmartSNMP - A Smart SNMP Agent
==============================

[![Build Status](https://travis-ci.org/credosemi/smartsnmp.svg?branch=master)](https://travis-ci.org/credosemi/smartsnmp) [![Coverage Status](https://coveralls.io/repos/credosemi/smartsnmp/badge.svg?branch=master)](https://coveralls.io/r/credosemi/smartsnmp?branch=master)

**SmartSNMP** is a minimal easy-config agent for network management supporting
SNMPv1/v2c/v3(non-encryption) and AgentX. It is written in C99 and Lua5.1. It
can run both on PC platforms like Linux and embedded systems such as OpenWRT.

License
-------

**SmartSNMP** is licensed under GPLv2 with "PRIVATE MIB" EXCEPTION, see `LICENSE` file for more details.

Configuration and Interfaces
----------------------------

One of the biggest bonuses (aka smartest features) of this agent is that you can
write your own private mibs and load it only if you learn to write lua files as
shown in `config` and `mibs` directory.

Operation
---------

**SmartSNMP** can run in two modes: the **SNMP mode** and the **AgentX mode**. In SNMP
mode the agent will run as an independent SNMP agent and process SNMP datagram
from the client, while in AgentX mode the agent will run as an sub-agent against
NET-SNMP as the master agent and process AgentX datagram from the master.

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

Assume **SmartSNMP** is running on Ubuntu, you shall install libraries such as:

    # lua5.1
    sudo apt-get install -y lua5.1 liblua5.1-0-dev

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
      --gcov=[yes|no]             compile C source code with gcov support

You can specify options above you need to build the project.

_Installation scripts is coming soon._

Test script
-----------

Any SNMP daemon installed in you system should be closed before test.

    sudo /etc/init.d/snmpd stop

In **SNMP** mode, we would run the SmartSNMP daemon:

    cd smartsnmp
    sudo ./tests/snmp_daemon.sh

Then run test cases at another terminal:

    cd smartsnmp
    ./tests/testcase.sh

In **AgentX** mode, NET-SNMP will be tested as the master agent, so we will
download **NET-SNMP-5.7.2.1** source and build out the image in `tests` directory:

    cd smartsnmp
    ./tests/netsnmp_build.sh

Then run NET-SNMP as the master agent:

    sudo ./tests/net-snmp-release/sbin/snmpd -f -Lo -m "" -C -c tests/snmpd.conf

Then run the SmartSNMP as a sub-agent at another terminal:

    cd smartsnmp
    sudo ./tests/agentx_daemon.sh

Then run test cases at the third terminal:

    cd smartsnmp
    ./tests/testcase.sh

TODO
----

See `TODO.md`.

Authors
-------

See `AUTHORS`.
