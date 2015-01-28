Generate Coverage Report for SmartSNMP
======================================

Get the report from coveralls.io
--------------------------------

[Click Here](https://coveralls.io/r/credosemi/smartsnmp) to get the report on coveralls.io.

For C Code
----------

Add `--gcov=yes` to compile the source code with _gcov_ support.

Later, after you run the smartsnmp, you can use _gcov_ to analyze the report.

For Lua Code
------------

You will need to install luacov package by luarocks or your own way.

Add `LUACOV=1` when run snmp agent or subagent, like

    sudo LUACOV=1 ./tests/snmp_daemon.sh

or

    sudo LUACOV=1 ./tests/agentx_daemon.sh
