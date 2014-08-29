How to Write MIB Group for SmartSNMP
====================================

This document is a simple tutorial on how to write mib instance groups in Lua5.1
for the upper logic dynamical changes of SmartSNMP.

It is noted that the instance group is referred as to the bottom gourp in mib
tree. Nested groups are not allowed in our examples while they are represented
as oid prefixes registerd in C module in which mib tree is made.

In general, an instance group is represented as a Lua table which is the first
class data structure in Lua. In this container, we can define three objects as
mentioned in SNMP RFC: Scalar, Table, and Entry. SmartSNMP also provides
relevant constructor interfaces for each object variables.

The constructor functions of mib variables are defined in init.lua. The
signature of each function indicates the read/write access permissions and BER
codec tags. The arguments of the function are the get/set methods of the value
data implemented by users. Before we start from an mib file, we shall require
the init.lua to get access to these methods.

Variable Constructor
---------------------

Let us take system.lua as an example. In this file we have defined a table
called sysGroup represented as mib system group. In this group, we need to
constructure a Scalar variable named sysDesc which can show the full name and
version identification of the system hardware type, software OS and networking.
So we can write the constructor like this.

    local mib = require "smartsnmp"
    local sysDesc = 1
    local sysGroup = {
        [sysDesc] = mib.ConstString(function () reuturn mib.sh_call("uname -a", "*line") end),
        ...
    }

The [sysDesc] is the system group indice defined as a Scalar object id.
mib.ConstString shows that the variable is read-only and string type. And then
we define a closure function as the get method which will return mib.sh_call
method provided in init.lua. In this method Lua VM will execute a shell command
and return a string value. We did not write a set functionmethod bocause the
Scalar variable is read-only.

Table and Entry
---------------

Next we will implement sysORTable and sysOREntry. In SmartSNMP Table and Entry
are also represented as Lua table containers, and Entry variables as sequences
of instance value. So we may well write get/set methods different from those in
Scalar variables.

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

    [sysORTable]      = {
        [sysOREntry]  = {
            indexes = or_entry_cache,
            [sysORIndex]  = mib.ConstInt(function () return nil, mib.SNMP_ERR_STAT_UNACCESS end),
            [sysORID]     = mib.ConstOid(function (i) return or_entry_get(i, 'oid') end),
            [sysORDesc]   = mib.ConstString(function (i) return or_entry_get(i, 'desc') end),
            [sysORUpTime] = mib.ConstTimeticks(function (i) return or_entry_get(i, 'uptime') end),
        }
    }

Note: On Table object can hold only on Entry object. If you want more Entry
objects, define other Table objects to contain them.

In sysORTable we define or_entry_cache to record the ORTable information. It
is referred to by the "indexes" member in order to tell SmartSNMP the right
traversal order of this Table by generating a temporal group index table
according to or_entry_cache.

Now there are four Entry variables in sysOREntry. Amang these variables the
sysORIndex is the index variable of sysOREntry. In its get function method we
will return [value, err_stat] pair to show that this variable is unaccessible
(invisible in response). The second value is dummy as defaut and can be ignored.
For other Entry variables, we use or_entry_cache to record the instance value
from configure files or other non-RAM places. The or_entry_cache composes of
several rows, each of them corresponds to one record which contains one
respective instance value of all the other Entry variables.

    local row = {
        oid = { 1, 3, 6, 1, 2, 1, 1 }
        desc = "The MIB module for managing IP and ICMP inplementations"
        uptime = os.time()
    }
    table.insert(or_entry_cache, row)

Long Index
----------

There are three styles of index table to be referred to by 'indexes' in Entry:
single index, long index, and cascaded index.

Long index is made up with multi-oids such as IP address following port which
means the index is made up with two or more entry variables. In udpTable we
examplified that with two entry variables as part of the index value and their
get function method as well as how to write udp_entry_cache.

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

SmartSNMP also supports cascaded indexes in Table and Entry. TwoIndexTable and
ThreeIndexTable respectively show the two-index-cascaded and three-index-cascaded
indexes. The index cache that "indexes" refers to should be written as follows.

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

More details on how-to is shown in mibs directory.

Community String
----------------

In SmartSNMP mibs we can also set private community strings in each group which
is independent of the global community strings like this. It should be matched
with the one in the request command despite the global community string is
matched.

    local ipGroup = {
        rocommunity = "public"
        rwcommunity = "ipprivate"
        ...
    }

OR Table Register
-----------------

A new mib group can be registered as a record in sysORTable by calling
mib.module_methods.or_table_reg with arguments of oid prefix and description.

    mib.module_methods.or_table_reg("1.3.6.1.2.1.4", "The MIB module for managing IP and ICMP inplementations")

Then the registry (e.x. the OID and descrption) of ipGroup will be recorded in
sysORTable and shown in SNMP request query.
