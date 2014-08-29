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

local tcp_conn_entry_cache = {
    ["0.0.0.0.22.0.0.0.0.0"] = { stat = 1 },
    ["10.2.12.229.33874.91.189.92.10.443"] = { conn_stat = 8 },
    ["10.2.12.229.33875.91.189.92.23.443"] = { conn_stat = 8 },
    ["10.2.12.229.37700.180.149.153.11.80"] = { conn_stat = 9 },
    ["10.2.12.229.46149.180.149.134.54.80"] = { conn_stat = 5 },
    ["10.2.12.229.53158.123.58.181.140.80"] = { conn_stat = 5 },
    ["127.0.0.1.631.0.0.0.0.0"] = { conn_stat = 1 },
}

mib.module_methods.or_table_reg("1.3.6.1.2.1.6", "The MIB module for managing TCP inplementations")

local function tcp_conn_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    local key = table.concat(sub_oid, ".")
    if tcp_conn_entry_cache[key] then
        value = tcp_conn_entry_cache[key][name]
    end
    return value
end

local function tcp_conn_entry_set(sub_oid, value, name)
    assert(type(name) == 'string')
    local key = table.concat(sub_oid, ".")
    if tcp_conn_entry_cache[key] then
         tcp_conn_entry_cache[key][name] = value
    end
end

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
            indexes = tcp_conn_entry_cache,
            [1] = mib.Int(function (sub_oid) return tcp_conn_entry_get(sub_oid, 'conn_stat') end,
                          function (sub_oid, value) return tcp_conn_entry_set(sub_oid, value, 'conn_stat') end),
            [2] = mib.ConstIpaddr(function (sub_oid)
                                      local ipaddr
                                      if tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                          ipaddr = {}
                                          for i = 1, 4 do
                                              table.insert(ipaddr, sub_oid[i])
                                          end
                                      end
                                      return ipaddr
                                  end),
            [3] = mib.ConstInt(function (sub_oid)
                                  if tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                      return sub_oid[5]
                                  else
                                      return nil
                                  end
                               end),
            [4] = mib.ConstIpaddr(function (sub_oid)
                                      local ipaddr
                                      if tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                          ipaddr = {}
                                          for i = 6, 9 do
                                              table.insert(ipaddr, sub_oid[i])
                                          end
                                      end
                                      return ipaddr
                                  end),
            [5] = mib.ConstInt(function (sub_oid)
                                  if tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                      return sub_oid[10]
                                  else
                                      return nil
                                  end
                               end),
        }
    },
    [14] = mib.ConstCount(function () return tcpInErrs_ end),
    [15] = mib.ConstCount(function () return tcpOutRsts_ end),
}

return tcpGroup
