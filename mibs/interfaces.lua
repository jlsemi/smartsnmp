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

local if_cache = {}
local if_index_cache = {}

local row = {}
row['desc'] = "lo"
row['type'] = 24
row['mtu'] = 65535
row['speed'] = 10000000
row['phy_addr'] = ''
row['admin_stat'] = 1
row['open_stat'] = 1
row['in_octet'] = 2449205
row['out_octet'] = 2449198
row['spec'] = { 0, 0 }
table.insert(if_cache, row)
table.insert(if_index_cache, 1)

row = {}
row['desc'] = "eth0"
row['type'] = 6
row['mtu'] = 1500
row['speed'] = 1000000
row['phy_addr'] = '001f1633e721'
row['admin_stat'] = 1
row['open_stat'] = 1
row['in_octet'] = 672549159
row['out_octet'] = 672549138
row['spec'] = { 0, 0 }
table.insert(if_cache, row)
table.insert(if_index_cache, 2)

row = {}
row['desc'] = "eth1"
row['type'] = 6
row['mtu'] = 1500
row['speed'] = 100000000
row['phy_addr'] = '8cae4cfe179c'
row['admin_stat'] = 1
row['open_stat'] = 1
row['in_octet'] = 4914346
row['out_octet'] = 4914345
row['spec'] = { 0, 0 }
table.insert(if_cache, row)
table.insert(if_index_cache, 3)

row = {}
row['desc'] = "wlan0"
row['type'] = 6
row['mtu'] = 1500
row['speed'] = 0
row['phy_addr'] = '0026c6606030'
row['admin_stat'] = 2
row['open_stat'] = 2
row['in_octet'] = 0
row['out_octet'] = 0
row['spec'] = { 0, 0 }
table.insert(if_cache, row)
table.insert(if_index_cache, 4)

row = {}
row['desc'] = "virbr0"
row['type'] = 6
row['mtu'] = 1500
row['speed'] = 0
row['phy_addr'] = '160a8074ee77'
row['admin_stat'] = 1
row['open_stat'] = 2
row['in_octet'] = 0
row['out_octet'] = 0
row['spec'] = { 0, 0 }
table.insert(if_cache, row)
table.insert(if_index_cache, 5)

local last_changed_time = os.time()

local ifGroup = {
    [1]  = mib.ConstInt(function () return #if_cache end),
    [2] = {
        [1] = {
            [1] = mib.ConstIndex(function () return if_index_cache end),
            [2] = mib.ConstString(function (i) return if_cache[i]['desc'] end),
            [3] = mib.ConstInt(function (i) return if_cache[i]['type'] end),
            [4] = mib.ConstInt(function (i) return if_cache[i]['mtu'] end),
            [5] = mib.ConstInt(function (i) return if_cache[i]['speed'] end),
            [6] = mib.ConstString(function (i) return if_cache[i]['phy_addr'] end),
            [7] = mib.Int(function (i) return if_cache[i]['admin_stat'] end, function (i, v) if_cache[i]['admin_stat'] = v end),
            [8] = mib.ConstInt(function (i) return if_cache[i]['open_stat'] end),
            [9] = mib.ConstTimeticks(function (i) return os.difftime(os.time(), last_changed_time) * 100 end),
            [10] = mib.ConstInt(function (i) return if_cache[i]['in_octet'] end),
            [16] = mib.ConstInt(function (i) return if_cache[i]['out_octet'] end),
            [22] = mib.ConstOid(function (i) return if_cache[i]['spec'] end),
        }
    }
}

return ifGroup
