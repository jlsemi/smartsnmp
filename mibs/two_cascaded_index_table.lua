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

local TwoIndexTableIdxA  = 1
local TwoIndexTableIdxB  = 2
local TwoIndexTableDescr = 3

-- cascade multi-index
local two_dim_entry_cache = {
    cascade = true,
    { 1, 2 },
    { 2, 3 },
}

local two_dim_cache = {
    [1] = {
        [2] = { desc = "A12" },
        [3] = { desc = "B13" },
    },
    [2] = {
        [2] = { desc = "C21" },
        [3] = { desc = "D22" },
    },
}

local TwoIndexTableGroup = {
    [1] = {
        [1] = {
            indexes = two_dim_entry_cache,
            [TwoIndexTableIdxA] = mib.ConstInt(function (sub_oid)
                                                         for i, card_index in ipairs(two_dim_entry_cache[1]) do
                                                             if sub_oid[1] == card_index then
                                                                 return card_index
                                                             end
                                                         end
                                                         return nil
                                                     end),
            [TwoIndexTableIdxB] = mib.ConstInt(function (sub_oid)
                                                     for i, mode_index in ipairs(two_dim_entry_cache[2]) do
                                                         if sub_oid[2] == mode_index then
                                                             return mode_index
                                                         end
                                                     end
                                                     return nil
                                                 end),
            [TwoIndexTableDescr] = mib.ConstOctString(function (sub_oid)
                                                        local i = sub_oid[1]
                                                        local j = sub_oid[2]
                                                        if two_dim_cache[i][j] then
                                                            return two_dim_cache[i][j].desc
                                                        end
                                                    end),
        }
    },
}

return TwoIndexTableGroup
