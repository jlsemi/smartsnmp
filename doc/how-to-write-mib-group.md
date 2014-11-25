How to Write MIB Group for SmartSNMP
====================================

This document is a simple tutorial on how to write mib groups in Lua5.1 for the
upper businesses of SmartSNMP.

Configuration
-------------

First we would tell you on how to write the configuration files. Now we have had
two examples shown in `config` directroy which indicate such fields as protocol
name, port number, community string and mib module path.

    protocol = 'snmp'
    port = 161
    ro_community = 'public'
    rw_community = 'private'
    mib_module_path = 'mibs'

To register mib modules, we provide an Lua table called `mib_modules` containing
the oid and name of mib groups:

    mib_modules = {
        ["1.3.6.1.2.1.1"] = 'system',
        ["1.3.6.1.2.1.2"] = 'interfaces',
        ["1.3.6.1.2.1.4"] = 'ip',
        ["1.3.6.1.2.1.6"] = 'tcp',
        ["1.3.6.1.2.1.7"] = 'udp',
        ...
    }

A pair of group oid and name represents a mib module. The group oid is shown as
the prefix of the whole oid in SNMP requests and will be registered in mib tree
in core just after the start of the agent. And the group name can be whatever as
you like.

**Note:** In AgentX mode the group node should NOT conflict with those in master
agent, otherwise the access control of the master will keep that mib group in
sub-agent from registry.

MIB Examples
------------

MIB group examples are shown in `mibs` directory. It is noted that each group
should be the bottom node in the mib tree as convention. Nested groups are not
allowed in our examples. A group is represented by a Lua table which is the first
class data structure in Lua and will be returned by the mib file when loaded.

    local sysGroup = {
        ...
    }

    return sysGroup

In the group container, we can define four objects as mentioned in SNMP RFC:
Scalar, Table, Entry and Variable. SmartSNMP also provides relevant constructor
interfaces for each object.

Variable Constructor
--------------------

The constructor interfaces of each group variable are defined in `init.lua`. The
signature of each constructor indicates the read/write access permission and
ASN.1 variable tags. The arguments of the constructor are get/set methods of the
variable data to be implemented by users. Before we start to write it, we shall
require the `init.lua` to get access to these methods.

Let us take `system.lua` as an example. In this file we have defined a table
called `sysGroup` representing mib system group. In this group, we need to
constructure a scalar variable named `sysDesc` which can show the full name and
version identification of the system hardware type, software OS and networking.
So we can write the constructor like this.

    local mib = require "smartsnmp"
    local sysDesc = 1
    local sysGroup = {
        [sysDesc] = mib.ConstString(function() reuturn mib.sh_call("uname -a", "*line") end),
        ...
    }
    return sysGroup

The `sysDesc` is the group table indice and defined as a scalar object id.
`mib.ConstString` shows that the variable is read-only and string type. And then
we have defined a closure function as the get method which will return
`mib.sh_call` method provided in `init.lua` as required before. When the method
invoked, Lua VM will execute a shell command and return a string value. We did
not write a set method because the scalar variable is read-only.

Table and Entry
---------------

Next we will implement `sysORTable` and `sysOREntry`. In SmartSNMP Table and
Entry are also represented by Lua table. However, the get/set methods are
somewhat different from those in scalar variable because Table and Entry are
sequence variable.

    local function or_entry_get(i, name)
        assert(type(name) == 'string')
        local value
        if or_entry_cache[i] then
            if name == 'uptime' then
                value = os.difftime(os.time(), or_entry_cache[i][name]) * 100
            else
                value = or_entry_cache[i][name]
            end
        end
        return value
    end

    local sysGroup = {
        [sysORTable] = {
            [sysOREntry]  = {
                indexes = or_entry_cache,
                [sysORIndex]  = mib.ConstInt(function() return nil, mib.SNMP_ERR_STAT_UNACCESS end),
                [sysORID]     = mib.ConstOid(function(i) return or_entry_get(i, 'oid') end),
                [sysORDesc]   = mib.ConstString(function(i) return or_entry_get(i, 'desc') end),
                [sysORUpTime] = mib.ConstTimeticks(function(i) return or_entry_get(i, 'uptime') end),
            }
        }
    }

