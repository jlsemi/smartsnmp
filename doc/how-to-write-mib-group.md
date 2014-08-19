How to Write MIB Group for SmartSNMP
====================================

This document is a simple tutorial on how to write mib instance groups in Lua5.1
for the upper logic dynamical changes of SmartSNMP.

In general, an instance group is represented as a Lua table. In this container,
we can define four objects as mentioned in SNMP RFC: Scalar, Table, Entry and
List. SmartSNMP also provides relevant constructor interfaces for each object
variables.

Variable constructor
---------------------

Let us take system.lua as an example. In this file we have defined a table
called sysGroup. In this group, we need to constructure a Scalar variable named
sysDesc which can show the system information. So we can write the constructor
like this.

    local mib = require "smartsnmp"
    local sysDesc = 1
    local sysGroup = {
        [sysDesc] = mib.ConstString(function () reuturn mib.sh_call("uname -a") end)
        ...
    }

Next we will implement sysORTable and sysOREntry. In SmartSNMP Table and Entry
are also Lua table containers, and List is a variable of a sequence of instance
value.

    [sysORTable]      = {
        [sysOREntry]  = {
            [sysORIndex]  = mib.UnaIndex(function () return or_index_cache end),
            [sysORID]     = mib.ConstOid(function (i) return or_table_cache[i].oid end),
            [sysORDesc]   = mib.ConstString(function (i) return or_table_cache[i].desc end),
            [sysORUpTime] = mib.ConstTimeticks(function (i) return os.difftime(os.time(), or_table_cache[i].uptime) * 100 end),
        }
    }

Community String
----------------

In SmartSNMP mibs we can also set private community strings for each group which
is independent of the global community strings like this.

    local ipGroup = {
        rocommunity = "public"
        rwcommunity = "ipprivate"
        ...
    }

