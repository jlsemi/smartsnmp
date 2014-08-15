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

local udpInDatagrams  = 1
local udpNoPorts      = 2
local udpInErrors     = 3
local udpOutDatagrams = 4
local udpTable        = 5

local udpInDatagrams_  = 33954
local udpNoPorts_      = 751
local udpInErrors_     = 0
local udpOutDatagrams_ = 35207

local udp_cache = {}
local udp_index_cache = {}

local row = {
    loc_addr = {0,0,0,0},
    loc_port = 67
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 1)

row = {
    loc_addr = {0,0,0,0},
    loc_port = 161
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 2)

row = {
    loc_addr = {0,0,0,0},
    loc_port = 68
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 3)

row = {
    loc_addr = {0,0,0,0},
    loc_port = 5353
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 4)

row = {
    loc_addr = {0,0,0,0},
    loc_port = 44681
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 5)

row = {
    loc_addr = {0,0,0,0},
    loc_port = 51586
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 6)

row = {
    loc_addr = {127,0,0,1},
    loc_port = 53
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 7)

row = {
    loc_addr = {192,168,122,1},
    loc_port = 53
}
table.insert(udp_cache, row)
table.insert(udp_index_cache, 8)
mib.module_methods.or_table_reg("1.3.6.1.2.1.7", "The MIB module for managing UDP inplementations")

local udpGroup = {
    [udpInDatagrams]  = mib.ConstCount(function () return udpInDatagrams_ end),
    [udpNoPorts]      = mib.ConstCount(function () return udpNoPorts_ end),
    [udpInErrors]     = mib.ConstCount(function () return udpInErrors_ end),
    [udpOutDatagrams] = mib.ConstCount(function () return udpOutDatagrams_ end),
    [udpTable] = {
        [1] = {
            [1] = mib.ConstIndex(function () return udp_index_cache end),
            [2] = mib.ConstIpaddr(function (i) return udp_cache[i]['loc_addr'] end),
            [3] = mib.ConstInt(function (i) return udp_cache[i]['loc_port'] end),
        }
    }
}

return udpGroup
