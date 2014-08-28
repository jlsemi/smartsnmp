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
    record,       -- some thing need to be recorded
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

    -- Empty.
    if next(it[dim]) == nil then
        return {}
    end

    record[dim] = record[dim] or {}

    if oid[offset] == nil then
        -- then point to first element
        oid = concat(oid, #oid + 1, it[dim][1])
        if dim == #it then
            return oid
        else
            record[dim].offset = offset
            record[dim].pos = 1
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
                record[dim].offset = offset
                record[dim].pos = i
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
                    record[dim].offset = offset
                    record[dim].pos = i
                    offset = offset + elem_len(xl[i])
                    dim = dim + 1
                    break
                end
            end
        end

        -- if didn't find anything
        if not found then
            local pos
            repeat
                if dim == 1 then
                    -- can't be recursive
                    return {}
                else
                    -- backtracking
                    dim = dim - 1
                    pos = record[dim].pos + 1
                end
            until pos <= #it[dim]
            offset = record[dim].offset
            for i = offset, #oid do
                oid[i] = nil
            end
            oid = concat(oid, offset, it[dim][pos])
        end
    end

    -- Tail recursion
    return getnext(oid, offset, record, it, dim)
end

-- index table iterator
local function oid_table_getnext(oid, it)
    return getnext(oid, 1, {}, it, 1)
end

return oid_table_getnext
