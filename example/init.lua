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

package.path = package.path .. ";src/lua/?.lua;example/?.lua"

mib = require "mib"
sysGroup = require "system"
ifGroup = require "interfaces"
ipGroup = require "ip"
tcpGroup = require "tcp"
udpGroup = require "udp"
mib_lib = require "mib_lib"

mib.SetRoCommunity('public')
mib.SetRwCommunity('private')

mib_group_register({1,3,6,1, 2,1, 1}, sysGroup, 'sysGroup')
mib_group_register({1,3,6,1, 2,1, 2}, ifGroup, 'ifGroup')
mib_group_register({1,3,6,1, 2,1, 4}, ipGroup, 'ipGroup')
mib_group_register({1,3,6,1, 2,1, 6}, tcpGroup, 'tcpGroup')
mib_group_register({1,3,6,1, 2,1, 7}, udpGroup, 'udpGroup')

--mib_group_unregister({1,3,6,1, 2,1, 1})
--mib_group_unregister({1,3,6,1, 2})
