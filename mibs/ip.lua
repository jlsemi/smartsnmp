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

local ipForwarding_ = 1
local ipDefalutTTL_ = 64
local ipInReceives_ = 669874
local ipInHdrErrors_ = 0
local ipInAddrErrors_ = 0
local ipForwDatagrams_ = 86
local ipInUnknownProtos_ = 0
local ipInDiscards_ = 0
local ipInDelivers_ = 664487
local ipOutRequests_ = 802147
local ipOutDiscards_ = 1
local ipOutNoRoutes_ = 1802
local ipReasmTimeout_ = 1024
local ipReasmReqds_ = 30
local ipReasmOKs = 0
local ipReasmFails = 0
local ipFragsOKs_ = 0
local ipFragsFails_ = 0
local ipFragsCreates_ = 0
local ipRoutingDiscards_ = 0

local ip_AdEnt_cache = {
    ["10.2.12.229"] = {
        mask = {255,255,255,0},
        bcast = 1,
    },
    ["127.0.0.1"] = {
        mask = {255,0,0,0},
        bcast = 0,
    }
}

local ip_RouteIf_cache = {
    ["10.2.12.0"] = {
        next_hop = { 10, 2, 12, 1 },
        mask = { 255, 255, 255, 255 },
    },
    ["0.0.0.0"] = {
        next_hop = { 0, 0, 0, 0 },
        mask = { 0, 0, 0, 0 },
    },
}

local ip_NetToMedia_cache = {
    ["2.10.2.12.1"] = { phyaddr = "0C-82-68-42-A0-A5", type = 3 },
    ["2.10.2.12.164"] = { phyaddr = "00-1B-77-7C-E5-7C", type = 3 },
}

mib.module_methods.or_table_reg("1.3.6.1.2.1.4", "The MIB module for managing IP and ICMP inplementations")

local function ip_AdEnt_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    local key = table.concat(sub_oid, ".")
    if ip_AdEnt_cache[key] then
        if name == "" then
            value = {}
            for i = 1, 4 do
                table.insert(value, sub_oid[i])
            end
        else
            value = ip_AdEnt_cache[key][name]
        end
    end
    return value
end

local function ip_RouteIf_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    local key = table.concat(sub_oid, ".")
    if ip_RouteIf_cache[key] then
        if name == "" then
            value = {}
            for i = 1, 4 do
                table.insert(value, sub_oid[i])
            end
        else
            value = ip_RouteIf_cache[key][name]
        end
    end
    return value
end

local function ip_RouteIf_entry_set(sub_oid, v, name)
    assert(type(name) == 'string')
    local key = table.concat(sub_oid, ".")
    if ip_RouteIf_cache[key] then
        if name == '' then
            ip_RouteIf_cache[table.concat(v, ".")] = ip_RouteIf_cache[key]
            ip_RouteIf_cache[key] = nil
        else
            ip_RouteIf_cache[key][name] = v
        end
    end
end

local function ip_NetToMedia_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    local key = table.concat(sub_oid, ".")
    if ip_NetToMedia_cache[i] then
        value = ip_NetToMedia_cache[key][name]
    end
    return value
end

local function ip_NetToMedia_entry_set(sub_oid, v, name)
    assert(type(name) == 'string')
    local key = table.concat(sub_oid, ".")
    if ip_NetToMedia_cache[key] then
        ip_NetToMedia_cache[key][name] = v
    end
end

