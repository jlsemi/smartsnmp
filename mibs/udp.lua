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

udpInDatagrams  = 1
udpNoPorts      = 2
udpInErrors     = 3
udpOutDatagrams = 4
udpTable        = 5

udpLocalAddress = {
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {127,0,0,1},
    {192,168,122,1},
}

udpLocalPort = {
    67,
    68,
    161,
    5353,
    44681,
    51586,
    53,
    53,
}

udpGroup = {
    [udpInDatagrams]  = mib.ConstCount(33954),
    [udpNoPorts]      = mib.ConstCount(751),
    [udpInErrors]     = mib.ConstCount(0),
    [udpOutDatagrams] = mib.ConstCount(35207),
    [udpTable] = {
        [1] = {
            [1] = mib.ConstIpaddrList(udpLocalAddress),
            [2] = mib.ConstIntList(udpLocalPort),
        }
    }
}

return udpGroup
