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

local icmp_scalar_cache = {}

local function __load_config()
    icmp_scalar_cache = {}
    for line in io.lines("/proc/net/snmp") do
        if string.match(line, "%w+") == 'Icmp' then
            for w in string.gmatch(line, "%d+") do
                table.insert(icmp_scalar_cache, tonumber(w))
            end
        end
    end
end

local last_load_time = os.time()

local function need_to_reload()
    if os.difftime(os.time(), last_load_time) < 3 then
        return false
    else
        last_load_time = os.time()
        return true
    end
end

local function load_config()
    if need_to_reload() == true then
        __load_config()
    end
end

__load_config()

mib.module_methods.or_table_reg("1.3.6.1.2.1.5", "The MIB module for managing icmp and ICMP inplementations")

local icmpGroup = {
     [1]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[1] end),
     [2]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[2] end),
     [3]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[4] end),
     [4]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[5] end),
     [5]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[6] end),
     [6]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[7] end),
     [7]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[8] end),
     [8]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[9] end),
     [9]  = mib.ConstCount(function () load_config() return icmp_scalar_cache[10] end),
     [10] = mib.ConstCount(function () load_config() return icmp_scalar_cache[11] end),
     [11] = mib.ConstCount(function () load_config() return icmp_scalar_cache[12] end),
     [12] = mib.ConstCount(function () load_config() return icmp_scalar_cache[13] end),
     [13] = mib.ConstCount(function () load_config() return icmp_scalar_cache[14] end),
     [14] = mib.ConstCount(function () load_config() return icmp_scalar_cache[15] end),
     [15] = mib.ConstCount(function () load_config() return icmp_scalar_cache[16] end),
     [16] = mib.ConstCount(function () load_config() return icmp_scalar_cache[17] end),
     [17] = mib.ConstCount(function () load_config() return icmp_scalar_cache[18] end),
     [18] = mib.ConstCount(function () load_config() return icmp_scalar_cache[19] end),
     [19] = mib.ConstCount(function () load_config() return icmp_scalar_cache[20] end),
     [20] = mib.ConstCount(function () load_config() return icmp_scalar_cache[21] end),
     [21] = mib.ConstCount(function () load_config() return icmp_scalar_cache[22] end),
     [22] = mib.ConstCount(function () load_config() return icmp_scalar_cache[23] end),
     [23] = mib.ConstCount(function () load_config() return icmp_scalar_cache[24] end),
     [24] = mib.ConstCount(function () load_config() return icmp_scalar_cache[25] end),
     [25] = mib.ConstCount(function () load_config() return icmp_scalar_cache[26] end),
     [26] = mib.ConstCount(function () load_config() return icmp_scalar_cache[27] end),
}

return icmpGroup
