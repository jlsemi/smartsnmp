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

ipForwarding_ = 1
ipDefalutTTL_ = 64
ipInReceives_ = 669874
ipInHdrErrors_ = 0
ipInAddrErrors_ = 0
ipForwDatagrams_ = 86
ipInUnknownProtos_ = 0
ipInDiscards_ = 0
ipInDelivers_ = 664487
ipOutRequests_ = 802147
ipOutDiscards_ = 1
ipOutNoRoutes_ = 1802
ipReasmTimeout_ = 1024
ipReasmReqds_ = 30
ipReasmOKs = 0
ipReasmFails = 0
ipFragsOKs_ = 0
ipFragsFails_ = 0
ipFragsCreates_ = 0

ipAdEntAddr_ = {
    {10,2,12,164},
    {127,0,0,1},
    {192,168,1,4},
    {192,168,122,1},
}

ipAdEntIfIndex_ = {
    2,
    1,
    4,
    5,
}

ipAdEntNetMask_ = {
    {255,255,255,0},
    {255,0,0,0},
    {255,255,255,0},
    {255,255,255,0},
}

ipAdEntBcastAddr_ = {
    1,
    0,
    1,
    1,
}

ipRouteDest_ = {
    {0,0,0,0},
    {10,2,12,0},
    {169,254,0,0},
    {192,168,1,0},
    {192,168,122,0},
}

ipRouteIfIndex_ = {
    2,
    2,
    4,
    4,
    5,
}

ipNetToMediaPhysAddress_ = {
    '9c216ab06f3e',
}

ipNetToMediaNetAddress_ = {
    {10,2,12,1},
}

ipNetToMediaType_ = {
    3,
}

ipRoutingDiscards_ = 0

ipGroup = {
     rwcommunity = 'ipprivate',
     [1]  = mib.Int(function () return ipForwarding_ end, function (v) ipForwarding_ = v end),
     [2]  = mib.Int(function () return ipDefalutTTL_ end, function (v) ipDefalutTTL_ = v end),
     [3]  = mib.ConstInt(function () return ipInReceives_ end),
     [4]  = mib.ConstInt(function () return ipInHdrErrors_ end),
     [5]  = mib.ConstInt(function () return ipInAddrErrors_ end),
     [6]  = mib.ConstInt(function () return ipForwDatagrams_ end),
     [7]  = mib.ConstInt(function () return ipInUnknownProtos_ end),
     [8]  = mib.ConstInt(function () return ipInDiscards_ end),
     [9]  = mib.ConstInt(function () return ipInDelivers_ end),
     [10] = mib.ConstInt(function () return ipOutRequests_ end),
     [11] = mib.ConstInt(function () return ipOutDiscards_ end),
     [12] = mib.ConstInt(function () return ipOutNoRoutes_ end),
     [13] = mib.ConstInt(function () return ipReasmTimeout_ end),
     [14] = mib.ConstInt(function () return ipReasmReqds_ end),
     [15] = mib.ConstInt(function () return ipReasmOKs end),
     [16] = mib.ConstInt(function () return ipReasmFails end),
     [17] = mib.ConstInt(function () return ipFragsOKs_ end),
     [18] = mib.ConstInt(function () return ipFragsFails_ end),
     [19] = mib.ConstInt(function () return ipFragsCreates_ end),
     [20] = {
         [1] = {
             [1] = mib.AutoIndex(4),
             [2] = mib.ConstIpaddr(function (i) return ipAdEntAddr_[i] end),
             [3] = mib.ConstInt(function (i) return ipAdEntIfIndex_[i] end),
             [4] = mib.ConstIpaddr(function (i) return ipAdEntNetMask_[i] end),
             [5] = mib.ConstInt(function (i) return ipAdEntBcastAddr_[i] end),
         }
     },
     [21] = {
         [1] = {
             [1] = mib.AutoIndex(5),
             [2] = mib.ConstIpaddr(function (i) return ipRouteDest_[i] end),
             [3] = mib.Int(function (i) return ipRouteIfIndex_[i] end, function (i, v) ipRouteIfIndex_[i] = v end),
         }
     },
     [22] = {
         [1] = {
             [1] = mib.AutoIndex(1),
             [2] = mib.String(function (i) return ipNetToMediaPhysAddress_[i] end, function (i, v) ipNetToMediaPhysAddress_[i] = v end),
             [3] = mib.ConstIpaddr(function (i) return ipNetToMediaNetAddress_[i] end),
             [4] = mib.Int(function (i) return ipNetToMediaType_[i] end, function (i, v) ipNetToMediaType_[i] = v end),
         }
     },
     [23] = mib.ConstInt(function () return ipRoutingDiscards_ end),
}

return ipGroup
