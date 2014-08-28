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
local sysDesc             = 1
local sysObjectID         = 2
local sysUpTime           = 3
local sysContact          = 4
local sysName             = 5
local sysLocation         = 6
local sysServices         = 7
local sysORLastChange     = 8

-- table index
local sysORTable          = 9
-- entry index
local sysOREntry          = 1
-- list index
local sysORIndex          = 1
local sysORID             = 2
local sysORDesc           = 3
local sysORUpTime         = 4

local startup_time = 0
local or_last_changed_time = 0

local function mib_system_startup(time)
    startup_time = time
    or_last_changed_time = time
end

mib_system_startup(os.time())

local or_oid_cache = {}
local or_entry_cache = {}

local or_table_reg = function (oid, desc)
    local entry = {}
    entry['oid'] = {}
    for i in string.gmatch(oid, "%d") do
        table.insert(entry['oid'], tonumber(i))
    end
    entry['desc'] = desc
    entry['uptime'] = os.time()
    table.insert(or_entry_cache, entry)

    or_last_changed_time = os.time()

    or_oid_cache[oid] = #or_entry_cache
end

local or_table_unreg = function (oid)
    local or_idx = or_oid_cache[oid]

    if or_entry_cache[or_idx] ~= nil then
        table.remove(or_entry_cache, or_idx)
        or_last_changed_time = os.time()
    end

    or_oid_cache[oid] = nil
end

local SNMP_ERR_STAT_ON_ACCESS = 6

local sysMethods = {
    ["or_table_reg"] = or_table_reg, 
    ["or_table_unreg"] = or_table_unreg
}
mib.module_method_register(sysMethods)

local sysGroup = {
    rocommunity = 'public',
    [sysDesc]         = mib.ConstString(function () return mib.sh_call("uname -a", "*line") end),
    [sysObjectID]     = mib.ConstOid(function () return { 1, 3, 6, 1, 2, 1, 1 } end),
    [sysUpTime]       = mib.ConstTimeticks(function () return os.difftime(os.time(), startup_time) * 100 end),
    [sysContact]      = mib.ConstString(function () return "Me <Me@example.org>" end),
    [sysName]         = mib.ConstString(function () return mib.sh_call("uname -n", "*line") end),
    [sysLocation]     = mib.ConstString(function () return "Shanghai" end),
    [sysServices]     = mib.ConstInt(function () return 72 end),
    [sysORLastChange] = mib.ConstTimeticks(function () return os.difftime(os.time(), or_last_changed_time) * 100 end),
    [sysORTable]      = {
        [sysOREntry]  = {
            indexes = or_entry_cache,
            [sysORIndex]  = mib.ConstInt(function () return nil, SNMP_ERR_STAT_ON_ACCESS end),
            [sysORID]     = mib.ConstOid(function (i)
                                             local oid
                                             if or_entry_cache[i] ~= nil then
                                                 oid = or_entry_cache[i].oid
                                             end
                                             return oid
                                         end),
            [sysORDesc]   = mib.ConstString(function (i)
                                                local desc
                                                if or_entry_cache[i] ~= nil then
                                                    desc = or_entry_cache[i].desc
                                                end
                                                return desc
                                            end),
            [sysORUpTime] = mib.ConstTimeticks(function (i)
                                                   local time
                                                   if or_entry_cache[i] ~= nil then
                                                       time = os.difftime(os.time(), or_entry_cache[i].uptime) * 100
                                                   end
                                                   return time
                                               end),
        }
    }
}

return sysGroup
