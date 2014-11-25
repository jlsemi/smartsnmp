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

local udp_entry_cache = {}
local udp_scalar_cache = {}
--[[
    ["0.0.0.0.67"] = {},
    ["0.0.0.0.68"] = {},
    ["0.0.0.0.161"] = {},
    ["0.0.0.0.5353"] = {},
    ["0.0.0.0.44681"] = {},
    ["0.0.0.0.51586"] = {},
    ["127.0.0.1.53"] = {},
    ["192.168.122.1.53"] = {},
]]

local hextab = {
  ['0'] = 0,
  ['1'] = 1,
  ['2'] = 2,
  ['3'] = 3,
  ['4'] = 4,
  ['5'] = 5,
  ['6'] = 6,
  ['7'] = 7,
  ['8'] = 8,
  ['9'] = 9,
  ['A'] = 10,
  ['a'] = 10,
  ['B'] = 11,
  ['b'] = 11,
  ['C'] = 12,
  ['c'] = 12,
  ['D'] = 13,
  ['d'] = 13,
  ['E'] = 14,
  ['e'] = 14,
  ['F'] = 15,
  ['f'] = 15,
}

local function ip_hex2num(hex)
    assert(type(hex) == 'string' and #hex == 8)
    local num = {} 
    for i = 8, 2, -2 do
        local msb = string.char(string.byte(hex, i - 1))
        local lsb = string.char(string.byte(hex, i))
        table.insert(num, hextab[msb] * 16 + hextab[lsb])
    end
    return table.concat(num, ".")
end

local function port_hex2num(hex)
    assert(type(hex) == 'string' and #hex == 4)
    local num = 0
    for i = 1, 4 do
        num = num * 16 + hextab[string.char(string.byte(hex, i))]
    end
    return tostring(num)
end

local function __load_config()
    udp_scalar_cache = {}
    udp_entry_cache = {}
    for line in io.lines("/proc/net/snmp") do
        if string.match(line, "%w+") == 'Udp' then
            for w in string.gmatch(line, "%d+") do
                table.insert(udp_scalar_cache, tonumber(w))
            end
        end
    end
    for line in io.lines("/proc/net/udp") do
        local ipaddr = string.match(line, ".-:%s+(.-):")
        local port = string.match(line, ".-:%s+.-:(.-)%s+")
        if ipaddr ~= nil and port ~= nil then
            ipaddr = ip_hex2num(ipaddr)
            port = port_hex2num(port)
            udp_entry_cache[ipaddr .. '.' .. port] = true
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

mib.module_methods.or_table_reg("1.3.6.1.2.1.7", "The MIB module for managing UDP inplementations")

local udpGroup = {
    [udpInDatagrams]  = mib.ConstCount(function () load_config() return udp_scalar_cache[1] end),
    [udpNoPorts]      = mib.ConstCount(function () load_config() return udp_scalar_cache[2] end),
    [udpInErrors]     = mib.ConstCount(function () load_config() return udp_scalar_cache[3] end),
    [udpOutDatagrams] = mib.ConstCount(function () load_config() return udp_scalar_cache[4] end),
    [udpTable] = {
        [1] = {
            indexes = udp_entry_cache,
            [1] = mib.ConstIpaddr(function (sub_oid)
                                     load_config()
                                     local ipaddr
                                     if type(sub_oid) == 'table' and udp_entry_cache[table.concat(sub_oid, ".")] ~= nil then
                                         ipaddr = {}
                                         for i = 1, 4 do
                                             table.insert(ipaddr, sub_oid[i])
                                         end
                                     end
                                     return ipaddr
                                end),
            [2] = mib.ConstInt(function (sub_oid)
                                   load_config()
                                   local port
                                   if type(sub_oid) == 'table' and udp_entry_cache[table.concat(sub_oid, ".")] ~= nil then
                                       port = sub_oid[5]
                                   end
                                   return port
                               end),
        }
    }
}

return udpGroup
