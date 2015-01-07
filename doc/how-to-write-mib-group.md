How To Write MIB Groups for SmartSNMP
=====================================

This document is a simple tutorial on how to write mib groups in Lua for the
upper businesses of SmartSNMP.

Configuration
-------------

First we would tell you on how to write the configuration file. Now we have had
two examples shown in `config` directroy which indicate such fields as protocol
name, port number and mib module path.

    protocol = 'snmp'
    port = 161
    mib_module_path = 'mibs'

As for access control, to set community string for SNMPv2c and user name for
SNMPv3, we have defined two fields in a Lua table: 'name' and 'views'. 'name' is
the string, and 'views' is the set of oid views to be allowed to be accessed
(e.x. "." means all nodes registered are available) with respective read and
write permissions.

    communities = {
        { community = 'public', views = { ["."] = 'ro' } },
        { community = 'private', views = { ["."] = 'rw' } }
    }

    users = {
        {
            user = 'Jack',
            views = {
                        [".1.3.6.1.2.1.1"] = 'ro',
                        [".1.3.6.1.2.1.4"] = 'rw',
                    }
        },
        {
            user = 'Rose',
            views = {
                        [".1.3.6.1.2.1.4"] = 'ro',
                        [".1.3.6.1.2.1.1"] = 'rw',
                    }
        }
    }


To register mib modules, we provide a Lua table called 'mib_modules' containing
the oid and the name of mib groups:

    mib_modules = {
        [".1.3.6.1.2.1.1"] = 'system',
        [".1.3.6.1.2.1.2"] = 'interfaces',
        [".1.3.6.1.2.1.4"] = 'ip',
        [".1.3.6.1.2.1.6"] = 'tcp',
        [".1.3.6.1.2.1.7"] = 'udp',
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
allowed in our examples. A group is represented by a Lua table as the first
class data structure in Lua and will be returned by the mib file when loaded.

    local sysGroup = {
        ...
    }

    return sysGroup

In the group container, we can define four objects as mentioned in SNMP RFC:
Scalar, Table, Entry and Variable. SmartSNMP also provides relevant constructor
methods for each object.

Variable Constructor
--------------------

The constructor methods of each group variable are defined in `init.lua`. The
signature of each constructor indicates the ASN.1 tags and the read/write access
permissions of each mib variable. The arguments of the constructor are get/set
methods of the data value set by users since function is the first-class value
in Lua. Before we start to write it, we shall require the `init.lua` to get
access to these methods.

Let us take `system.lua` as an example. In this file we have defined a Lua table
called 'sysGroup' representing the system mib group. In this group, we need to
constructure a scalar object named 'sysDesc' which can show the full name and
version identification of the system hardware type, software OS and networking.
So we can write the variable constructor like this.

    local mib = require "smartsnmp"
    local sysDesc = 1
    local sysGroup = {
        [sysDesc] = mib.ConstString(function() reuturn mib.sh_call("uname -a", "*line") end),
        ...
    }
    return sysGroup

The 'sysDesc' is the group table indice and defined as a scalar object id.
'mib.ConstString' shows that the variable is read-only and string type. And then
we have defined a get method which returns 'mib.sh_call' method provided by
`init.lua` as required before. When the method invoked, Lua VM will execute a
shell command and return a string value. We do not need to write a set method
because the scalar object is read-only.

Table and Entry
---------------

Next we will implement 'sysORTable' and 'sysOREntry'. In SmartSNMP Table and
Entry are also represented by Lua tables. However, the get/set methods are
somewhat different from those in scalar object because Table and Entry are
sequence objects.

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

**Note:** One table object can hold only one entry object. If you want one more
entry object, define another table object to contain it.

In 'sysORTable' we have defined 'or_entry_cache' recording the ORTable
information. It is referred to by 'indexes' field so as to tell SmartSNMP the
lexicographical traversal order of 'sysORTable' by generating a temporal index
table through `init.lua`.

Now there are four variables in 'sysOREntry'. The 'sysORIndex' is the index
variable of 'sysOREntry'. Its get method would return [value, err_stat] pair so
as to show that this variable is unaccessible (invisible in SNMP response).  The
'err_stat' can be a dummy value as defaut which can be ignored in return. Other
variables are stored in 'or_entry_cache' which can load data value from
configuration files or other non-RAM places. The 'or_entry_cache' comprises
several rows, each of them corresponds to the lexicographical sorted variable in
'sysOREntry'.

    local row = {
        oid = { 1, 3, 6, 1, 2, 1, 1 },
        desc = "The MIB module for managing IP and ICMP inplementations",
        uptime = os.time(),
    }
    table.insert(or_entry_cache, row)

Long Index
----------

There are three styles of variable indexes referred to by 'indexes' field in
entry objects: single index, long index, and cascaded index.

Long index is a string of multiple continous oid numbers such as an IP address
(maybe following a port number). In 'udpTable' we examplified that with two
entry objects and the 'udp_entry_cache'. By the way, you would better check if
the type of 'sub_oid' argument is Lua table when it is passed down.

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
                                     if type(sub_oid) == 'table' and udp_entry_cache[table.concat(sub_oid, ".")] ~= nil then
                                         ipaddr = {}
                                         for i = 1, 4 do
                                             table.insert(ipaddr, sub_oid[i])
                                         end
                                     end
                                     return ipaddr
                                  end),
            [2] = mib.ConstInt(function (sub_oid)
                                   local port
                                   if type(sub_oid) == 'table' and udp_entry_cache[table.concat(sub_oid, ".")] ~= nil then
                                       port = sub_oid[5]
                                   end
                                   return port
                               end),
        }
    }

Cascaded Indexes
----------------

SmartSNMP also supports cascaded indexes in Table and Entry. 'TwoIndexTable' and
'ThreeIndexTable' group respectively show the two-index-cascaded and
three-index-cascaded indexing. The entry cache that 'indexes' field refers to is
written as below, note we have used a bool indicator called 'cascaded'.

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

Indexes Verification
--------------------

After you have finished your private mibs, you may check if each group can be
walked in lexicographical order correctly by using 'mib.group_index_table_check'
method before it returns in the lua file. 

    local udpGroup = {
        ...
    }
    mib.group_index_table_check(udpGroup, 'udpGroup')
    return udpGroup

This method will print all the indexes of the group on terminal and help you
roughly locate the invalid indexes of the group that you have constructured.

OR Table Register
-----------------

A new registered mib group can be stored as a record in 'sysORTable'(if there
are any) by calling 'mib.module_methods.or_table_reg' with arguments of a string
of group oid and a piece of description:

    mib.module_methods.or_table_reg("1.3.6.1.2.1.4", "The MIB module for managing IP and ICMP inplementations")

Then the registry of the new group will be shown as three fields of 'sysORID',
'sysORDesc' and 'sysORUpTime' stored in 'sysORTable' and shown in SNMP query
response later.
