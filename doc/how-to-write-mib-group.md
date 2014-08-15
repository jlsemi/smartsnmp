How to Write MIB Group for SmartSNMP
====================================

  This document is a simple tutorial on how to write mib instance groups in
Lua5.1 for the upper logic dynamical changes of SmartSNMP.

  It is noted that the instance group is referred as to the bottom gourp in mib
tree. Nested groups are not allowed in our examples while they are represented
as oid prefixes registerd in C module in which mib tree is made.

  In general, an instance group is represented as a Lua table which is the first
class data structure in Lua. In this container, we can define four objects as
mentioned in SNMP RFC: Scalar, Table, Entry and List. SmartSNMP also provides
revelent constructure interfaces of each object variables.

  The constructure functions of mib variables are defined in init.lua. The
signature of each function indicates the read/write access permissions and BER
codec tags. The arguments of the function are the get/set methods of the value
data implemented by users. Before we start from an mib file, we shall require
the init.lua to get access to these methods.

Variable Constructure
---------------------

  Let us take system.lua as an example. In this file we have defined a table
called sysGroup represented as mib system group. In this group, we need to
constructure a Scalar variable named sysDesc which can show the full name and
version identification of the system hardware type, software OS and networking.
So we can write the constructure like this.

    local mib = require "smartsnmp"
    local sysDesc = 1
    local sysGroup = {
        [sysDesc] = mib.ConstString(function () reuturn mib.sh_call("uname -a") end)
        ...
    }

  The [sysDesc] is the system group indice defined as a Scalar object id.
ConstString shows that the variable is read-only and string type. And then we
define a closure function as the get method which will return mib.sh_call method
provided in init.lua. In this method Lua VM will execute a shell command and
return a string value. We did not write a set method bocause the variable is
read-only.

  Next we will implement sysORTable and sysOREntry. In SmartSNMP Table and Entry
are also represented as Lua table containers, and List is a variable of a sequence
of instance value. So we may well provide get/set methods different from those
for Scalar variable.

    [sysORTable]      = {
        [sysOREntry]  = {
            [sysORIndex]  = mib.UnaIndex(function () return or_index_cache end),
            [sysORID]     = mib.ConstOid(function (i) return or_table_cache[i].oid end),
            [sysORDesc]   = mib.ConstString(function (i) return or_table_cache[i].desc end),
            [sysORUpTime] = mib.ConstTimeticks(function (i) return os.difftime(os.time(), or_table_cache[i].uptime) * 100 end),
        }
    }

  Note: On Table object can hold only on Entry object. If you want more Entry
objects, define other Table objects to contain them.

  Now there are four List variables in sysOREntry. Amang these variables the
sysORIndex is the index variable of sysOREntry. We use mib.UnaIndex constructure
to generate a sequence of indexes which is unaccessible (invisible in response).
You may also use mib.ConstIndex to let them be shown in SNMP response datagram.
For other List variables, we use or_table_cache to record the instance value
from configure files and other non-RAM places. The or_table_cache composes of
several rows, each of them corresponds to one record which contains one
respective instance value of all the other List variables.

    local row = {
        oid = { 1, 3, 6, 1, 2, 1, 1 }
        desc = "The MIB module for managing IP and ICMP inplementations"
        uptime = os.time()
    }
    table.insert(or_table_cache, row)

    for i in ipairs(or_table_cache) do
        table.insert(or_index_cache, i)
    end

Community String
----------------

  In SmartSNMP mibs we can also set private community strings for each group
which is independent of the global community strings like this.

    local ipGroup = {
        rocommunity = "public"
        rwcommunity = "ipprivate"
        ...
    }

OR Table Register
-----------------

  A new mib group can be registered as a record in sysORTable by calling
mib.module_methods.or_table_reg with arguments of oid prefix and description.
