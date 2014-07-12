-- 
-- This file is part of SmartSNMP
-- Copyright (C) 2014, Credo Semiconductor Inc.
-- 
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
-- 
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
-- 
-- You should have received a copy of the GNU General Public License along
-- with this program; if not, write to the Free Software Foundation, Inc.,
-- 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
-- 

local mib = require "smartsnmp"

-- scalar index
sysDesc             = 1
sysObjectID         = 2
sysUpTime           = 3
sysContact          = 4
sysName             = 5
sysLocation         = 6
sysServices         = 7
sysORLastChange     = 8

-- table index
sysORTable          = 9

-- entry index
sysOREntry          = 1

-- list index
sysORIndex          = 1
sysORID             = 2
sysORDesc           = 3
sysORUpTime         = 4

local function sh_call(command)
    if type(command) ~= 'string' then
        return nil
    end

    local t = nil
    local f = io.popen(command)
    if f ~= nil then
        t = f:read("*line")
        f:close()
    end
    return t
end

local startup_time = 0
local or_last_changed_time = 0

local function sys_up_time()
    return function() return os.difftime(os.time(), startup_time) * 100 end
end

local function last_changed_time()
    return function() return os.difftime(os.time(), or_last_changed_time) * 100 end
end

function mib_system_startup(time)
    startup_time = time
    or_last_changed_time = time
end

mib_system_startup(os.time())

sys_or_ids = {
    { 1,3,6,1,6,3,10,3,1,1 },
}

sys_or_descs = {
    "The SNMP Management Architecture MIB",
}

sys_or_uptimes = {
    sys_up_time(),
}

sysGroup = {
    rocommunity = 'public',
    [sysDesc]         = mib.ConstString(sh_call("uname -a")),
    [sysObjectID]     = mib.ConstOid({1,3,6,1,4,1,8072,3,1}),
    [sysUpTime]       = mib.ConstTimeticks(sys_up_time()),
    [sysContact]      = mib.ConstString("Me <Me@example.org>"),
    [sysName]         = mib.ConstString("ThinkPad X200"),
    [sysLocation]     = mib.ConstString("Shanghai"),
    [sysServices]     = mib.ConstInt(72),
    [sysORLastChange] = mib.ConstTimeticks(last_changed_time()),
    [sysORTable]      = {
        [sysOREntry]  = {
            [sysORIndex]  = mib.AutoIndex(1),
            [sysORID]     = mib.ConstOidList(sys_or_ids),
            [sysORDesc]   = mib.ConstStringList(sys_or_descs),
            [sysORUpTime] = mib.ConstTimeticksList(sys_or_uptimes),
        }
    }
}

return sysGroup
