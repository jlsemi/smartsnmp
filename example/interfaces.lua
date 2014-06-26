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

ifndex = { 1, 2, 3, 4, 5 }

ifDescr = {
    'lo',
    'eth0',
    'eth1',
    'wlan0',
    'virbr0',
}

ifType = { 24, 6, 6, 6, 6 }

ifMtu = { 65535, 1500, 1500, 1500, 1500 }

ifSpeed = { 10000000, 100000000, 100000000, 0, 0 }

ifPhysAddress = {
    '',
    '001f1633e721',
    '8cae4cfe179c',
    '0026c6606030',
    '160a8074ee77',
}

ifAdminStatus = { 1, 1, 1, 2, 1 }

ifOpenStatus = { 1, 1, 1, 2, 2 }

local last_changed_time = 0

local function if_last_changed_time()
    return function() return os.difftime(os.time(), last_changed_time) * 100 end
end

function mib_interfaces_startup(time)
    last_changed_time = time
end

ifLastChange = {
    if_last_changed_time(),
    if_last_changed_time(),
    if_last_changed_time(),
    if_last_changed_time(),
    if_last_changed_time(),
}

ifInOctets = {
    2449205,
    672549159,
    4914346,
    0,
    0,
}

ifSpecific = {
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
}

mib_interfaces_startup(os.time())

ifGroup = {
    [1]  = mib.ConstInt(5),
    [2] = {
        [1] = {
            [1] = mib.ConstIntList(ifndex),
            [2] = mib.ConstStringList(ifDescr),
            [3] = mib.ConstIntList(ifType),
            [4] = mib.ConstIntList(ifMtu),
            [5] = mib.ConstIntList(ifSpeed),
            [6] = mib.ConstStringList(ifPhysAddress),
            [7] = mib.IntList(ifAdminStatus),
            [8] = mib.ConstIntList(ifOpenStatus),
            [9] = mib.ConstTimeticksList(ifLastChange),
            [22] = mib.ConstOidList(ifSpecific),
        }
    }
}

return ifGroup
