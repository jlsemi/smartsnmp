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

local udpInDatagrams_  = 33954
local udpNoPorts_      = 751
local udpInErrors_     = 0
local udpOutDatagrams_ = 35207

local udpLocalAddress_ = {
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    {127,0,0,1},
    {192,168,122,1},
}

local udpLocalPort_ = {
    67,
    68,
    161,
    5353,
    44681,
    51586,
    53,
    53,
}

mib.module_methods.or_table_reg("1.3.6.1.2.1.7", "The MIB module for managing UDP inplementations")

local udpGroup = {
    [udpInDatagrams]  = mib.ConstCount(function () return udpInDatagrams_ end),
    [udpNoPorts]      = mib.ConstCount(function () return udpNoPorts_ end),
    [udpInErrors]     = mib.ConstCount(function () return udpInErrors_ end),
    [udpOutDatagrams] = mib.ConstCount(function () return udpOutDatagrams_ end),
    [udpTable] = {
        [1] = {
            [1] = mib.ConstIndex(function () return { 1, 2, 3, 4, 5, 6, 7, 8 } end),
            [2] = mib.ConstIpaddr(function (i) return udpLocalAddress_[i] end),
            [3] = mib.ConstInt(function (i) return udpLocalPort_[i] end),
        }
    }
}

return udpGroup
