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

local tcp_scalar_cache = {}
local tcp_conn_entry_cache = {}
--[[
    ["0.0.0.0.22.0.0.0.0.0"] = { conn_stat = 1 },
    ["10.2.12.229.33874.91.189.92.10.443"] = { conn_stat = 8 },
    ["10.2.12.229.33875.91.189.92.23.443"] = { conn_stat = 8 },
    ["10.2.12.229.37700.180.149.153.11.80"] = { conn_stat = 9 },
    ["10.2.12.229.46149.180.149.134.54.80"] = { conn_stat = 5 },
    ["10.2.12.229.53158.123.58.181.140.80"] = { conn_stat = 5 },
    ["127.0.0.1.631.0.0.0.0.0"] = { conn_stat = 1 },
]]

local tcp_snmp_conn_stat_map = {
    [0] = 12, -- ERROR
    5, -- ESTABLISHED
    3, -- SYN_SENT
    4, -- SYN_RECV
    6, -- FIN_WAIT1
    7, -- FIN_WAIT2
    11, -- TIME_WAIT
    1, -- CLOSED
    8, -- CLOSE_WAIT
    9, -- LAST_ACK
    2, -- LISTEN
    10 -- CLOSING
}

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
    assert(type(hex) == 'string')
    local num = {} 
    for i = #hex, 1, -2 do
        local msb = string.char(string.byte(hex, i - 1))
        local lsb = string.char(string.byte(hex, i))
        table.insert(num, hextab[msb] * 16 + hextab[lsb])
    end
    return table.concat(num, ".")
end

local function hex2num(hex)
    assert(type(hex) == 'string')
    local num = 0
    for i = 1, #hex do
        num = num * 16 + hextab[string.char(string.byte(hex, i))]
    end
    return num
end

local function __load_config()
    tcp_scalar_cache = {}
    tcp_conn_entry_cache = {}
    for line in io.lines("/proc/net/snmp") do
        if string.match(line, "%w+") == 'Tcp' then
            for w in string.gmatch(line, "%d+") do
                table.insert(tcp_scalar_cache, tonumber(w))
            end
        end
    end
    for line in io.lines("/proc/net/tcp") do
        local loc_addr = string.match(line, ".-:%s+(.-):")
        local loc_port = string.match(line, ".-:%s+.-:(.-)%s+")
        local rem_addr = string.match(line, ".-:.-:.-%s+(.-):")
        local rem_port = string.match(line, ".-:%s+.-:.-:(.-)%s+")
        local conn_stat = string.match(line, ".-:%s+.-:.-:.-%s+(.-)%s+")
        local key = {}
        if loc_addr ~= nil and loc_port ~= nil and rem_addr ~= nil and rem_port ~= nil and conn_stat ~= nil then
            table.insert(key, ip_hex2num(loc_addr))
            table.insert(key, tostring(hex2num(loc_port)))
            table.insert(key, ip_hex2num(rem_addr))
            table.insert(key, tostring(hex2num(rem_port)))
            conn_stat = hex2num(conn_stat)
            tcp_conn_entry_cache[table.concat(key, '.')] = {}
            tcp_conn_entry_cache[table.concat(key, '.')].conn_stat = tcp_snmp_conn_stat_map[conn_stat]
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

mib.module_methods.or_table_reg("1.3.6.1.2.1.6", "The MIB module for managing TCP inplementations")

local function tcp_conn_entry_get(sub_oid, name)
    assert(type(name) == 'string')
    local value
    if type(sub_oid) == 'table' then
        local key = table.concat(sub_oid, ".")
        if tcp_conn_entry_cache[key] then
            value = tcp_conn_entry_cache[key][name]
        end
    end
    return value
end

local function tcp_conn_entry_set(sub_oid, value, name)
    assert(type(name) == 'string')
    if type(sub_oid) == 'table' then
        local key = table.concat(sub_oid, ".")
        if tcp_conn_entry_cache[key] then
             tcp_conn_entry_cache[key][name] = value
        end
    end
end

local tcpGroup = {
    [1] = mib.ConstInt(function () load_config() return tcp_scalar_cache[1] end),
    [2] = mib.ConstInt(function () load_config() return tcp_scalar_cache[2] end),
    [3] = mib.ConstInt(function () load_config() return tcp_scalar_cache[3] end),
    [4] = mib.ConstInt(function () load_config() return tcp_scalar_cache[4] end),
    [5] = mib.ConstCount(function () load_config() return tcp_scalar_cache[5] end),
    [6] = mib.ConstCount(function () load_config() return tcp_scalar_cache[6] end),
    [7] = mib.ConstCount(function () load_config() return tcp_scalar_cache[7] end),
    [8] = mib.ConstCount(function () load_config() return tcp_scalar_cache[8] end),
    [9] = mib.ConstGauge(function () load_config() return tcp_scalar_cache[9] end),
    [10] = mib.ConstCount(function () load_config() return tcp_scalar_cache[10] end),
    [11] = mib.ConstCount(function () load_config() return tcp_scalar_cache[11] end),
    [12] = mib.ConstCount(function () load_config() return tcp_scalar_cache[12] end),
    [13] = {                                  
        [1] = {                               
            indexes = tcp_conn_entry_cache,
            [1] = mib.Int(function (sub_oid) load_config() return tcp_conn_entry_get(sub_oid, 'conn_stat') end,
                          function (sub_oid, value) load_config() return tcp_conn_entry_set(sub_oid, value, 'conn_stat') end),
            [2] = mib.ConstIpaddr(function (sub_oid)
                                      load_config() 
                                      local ipaddr
                                      if type(sub_oid) == 'table' and tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                          ipaddr = {}
                                          for i = 1, 4 do
                                              table.insert(ipaddr, sub_oid[i])
                                          end
                                      end
                                      return ipaddr
                                  end),
            [3] = mib.ConstInt(function (sub_oid)
                                  load_config() 
                                  if type(sub_oid) == 'table' and tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                      return sub_oid[5]
                                  else
                                      return nil
                                  end
                               end),
            [4] = mib.ConstIpaddr(function (sub_oid)
                                      load_config() 
                                      local ipaddr
                                      if type(sub_oid) == 'table' and tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                          ipaddr = {}
                                          for i = 6, 9 do
                                              table.insert(ipaddr, sub_oid[i])
                                          end
                                      end
                                      return ipaddr
                                  end),
            [5] = mib.ConstInt(function (sub_oid)
                                  load_config() 
                                  if type(sub_oid) == 'table' and tcp_conn_entry_cache[table.concat(sub_oid, ".")] then
                                      return sub_oid[10]
                                  else
                                      return nil
                                  end
                               end),
        }
    },
    [14] = mib.ConstCount(function () load_config() return tcp_scalar_cache[13] end),
    [15] = mib.ConstCount(function () load_config() return tcp_scalar_cache[14] end),
}

return tcpGroup
