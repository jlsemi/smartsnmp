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

local mib = require "mib"

ipAdEntAddr = {
    {10,2,12,164},
    {127,0,0,1},
    {192,168,1,4},
    {192,168,122,1},
}

ipAdEntIfIndex = {
    2,
    1,
    4,
    5,
}

ipAdEntNetMask = {
    {255,255,255,0},
    {255,0,0,0},
    {255,255,255,0},
    {255,255,255,0},
}

ipAdEntBcastAddr = {
    1,
    0,
    1,
    1,
}

ipRouteDest = {
    {0,0,0,0},
    {10,2,12,0},
    {169,254,0,0},
    {192,168,1,0},
    {192,168,122,0},
}

ipRouteIfIndex = {
    2,
    2,
    4,
    4,
    5,
}

ipNetToMediaPhysAddress = {
    '9c216ab06f3e',
}

ipNetToMediaNetAddress = {
    {10,2,12,1},
}

ipNetToMediaType = {
    3,
}

ipGroup = {
     [1]  = mib.Int(1),
     [2]  = mib.Int(64),
     [3]  = mib.ConstInt(669874),
     [4]  = mib.ConstInt(0),
     [5]  = mib.ConstInt(0),
     [6]  = mib.ConstInt(86),
     [7]  = mib.ConstInt(0),
     [8]  = mib.ConstInt(0),
     [9]  = mib.ConstInt(664487),
     [10] = mib.ConstInt(802147),
     [11] = mib.ConstInt(1),
     [12] = mib.ConstInt(1802),
     [13] = mib.ConstInt(30),
     [14] = mib.ConstInt(0),
     [15] = mib.ConstInt(0),
     [16] = mib.ConstInt(0),
     [17] = mib.ConstInt(0),
     [18] = mib.ConstInt(0),
     [19] = mib.ConstInt(0),
     [20] = {
         [1] = {
             [1] = mib.AutoIndex(4),
             [2] = mib.ConstIpaddrList(ipAdEntAddr),
             [3] = mib.ConstIntList(ipAdEntIfIndex),
             [4] = mib.ConstIpaddrList(ipAdEntNetMask),
             [5] = mib.ConstIntList(ipAdEntBcastAddr),
         }
     },
     [21] = {
         [1] = {
             [1] = mib.AutoIndex(5),
             [2] = mib.ConstIpaddrList(ipRouteDest),
             [3] = mib.IntList(ipRouteIfIndex),
         }
     },
     [22] = {
         [1] = {
             [1] = mib.AutoIndex(1),
             [2] = mib.StringList(ipNetToMediaPhysAddress),
             [3] = mib.ConstIpaddrList(ipNetToMediaNetAddress),
             [4] = mib.IntList(ipNetToMediaType),
         }
     },
     [23] = mib.ConstInt(0),
}

return ipGroup
