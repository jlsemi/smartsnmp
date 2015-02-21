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
local utils = require "smartsnmp.utils"

local if_entry_cache = {}

local entry = {
    desc = "lo",
    type = 24,
    mtu = 65535,
    speed = 10000000,
    phy_addr = utils.mac2oct('aa:0f:16:3e:79:42'),
    admin_stat = 1,
    open_stat = 1,
    in_octet = 2449205,
    out_octet = 2449198,
    spec = { 0, 0 }
}
table.insert(if_entry_cache, entry)

entry = {
    desc = "eth0",
    type = 6,
    mtu = 1500,
    speed = 1000000,
    phy_addr = utils.mac2oct('00:1f:16:33:e7:21'),
    admin_stat = 1,
    open_stat = 1,
    in_octet = 672549159,
    out_octet = 672549138,
    spec = { 0, 0 }
}
table.insert(if_entry_cache, entry)

entry = {
    desc = "eth1",
    type = 6,
    mtu = 1500,
    speed = 100000000,
    phy_addr = utils.mac2oct('8c:ae:4c:fe:17:9c'),
    admin_stat = 1,
    open_stat = 1,
    in_octet = 4914346,
    out_octet = 4914345,
    spec = { 0, 0 }
}
table.insert(if_entry_cache, entry)

entry = {
    desc = "wlan0",
    type = 6,
    mtu = 1500,
    speed = 0,
    phy_addr = utils.mac2oct('00:26:c6:60:60:30'),
    admin_stat = 2,
    open_stat = 2,
    in_octet = 0,
    out_octet = 0,
    spec = { 0, 0 }
}
table.insert(if_entry_cache, entry)

entry = {
    desc = "virbr0",
    type = 6,
    mtu = 1500,
    speed = 0,
    phy_addr = utils.mac2oct('16:0a:80:74:ee:77'),
    admin_stat = 1,
    open_stat = 2,
    in_octet = 0,
    out_octet = 0,
    spec = { 0, 0 }
}
table.insert(if_entry_cache, entry)

local last_changed_time = os.time()

local function if_entry_get(i, name)
    assert(type(name) == 'string')
    local value
    if if_entry_cache[i] then
        if name == "" then
            value = i
        else
            value = if_entry_cache[i][name]
        end
    end
    return value
end

local function if_entry_set(i, v, name)
    assert(type(name) == 'string')
    if if_entry_cache[i] then
        if_entry_cache[i][name] = v
    end
end

local ifGroup = {
    [1]  = mib.ConstInt(function () return #if_entry_cache end),
    [2] = {
        [1] = {
            indexes = if_entry_cache,
            [1] = mib.ConstInt(function (i) return if_entry_get(i, '') end),
            [2] = mib.ConstOctString(function (i) return if_entry_get(i, 'desc') end),
            [3] = mib.ConstInt(function (i) return if_entry_get(i, 'type') end),
            [4] = mib.ConstInt(function (i) return if_entry_get(i, 'mtu') end),
            [5] = mib.ConstInt(function (i) return if_entry_get(i, 'speed') end),
            [6] = mib.ConstOctString(function (i) return if_entry_get(i, 'phy_addr') end),
            [7] = mib.Int(function (i) return if_entry_get(i, 'admin_stat') end, function (i, v) return if_entry_set(i, v, 'admin_stat') end),
            [8] = mib.ConstInt(function (i) return if_entry_get(i, 'open_stat') end),
            [9] = mib.ConstTimeticks(function (i)
                                         local time
                                         if if_entry_cache[i] then
                                             time =  os.difftime(os.time(), last_changed_time) * 100
                                         end
                                         return time
                                     end),
            [10] = mib.ConstInt(function (i) return if_entry_get(i, 'in_octet') end),
            [16] = mib.ConstInt(function (i) return if_entry_get(i, 'out_octet') end),
            [22] = mib.ConstOid(function (i) return if_entry_get(i, 'spec') end),
        }
    }
}

return ifGroup
