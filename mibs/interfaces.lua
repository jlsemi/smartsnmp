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

local ifNumber_ = 5
local ifndex_ = { 1, 2, 3, 4, 5 }
local ifDescr_ = {
    'lo',
    'eth0',
    'eth1',
    'wlan0',
    'virbr0',
}
local ifType_ = { 24, 6, 6, 6, 6 }
local ifMtu_ = { 65535, 1500, 1500, 1500, 1500 }
local ifSpeed_ = { 10000000, 100000000, 100000000, 0, 0 }
local ifPhysAddress_ = {
    '',
    '001f1633e721',
    '8cae4cfe179c',
    '0026c6606030',
    '160a8074ee77',
}
local ifAdminStatus_ = { 1, 1, 1, 2, 1 }
local ifOpenStatus_ = { 1, 1, 1, 2, 2 }
local last_changed_time = 0
local function if_last_changed_time()
    return function() return os.difftime(os.time(), last_changed_time) * 100 end
end
local function mib_interfaces_startup(time)
    last_changed_time = time
end
local ifLastChange_ = {
    if_last_changed_time(),
    if_last_changed_time(),
    if_last_changed_time(),
    if_last_changed_time(),
    if_last_changed_time(),
}

local ifInOctets_ = {
    2449205,
    672549159,
    4914346,
    0,
    0,
}
local ifSpecific_ = {
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
}

mib_interfaces_startup(os.time())

local ifGroup = {
    [1]  = mib.ConstInt(function () return ifNumber_ end),
    [2] = {
        [1] = {
            [1] = mib.ConstIndex(function () return ifndex_ end),
            [2] = mib.ConstString(function (i) return ifDescr_[i] end),
            [3] = mib.ConstInt(function (i) return ifType_[i] end),
            [4] = mib.ConstInt(function (i) return ifMtu_[i] end),
            [5] = mib.ConstInt(function (i) return ifSpeed_[i] end),
            [6] = mib.ConstString(function (i) return ifPhysAddress_[i] end),
            [7] = mib.Int(function (i) return ifAdminStatus_[i] end, function (i, v) ifAdminStatus_[i] = v end),
            [8] = mib.ConstInt(function (i) return ifOpenStatus_[i] end),
            [9] = mib.ConstTimeticks(function (i) return ifLastChange_[i] end),
            [22] = mib.ConstOid(function (i) return ifSpecific_[i] end),
        }
    }
}

return ifGroup
