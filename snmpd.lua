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
-- local mib = require "lualib.mib"
--

local snmpd = require "lualib.core"
local sysGroup = require "example.system"
local ifGroup = require "example.interfaces"
local ipGroup = require "example.ip"
local tcpGroup = require "example.tcp"
local udpGroup = require "example.udp"

local port = 161
snmpd.init(port)

mib.SetRoCommunity('public')
mib.SetRwCommunity('private')

mib.group_node_register({1,3,6,1, 2,1, 1}, sysGroup, 'sysGroup')
mib.group_node_register({1,3,6,1, 2,1, 2}, ifGroup, 'ifGroup')
mib.group_node_register({1,3,6,1, 2,1, 4}, ipGroup, 'ipGroup')
mib.group_node_register({1,3,6,1, 2,1, 6}, tcpGroup, 'tcpGroup')
mib.group_node_register({1,3,6,1, 2,1, 7}, udpGroup, 'udpGroup')

snmpd.run()

return snmpd
