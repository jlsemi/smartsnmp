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

local ThreeIndexTableIdxA = 1
local ThreeIndexTableIdxB      = 2
local ThreeIndexTableIdxC     = 3

-- cascade multi-index
local three_dim_cache = {
    cascade = true,
    { 1, 3, 5, 7},
    { 2, 3 },
    { 32, 123, 87 },
}

local ThreeIndexTable = {
    [1] = {
        [1] = {
            indexes = three_dim_cache,
            [ThreeIndexTableIdxA] = mib.ConstInt(function (sub_oid)
                                                   for i, card_index in ipairs(three_dim_cache[1]) do
                                                       if sub_oid[1] == card_index then
                                                           return card_index
                                                       end
                                                   end
                                                   return nil
                                               end),
            [ThreeIndexTableIdxB] = mib.ConstInt(function (sub_oid)
                                              for i, cnu_index in ipairs(three_dim_cache[2]) do
                                                  if sub_oid[2] == cnu_index then
                                                      return cnu_index
                                                  end
                                              end
                                              return nil
                                          end),
            [ThreeIndexTableIdxC] = mib.ConstInt(function (sub_oid)
                                               for i, port_index in ipairs(three_dim_cache[3]) do
                                                   if sub_oid[3] == port_index then
                                                       return port_index
                                                   end
                                               end
                                               return nil
                                           end),
        }
    },
}

return ThreeIndexTable