local ipGroup = {
     rwcommunity = 'ipprivate',
     [1]  = mib.Int(function () return ipForwarding_ end, function (v) ipForwarding_ = v end),
     [2]  = mib.Int(function () return ipDefalutTTL_ end, function (v) ipDefalutTTL_ = v end),
     [3]  = mib.ConstInt(function () return ipInReceives_ end),
     [4]  = mib.ConstInt(function () return ipInHdrErrors_ end),
     [5]  = mib.ConstInt(function () return ipInAddrErrors_ end),
     [6]  = mib.ConstInt(function () return ipForwDatagrams_ end),
     [7]  = mib.ConstInt(function () return ipInUnknownProtos_ end),
     [8]  = mib.ConstInt(function () return ipInDiscards_ end),
     [9]  = mib.ConstInt(function () return ipInDelivers_ end),
     [10] = mib.ConstInt(function () return ipOutRequests_ end),
     [11] = mib.ConstInt(function () return ipOutDiscards_ end),
     [12] = mib.ConstInt(function () return ipOutNoRoutes_ end),
     [13] = mib.ConstInt(function () return ipReasmTimeout_ end),
     [14] = mib.ConstInt(function () return ipReasmReqds_ end),
     [15] = mib.ConstInt(function () return ipReasmOKs end),
     [16] = mib.ConstInt(function () return ipReasmFails end),
     [17] = mib.ConstInt(function () return ipFragsOKs_ end),
     [18] = mib.ConstInt(function () return ipFragsFails_ end),
     [19] = mib.ConstInt(function () return ipFragsCreates_ end),
     [20] = {
         [1] = {
             indexes = ip_AdEnt_cache,
             [1] = mib.ConstIpaddr(function (sub_oid) return ip_AdEnt_entry_get(sub_oid, '') end),
             [3] = mib.ConstIpaddr(function (sub_oid) return ip_AdEnt_entry_get(sub_oid, 'mask') end),
             [4] = mib.ConstInt(function (sub_oid) return ip_AdEnt_entry_get(sub_oid, 'bcast') end),
         }
     },
     [21] = {
         [1] = {
             indexes = ip_RouteIf_cache,
             [1] = mib.Ipaddr(function (sub_oid) return ip_RouteIf_entry_get(sub_oid, '') end,
                              function (sub_oid, value) return ip_RouteIf_entry_set(sub_oid, value, '') end),
             [7] = mib.Ipaddr(function (sub_oid) return ip_RouteIf_entry_get(sub_oid, 'next_hop') end,
                              function (sub_oid, value) return ip_RouteIf_entry_set(sub_oid, value, 'next_hop') end),
             [11] = mib.Ipaddr(function (sub_oid) return ip_RouteIf_entry_get(sub_oid, 'mask') end,
                               function (sub_oid, value) return ip_RouteIf_entry_set(sub_oid, value, 'mask') end),
         }
     },
     [22] = {
         [1] = {
             indexes = ip_NetToMedia_cache,
             [1] = mib.Int(function (sub_oid)
                               local index
                               if ip_NetToMedia_cache[table.concat(sub_oid, ".")] then
                                   index = sub_oid[1]
                               end
                               return index
                           end,
                           function (sub_oid, value)
                               local old = ip_NetToMedia_cache[table.concat(sub_oid, ".")]
                               if old then
                                   sub_oid[1] = value
                                   ip_NetToMedia_cache[table.concat(sub_oid, ".")] = old
                                   old = nil
                               end
                           end),
             [2] = mib.String(function (sub_oid) return ip_NetToMedia_entry_get(sub_oid, 'phyaddr') end,
                              function (sub_oid, value) return ip_NetToMedia_entry_set(sub_oid, value, 'phyaddr') end),
             [3] = mib.Ipaddr(function (sub_oid)
                                  local ipaddr
                                  if ip_NetToMedia_cache[table.concat(sub_oid, ".")] then
                                      ipaddr = {}
                                      for i = 2, 5 do
                                          table.insert(ipaddr, sub_oid[i])
                                      end
                                  end
                                  return ipaddr
                              end,
                              function (sub_oid, value)
                                  local old = ip_NetToMedia_cache[table.concat(sub_oid, ".")]
                                  if old then
                                      for i = 1, 4 do
                                          sub_oid[i + 1] = value[i]
                                      end
                                      ip_NetToMedia_cache[table.concat(sub_oid, ".")] = old
                                      old = nil
                                  end
                              end),
             [4] = mib.Int(function (sub_oid) return ip_NetToMedia_entry_get(sub_oid, 'type') end,
                           function (sub_oid, value) return ip_NetToMedia_entry_set(sub_oid, value, 'type') end),
         }
     },
     [23] = mib.ConstInt(function () return ipRoutingDiscards_ end),
}

return ipGroup
