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

udpInDatagrams_  = 33954
udpNoPorts_      = 751
udpInErrors_     = 0
udpOutDatagrams_ = 35207


udpLocalAddress_ = {
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {127,0,0,1},
    {192,168,122,1},
}

udpLocalPort_ = {
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
    [udpInDatagrams]  = mib.ConstCount(function () return udpInDatagrams_ end),
    [udpNoPorts]      = mib.ConstCount(function () return udpNoPorts_ end),
    [udpInErrors]     = mib.ConstCount(function () return udpInErrors_ end),
    [udpOutDatagrams] = mib.ConstCount(function () return udpOutDatagrams_ end),
    [udpTable] = {
        [1] = {
            [1] = mib.AutoIndex(8),
            [2] = mib.ConstIpaddr(function (i) return udpLocalAddress_[i] end),
            [3] = mib.ConstInt(function (i) return udpLocalPort_[i] end),
        }
    }
}

return udpGroup