Note: One table object can hold only one entry object. If you want more entry
objects, define other table objects to contain them.

In `sysORTable` we have defined `or_entry_cache` recording the ORTable
information. It is referred by `indexes` field so as to tell SmartSNMP the
lexicographical traversal order of `sysORTable` by generating a temporal index
table through `init.lua`.

Now there are four variables in `sysOREntry`. The `sysORIndex` is the index
variable of `sysOREntry`. Its get method would return [value, err_stat] pair so
as to show that this variable is unaccessible (invisible in response).  The
`err_stat` can be dummy as defaut which is ignored in return. Other variables
are stored in `or_entry_cache` which can load value data from configuration
files or other non-RAM places. The `or_entry_cache` comprises several rows, each
of them corresponds to the lexicographical sorted variable in `sysOREntry`.

    local row = {
        oid = { 1, 3, 6, 1, 2, 1, 1 },
        desc = "The MIB module for managing IP and ICMP inplementations",
        uptime = os.time(),
    }
    table.insert(or_entry_cache, row)

Long Index
----------

There are three styles of variable index referred by `indexes` field in entry
object: single index, long index, and cascaded index.

Long index is a string of multiple continous oid numbers such as IP address
(maybe following a port number). In `udpTable` we examplified that with two
entry variables as well as `udp_entry_cache`.

    local udp_entry_cache = {
        ["0.0.0.0.67"] = {},
        ["0.0.0.0.68"] = {},
        ["0.0.0.0.161"] = {},
        ["0.0.0.0.5353"] = {},
        ["0.0.0.0.44681"] = {},
        ["0.0.0.0.51586"] = {},
        ["127.0.0.1.53"] = {},
        ["192.168.122.1.53"] = {},
    }

    [udpTable] = {
        [1] = {
            indexes = udp_entry_cache,
            [1] = mib.ConstIpaddr(function (sub_oid)
                                     local ipaddr
                                     if udp_entry_cache[table.concat(sub_oid, ".")] ~= nil then
                                         ipaddr = {}
                                         for i = 1, 4 do
                                             table.insert(ipaddr, sub_oid[i])
                                         end
                                     end
                                     return ipaddr
                                  end),
            [2] = mib.ConstInt(function (sub_oid)
                                   local port
                                   if udp_entry_cache[table.concat(sub_oid, ".")] ~= nil then
                                       port = sub_oid[5]
                                   end
                                   return port
                               end),
        }
    }

Cascaded Indexes
----------------

SmartSNMP also supports cascaded indexes in Table and Entry. `TwoIndexTable` and
`ThreeIndexTable` group respectively show the two-index-cascaded and
three-index-cascaded indexing. The entry cache that `indexes` field refers is
written as below, note we have used a bool indicator called `cascaded`.

    local two_dim_entry_cache = {
        cascade = true,
        { 1, 2 },
        { 2, 3 },
    }

    [TwoIndexEntry] = {
        indexes = two_dim_entry_cache,
        ...
    }

    local three_dim_entry_cache = {
        cascade = true,
        { 1, 3, 5, 7},
        { 2, 3 },
        { 32, 123, 87 },
    }

    [ThreeIndexEntry] = {
        indexes = three_dim_entry_cache,
        ...
    }

OR Table Register
-----------------

A new registered mib group can be stored as a record in `sysORTable` by calling
`mib.module_methods.or_table_reg` with arguments of a string of group oid and a
piece of description:

    mib.module_methods.or_table_reg("1.3.6.1.2.1.4", "The MIB module for managing IP and ICMP inplementations")

Then the registry of the new group including `sysORID`, `sysORDesc` and
`sysORUpTime` will be stored in `sysORTable` and will be shown in SNMP request
query later.
