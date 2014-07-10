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

tcpConnState = { 5,6,8,5,5,11,6,2,11,5, }

tcpConnLocalAddress = {
    {127,0,0,1},
    {192,168,122,1},
    {127,0,0,1},
    {192,168,122,1},
    {127,0,0,1},
    {192,168,122,1},
    {127,0,0,1},
    {192,168,122,1},
    {127,0,0,1},
    {192,168,122,1},
}

tcpConnLocalPort = {
    67,
    68,
    161,
    5353,
    44681,
    51586,
    53,
    53,
    35194,
    35769,
}

tcpConnRemAddress = {
    {173,194,72,19},
    {180,149,134,53},
    {74,125,134,138},
    {74,125,136,100},
    {74,125,136,100},
    {74,125,136,100},
    {74,125,136,100},
    {74,125,136,100},
    {0,0,0,0},
    {0,0,0,0},
}

tcpConnRemPort = {
    80,
    80,
    80,
    443,
    80,
    80,
    80,
    80,
    443,
    80,
}

tcpGroup = {
    [1] = mib.ConstInt(1),
    [2] = mib.ConstInt(200),
    [3] = mib.ConstInt(120000),
    [4] = mib.ConstInt(-1),
    [5] = mib.ConstCount(19390),
    [6] = mib.ConstCount(0),
    [7] = mib.ConstCount(4058),
    [8] = mib.ConstCount(64),
    [9] = mib.ConstGauge(44),
    [10] = mib.ConstCount(380765),
    [11] = mib.ConstCount(384402),
    [12] = mib.ConstCount(37724),
    [13] = {
        [1] = {
            [1] = mib.IntList(tcpConnState),
            [2] = mib.ConstIpaddrList(tcpConnLocalAddress),
            [3] = mib.ConstIntList(tcpConnLocalPort),
            [4] = mib.ConstIpaddrList(tcpConnRemAddress),
            [5] = mib.ConstIntList(tcpConnRemPort),
        }
    },
    [14] = mib.ConstCount(3314),
    [15] = mib.ConstCount(825),
}

return tcpGroup
