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

local function getnext(
    oid,          -- request oid
    offset,       -- offset indicator of request oid
    last_offset,  -- offset indicator records
    it,           -- index table
    dim           -- offset dimension of *t*
)
    local elem_len = function(e)
        if (type(e) == 'table') then
            return #e
        else
            return 1
        end
    end
    local compare = function (oid, offset, e)
        if type(e) == 'number' then
            return oid[offset] - e
        elseif type(e) == 'table' then
            for i in ipairs(e) do
                local diff = oid[offset + i - 1] - e[i]
                if diff ~= 0 then return diff end
            end
            return (#oid - offset + 1) - #e
        end
    end
    local concat = function (oid, offset, e)
        for i = offset, #oid do
            oid[i] = nil
        end
        if type(e) == 'number' then
            table.insert(oid, e)
        else
            for i in ipairs(e) do
                table.insert(oid, e[i])
            end
        end
        return oid
    end

    -- Assertion
    assert(offset > 0 and dim > 0)

    -- Empty.
    if next(it[dim]) == nil then
        return {}
    end

    if oid[offset] == nil then
        -- then point to first element
        oid = concat(oid, #oid + 1, it[dim][1])
        if dim == #it then
            return oid
        else
            last_offset[dim] = offset
            offset = offset + elem_len(it[dim][1])
            dim = dim + 1
        end
    else
        local found = false

        xl = it[dim]
        assert(next(xl) ~= nil)
        for i, index in ipairs(xl) do
            -- if all match then return
            local cmp = compare(oid, offset, xl[i])
            if cmp == 0 and dim < #it then
                last_offset[dim] = offset
                offset = offset + elem_len(xl[i])
                dim = dim + 1
                found = true
                break
            -- if the request value is less than me, fetch the next one.
            elseif cmp < 0 then
                found = true
                -- set it to me
                oid = concat(oid, offset, xl[i])
                -- all dim found, return it
                if dim == #it then
                    return oid
                else
                    last_offset[dim] = offset
                    offset = offset + elem_len(xl[i])
                    dim = dim + 1
                    break
                end
            end
        end

        -- if didn't find anything
        if not found then
            -- if can't be recursive
            if dim == 1 then
                return {}
            else
                for i = offset, #oid do
                    oid[i] = nil
                end
                -- backtracking
                dim = dim - 1
                offset = last_offset[dim]
                oid[offset] = oid[offset] + 1
            end
        end
    end

    -- Tail recursion
    return getnext(oid, offset, last_offset, it, dim)
end

local function oid_table_getnext(oid, it)
    return getnext(oid, 1, {}, it, 1)
end

--------------------------------------------------------------------------------
-- unit tests
--[[
local test_matrix_find_next = function (t, #t, input, expect_out)
    local sinput = table.concat(input, '.')
    print("-------------------------------------")
    output = matrix_find_next(t, input, 1, #t)
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
]]
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

return oid_table_getnext
