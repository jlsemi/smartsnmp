#!/bin/bash

# SNMPv2 cases

snmpget -v2c -cpublic localhost .
snmpget -v2c -cpublic localhost .0
snmpget -v2c -cpublic localhost .1.3
snmpget -v2c -cpublic localhost .1.4
snmpget -v2c -cpublic localhost .1.5.6.7.8.100

snmpget -v2c -cpublic localhost .1.3.6.1.2.1.1.9.1.2.1
snmpget -v2c -cpublic localhost .1.3.6.1.2.1.1.1.0 .1.3.6.1.2.1.1.2.0
snmpget -v2c -cpublic localhost .1.3.6.1.2.1.1.9.1.1
snmpget -v2c -cpublic localhost .1.3.6.1.2.1.1.9.1.2
snmpget -v2c -cpublic localhost .1.3.6.1.2.1.1.9.1.5
snmpget -v2c -cpublic localhost .1.3.6.1.2.1.1.0

snmpgetnext -v2c -cpublic localhost .
snmpgetnext -v2c -cpublic localhost .0
snmpgetnext -v2c -cpublic localhost .1.3
snmpgetnext -v2c -cpublic localhost .1.4
snmpgetnext -v2c -cpublic localhost .1.5.6.7.8.100

snmpbulkget -v2c -cpublic localhost .1.3.6.1.2.1.1

# Error test (community authorization)
snmpset -v2c -cpublic localhost .1.3.6.1.2.1.1.9.1.1 i 1
# Error test (community authorization)
snmpset -v2c -cpublic localhost .1.3.6.1.2.1.4.1.0 s "This agent is really smart!"
# Error test (community authorization)
snmpset -v2c -cpublic localhost .1.3.6.1.2.1.4.1.0 i 8888

# Error test (unaccessible)
snmpset -v2c -cprivate localhost .1.3.6.1.2.1.1.9.1.1 i 1
# Error test (wrong type)
snmpset -v2c -cprivate localhost .1.3.6.1.2.1.4.1.0 s "This agent is really smart!"
# OK test
snmpset -v2c -cprivate localhost .1.3.6.1.2.1.4.1.0 i 8888

snmpwalk -v2c -cpublic localhost .1.3.6.1.2.1.1
snmpwalk -v2c -cpublic localhost .

# SNMPv3 cases (non-encrpted)

snmpget -u noAuthUser -l noAuthNoPriv localhost .
snmpget -u noAuthUser -l noAuthNoPriv localhost .0
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.3
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.4
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.5.6.7.8.100

snmpget -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.9.1.2.1
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.1.0 .1.3.6.1.2.1.1.2.0
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.9.1.1
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.9.1.2
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.9.1.5
snmpget -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.0

snmpgetnext -u noAuthUser -l noAuthNoPriv localhost .
snmpgetnext -u noAuthUser -l noAuthNoPriv localhost .0 
snmpgetnext -u noAuthUser -l noAuthNoPriv localhost .1.3
snmpgetnext -u noAuthUser -l noAuthNoPriv localhost .1.4
snmpgetnext -u noAuthUser -l noAuthNoPriv localhost .1.5.6.7.8.100

snmpbulkget -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1

# Error test (user authorization)
snmpset -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.9.1.1 i 1
# Error test (user authorization)
snmpset -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.4.1.0 s "This agent is really smart!"
# Error test (user authorization)
snmpset -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.4.1.0 i 8888

# Error test (unaccessible)
snmpset -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1.9.1.1 i 1
# Error test (wrong type)
snmpset -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.4.1.0 s "This agent is really smart!"
# OK test
snmpset -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.4.1.0 i 8888

snmpwalk -u noAuthUser -l noAuthNoPriv localhost .1.3.6.1.2.1.1
snmpwalk -u noAuthUser -l noAuthNoPriv localhost .
