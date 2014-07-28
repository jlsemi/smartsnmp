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

local function sys_up_time()
    return os.difftime(os.time(), startup_time) * 100
end

local function last_changed_time()
    return os.difftime(os.time(), or_last_changed_time) * 100
end

local function mib_system_startup(time)
    startup_time = time
    or_last_changed_time = time
end

mib_system_startup(os.time())

local sys_or_ids_ = {
    { 1,3,6,1,6,3,10,3,1,1 },
}

local sys_or_descs_ = {
    "The SNMP Management Architecture MIB",
}

local sysGroup = {
    rocommunity = 'public',
    [sysDesc]         = mib.ConstString(function () return mib.sh_call("uname -a") end),
    [sysObjectID]     = mib.ConstOid(function () return {1,3,6,1,4,1,8072,3,1} end),
    [sysUpTime]       = mib.ConstTimeticks(sys_up_time),
    [sysContact]      = mib.ConstString(function () return "Me <Me@example.org>" end),
    [sysName]         = mib.ConstString(function () return "ThinkPad X200" end),
    [sysLocation]     = mib.ConstString(function () return "Shanghai" end),
    [sysServices]     = mib.ConstInt(function () return 72 end),
    [sysORLastChange] = mib.ConstTimeticks(last_changed_time),
    [sysORTable]      = {
        [sysOREntry]  = {
            [sysORIndex]  = mib.AutoIndexUna(1),
            [sysORID]     = mib.ConstOid(function (i) return sys_or_ids_[i] end),
            [sysORDesc]   = mib.ConstString(function (i) return sys_or_descs_[i] end),
            [sysORUpTime] = mib.ConstTimeticks(sys_up_time),
        }
    }
}

return sysGroup
