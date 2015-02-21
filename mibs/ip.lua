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

local ip_scalar_cache = {}

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
    ["2.10.2.12.1"] = { phyaddr = utils.mac2oct("0C:82:68:42:A0:A5"), type = 3 },
    ["2.10.2.12.164"] = { phyaddr = utils.mac2oct("00:1B:77:7C:E5:7C"), type = 3 },
}

local function __load_config()
    ip_scalar_cache = {}
    for line in io.lines("/proc/net/snmp") do
        if string.match(line, "%w+") == 'Ip' then
            for w in string.gmatch(line, "%d+") do
                table.insert(ip_scalar_cache, tonumber(w))
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

mib.module_methods.or_table_reg("1.3.6.1.2.1.4", "The MIB module for managing IP and ICMP inplementations")

local function ip_AdEnt_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    if type(sub_oid) == 'table' then
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
    end
    return value
end

local function ip_RouteIf_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    if type(sub_oid) == 'table' then
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
    end
    return value
end

local function ip_RouteIf_entry_set(sub_oid, v, name)
    assert(type(name) == 'string')
    if type(sub_oid) == 'table' then
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
end

local function ip_NetToMedia_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    if type(sub_oid) == 'table' then
        local key = table.concat(sub_oid, ".")
        if ip_NetToMedia_cache[key] then
            value = ip_NetToMedia_cache[key][name]
        end
    end
    return value
end

local function ip_NetToMedia_entry_set(sub_oid, v, name)
    assert(type(name) == 'string')
    if type(sub_oid) == 'table' then
        local key = table.concat(sub_oid, ".")
        if ip_NetToMedia_cache[key] then
            ip_NetToMedia_cache[key][name] = v
        end
    end
end

local ipGroup = {
    [1]  = mib.Int(function () load_config() return ip_scalar_cache[1] end, function (v) ip_scalar_cache[1] = v end),
    [2]  = mib.Int(function () load_config() return ip_scalar_cache[2] end, function (v) ip_scalar_cache[2] = v end),
    [3]  = mib.ConstInt(function () load_config() return ip_scalar_cache[3] end),
    [4]  = mib.ConstInt(function () load_config() return ip_scalar_cache[4] end),
    [5]  = mib.ConstInt(function () load_config() return ip_scalar_cache[5] end),
    [6]  = mib.ConstInt(function () load_config() return ip_scalar_cache[6] end),
    [7]  = mib.ConstInt(function () load_config() return ip_scalar_cache[7] end),
    [8]  = mib.ConstInt(function () load_config() return ip_scalar_cache[8] end),
    [9]  = mib.ConstInt(function () load_config() return ip_scalar_cache[9] end),
    [10] = mib.ConstInt(function () load_config() return ip_scalar_cache[10] end),
    [11] = mib.ConstInt(function () load_config() return ip_scalar_cache[11] end),
    [12] = mib.ConstInt(function () load_config() return ip_scalar_cache[12] end),
    [13] = mib.ConstInt(function () load_config() return ip_scalar_cache[13] end),
    [14] = mib.ConstInt(function () load_config() return ip_scalar_cache[14] end),
    [15] = mib.ConstInt(function () load_config() return ip_scalar_cache[15] end),
    [16] = mib.ConstInt(function () load_config() return ip_scalar_cache[16] end),
    [17] = mib.ConstInt(function () load_config() return ip_scalar_cache[17] end),
    [18] = mib.ConstInt(function () load_config() return ip_scalar_cache[18] end),
    [19] = mib.ConstInt(function () load_config() return ip_scalar_cache[19] end),
    [20] = {
        [1] = {
            indexes = ip_AdEnt_cache,
            [1] = mib.ConstIpaddr(function (sub_oid) load_config() return ip_AdEnt_entry_get(sub_oid, '') end),
            [3] = mib.ConstIpaddr(function (sub_oid) load_config() return ip_AdEnt_entry_get(sub_oid, 'mask') end),
            [4] = mib.ConstInt(function (sub_oid) load_config() return ip_AdEnt_entry_get(sub_oid, 'bcast') end),
        }
    },
    [21] = {
        [1] = {
            indexes = ip_RouteIf_cache,
            [1] = mib.Ipaddr(function (sub_oid) load_config() return ip_RouteIf_entry_get(sub_oid, '') end,
                             function (sub_oid, value) load_config() return ip_RouteIf_entry_set(sub_oid, value, '') end),
            [7] = mib.Ipaddr(function (sub_oid) load_config() return ip_RouteIf_entry_get(sub_oid, 'next_hop') end,
                             function (sub_oid, value) load_config() return ip_RouteIf_entry_set(sub_oid, value, 'next_hop') end),
            [11] = mib.Ipaddr(function (sub_oid) load_config() return ip_RouteIf_entry_get(sub_oid, 'mask') end,
                              function (sub_oid, value) load_config() return ip_RouteIf_entry_set(sub_oid, value, 'mask') end),
        }
    },
    [22] = {
        [1] = {
            indexes = ip_NetToMedia_cache,
            [1] = mib.Int(function (sub_oid)
                              load_config()
                              local index
                              if type(sub_oid) == 'table' then
                                  if ip_NetToMedia_cache[table.concat(sub_oid, ".")] then
                                      index = sub_oid[1]
                                  end
                              end
                              return index
                          end,
                          function (sub_oid, value)
                              load_config()
                              if type(sub_oid) == 'table' then
                                  local old = ip_NetToMedia_cache[table.concat(sub_oid, ".")]
                                  if old then
                                      sub_oid[1] = value
                                      ip_NetToMedia_cache[table.concat(sub_oid, ".")] = old
                                      old = nil
                                  end
                              end
                          end),
            [2] = mib.OctString(function (sub_oid) load_config() return ip_NetToMedia_entry_get(sub_oid, 'phyaddr') end,
                             function (sub_oid, value) load_config() return ip_NetToMedia_entry_set(sub_oid, value, 'phyaddr') end),
            [3] = mib.Ipaddr(function (sub_oid)
                                 load_config()
                                 local ipaddr
                                 if type(sub_oid) == 'table' then
                                     if ip_NetToMedia_cache[table.concat(sub_oid, ".")] then
                                         ipaddr = {}
                                         for i = 2, 5 do
                                             table.insert(ipaddr, sub_oid[i])
                                         end
                                     end
                                 end
                                 return ipaddr
                             end,
                             function (sub_oid, value)
                                 load_config()
                                 if type(sub_oid) == 'table' then
                                     local old = ip_NetToMedia_cache[table.concat(sub_oid, ".")]
                                     if old then
                                         for i = 1, 4 do
                                             sub_oid[i + 1] = value[i]
                                         end
                                         ip_NetToMedia_cache[table.concat(sub_oid, ".")] = old
                                         old = nil
                                     end
                                 end
                             end),
            [4] = mib.Int(function (sub_oid) load_config() return ip_NetToMedia_entry_get(sub_oid, 'type') end,
                          function (sub_oid, value) load_config() return ip_NetToMedia_entry_set(sub_oid, value, 'type') end),
        }
    },
    [23] = mib.ConstInt(function () load_config() return 0 end),
}

return ipGroup
