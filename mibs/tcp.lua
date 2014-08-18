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

local tcpRtoAlgorithm_ = 1
local tcpRtoMin_ = 200
local tcpRtoMax_ = 120000
local tcpMaxConn_ = -1 
local tcpActiveOpens_ = 19390
local tcpPassiveOpens_ = 0
local tcpAttemptFails_ = 4058
local tcpEstabResets_ = 64
local tcpCurrEstab_ = 44
local tcpInSegs_ = 380765
local tcpOUtSegs_ = 384402
local tcpRetransSegs_ = 37724
local tcpInErrs_ = 3314
local tcpOutRsts = 825

local tcp_cache = {}
local tcp_index_cache = {}

local entry = {
    stat = 5,
    local_addr = {127,0,0,1},
    local_port = 35194,
    rem_addr = {173,194,72,19},
    rem_port = 80
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 1)

entry = {
    stat = 6,
    local_addr = {192,168,122,1},
    local_port = 67,
    rem_addr = {180,149,134,53},
    rem_port = 80
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 2)

entry = {
    stat = 8,
    local_addr = {127,0,0,1},
    local_port = 68,
    rem_addr = {74,125,134,138},
    rem_port = 51320
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 3)

entry = {
    stat = 5,
    local_addr = {192,168,122,1},
    local_port = 161,
    rem_addr = {74,125,136,100},
    rem_port = 80
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 4)

entry = {
    stat = 5,
    local_addr = {127,0,0,1},
    local_port = 5353,
    rem_addr = {74,125,136,100},
    rem_port = 443
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 5)

entry = {
    stat = 11,
    local_addr = {192,168,122,1},
    local_port = 44681,
    rem_addr = {74,125,136,100},
    rem_port = 80
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 6)

entry = {
    stat = 6,
    local_addr = {127,0,0,1},
    local_port = 51586,
    rem_addr = {74,125,136,100},
    rem_port = 80
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 7)

entry = {
    stat = 2,
    local_addr = {192,168,122,1},
    local_port = 53,
    rem_addr = {74,125,136,100},
    rem_port = 80
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 8)

entry = {
    stat = 11,
    local_addr = {127,0,0,1},
    local_port = 53,
    rem_addr = {0,0,0,0},
    rem_port = 443
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 9)

entry = {
    stat = 5,
    local_addr = {192,168,122,1},
    local_port = 35769,
    rem_addr = {0,0,0,0},
    rem_port = 80
}
table.insert(tcp_cache, entry)
table.insert(tcp_index_cache, 10)

mib.module_methods.or_table_reg("1.3.6.1.2.1.6", "The MIB module for managing TCP inplementations")

local tcpGroup = {
    [1] = mib.ConstInt(function () return tcpRtoAlgorithm_ end),
    [2] = mib.ConstInt(function () return tcpRtoMin_ end),
    [3] = mib.ConstInt(function () return tcpRtoMax_ end),
    [4] = mib.ConstInt(function () return tcpMaxConn_ end),
    [5] = mib.ConstCount(function () return tcpActiveOpens_ end),
    [6] = mib.ConstCount(function () return tcpPassiveOpens_ end),
    [7] = mib.ConstCount(function () return tcpAttemptFails_ end),
    [8] = mib.ConstCount(function () return tcpEstabResets_ end),
    [9] = mib.ConstGauge(function () return tcpCurrEstab_ end),
    [10] = mib.ConstCount(function () return tcpInSegs_ end),
    [11] = mib.ConstCount(function () return tcpOUtSegs_ end),
    [12] = mib.ConstCount(function () return tcpRetransSegs_ end),
    [13] = {
        [1] = {
            [1] = mib.ConstIndex(function () return tcp_index_cache end),
            [2] = mib.Int(function (i) return tcp_cache[i]['stat'] end, function (i, v) tcp_cache[i]['stat'] = v end),
            [3] = mib.ConstIpaddr(function (i) return tcp_cache[i]['local_addr'] end),
            [4] = mib.ConstInt(function (i) return tcp_cache[i]['local_port'] end),
            [5] = mib.ConstIpaddr(function (i) return tcp_cache[i]['rem_addr'] end),
            [6] = mib.ConstInt(function (i) return tcp_cache[i]['rem_port'] end),
        }
    },
    [14] = mib.ConstCount(function () return tcpInErrs_ end),
    [15] = mib.ConstCount(function () return tcpOutRsts_ end),
}

return tcpGroup
