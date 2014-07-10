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

-------------------------------------------------------------------------------
-- find next oid in a N-Dimensions table
--
-- use Lua's tail recursion feature, so it won't be stack overflow.
-------------------------------------------------------------------------------
matrix_find_next = function (
    t,          -- oid table, 2D array
    o,          -- request oid
    dim,        -- start dimension, should be start from 1
    max_dim     -- max dimension of *t*
)
    if next(t[dim]) == nil then return {} end

    if o[dim] == nil then
        -- then point to first element
        o[dim] = t[dim][1]
        if dim == max_dim then
            return o
        else
            dim = dim + 1
        end
    else
        local found = false
        -- for each
        xl = t[dim]
        for i, m in ipairs(xl) do
            if (i == 1 or xl[i-1] <= o[dim]) then
                -- if it all match then return
                if xl[i] == o[dim] and dim ~= max_dim then
                    dim = dim + 1
                    found = true
                    break
                -- if the request value is less then me, yes,
                -- the next is just me(`m`).
                elseif o[dim] < xl[i] then
                    found = true

                    -- set it to me
                    o[dim] = m

                    -- all dim found, return it
                    if dim == max_dim then
                        -- found it
                        return o
                    else
                        dim = dim + 1
                        -- clear all after this dim, cause all behind dim
                        -- should return its first.
                        for l = dim, max_dim do
                            o[l] = nil
                        end
                        break
                    end
                end
            end
        end

        -- if didn't find anything
        if not found then
            -- if can't be recursive
            if dim == 1 then
                return {}
            else
                o[dim] = nil
                dim = dim - 1
                o[dim] = o[dim] + 1
            end
        end
    end
    return matrix_find_next(t, o, dim, max_dim)
end

--------------------------------------------------------------------------------
-- unit tests

local test_matrix_find_next = function (t, max_dim, input, expect_out)
    local sinput = table.concat(input, '.')
    print("-------------------------------------")
    output = matrix_find_next(t, input, 1, max_dim)
    local soutput = table.concat(output, '.')
    local sexpout = table.concat(expect_out, '.')
    print(string.format(
        "IN :%-10s\nOUT:%-10s\nEXP:%-10s",
        sinput,
        soutput,
        sexpout
    ))
    return soutput == sexpout
end

--[[

-- test cases
local x = {}
x[1] = {1,2,5}
x[2] = {2,3,4,8}


assert(test_matrix_find_next(x, 2, {},           {1,2}           ))
assert(test_matrix_find_next(x, 2, {1},          {1,2}           ))
assert(test_matrix_find_next(x, 2, {2},          {2,2}           ))
assert(test_matrix_find_next(x, 2, {1,2},        {1,3}           ))
assert(test_matrix_find_next(x, 2, {1,8},        {2,2}           ))
assert(test_matrix_find_next(x, 2, {2,9},        {5,2}           ))
assert(test_matrix_find_next(x, 2, {5,2},        {5,3}           ))
assert(test_matrix_find_next(x, 2, {5,8},        {}              ))
assert(test_matrix_find_next(x, 2, {5,9},        {}              ))

local y = {}
y[1] = {1,2,5}
y[2] = {2,3,4,8}
y[3] = {2}
y[4] = {1,2,3,4,888}

assert(test_matrix_find_next(y, 4, {},           {1,2,2,1}       ))
assert(test_matrix_find_next(y, 4, {1},          {1,2,2,1}       ))
assert(test_matrix_find_next(y, 4, {2},          {2,2,2,1}       ))
assert(test_matrix_find_next(y, 4, {1,2},        {1,2,2,1}       ))
assert(test_matrix_find_next(y, 4, {1,8},        {1,8,2,1}       ))
assert(test_matrix_find_next(y, 4, {2,9},        {5,2,2,1}       ))
assert(test_matrix_find_next(y, 4, {5,2},        {5,2,2,1}       ))
assert(test_matrix_find_next(y, 4, {5,8},        {5,8,2,1}       ))
assert(test_matrix_find_next(y, 4, {5,9},        {}              ))
assert(test_matrix_find_next(y, 4, {1,2,2,1},    {1,2,2,2}       ))
assert(test_matrix_find_next(y, 4, {2,4,2,4},    {2,4,2,888}     ))
assert(test_matrix_find_next(y, 4, {2,4,2,888},  {2,8,2,1}       ))
assert(test_matrix_find_next(y, 4, {5,8,2,4},    {5,8,2,888}     ))
assert(test_matrix_find_next(y, 4, {5,8,2,888},  {}              ))
assert(test_matrix_find_next(y, 4, {2,5,1,8},    {2,8,2,1}       ))

]]--

return matrix_find_next
