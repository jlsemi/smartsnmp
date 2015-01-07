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

local core = require "smartsnmp.core"

local _M = {}
_M.core = core
_M._NAME = "smartsnmp"
_M._VERSION = "dev"

-- 
-- Constant Values for SNMP
--

-- Access
local MIB_ACES_UNA                   = 0
local MIB_ACES_RO                    = 1
local MIB_ACES_RW                    = 2
local MIB_ACES_RC                    = 2

-- SNMP request
local SNMP_REQ_GET                   = 0xA0
local SNMP_REQ_GETNEXT               = 0xA1
local SNMP_RESP                      = 0xA2
local SNMP_REQ_SET                   = 0xA3
local SNMP_REQ_GET_BLK               = 0xA5
local SNMP_REQ_INF                   = 0xA6
local SNMP_TRAP                      = 0xA7
local SNMP_REPO                      = 0xA8

-- ASN1 tag
local ASN1_TAG_BOOL                  = 0x01
local ASN1_TAG_INT                   = 0x02
local ASN1_TAG_BITSTR                = 0x03
local ASN1_TAG_OCTSTR                = 0x04
local ASN1_TAG_NUL                   = 0x05
local ASN1_TAG_OBJID                 = 0x06
local ASN1_TAG_SEQ                   = 0x30
local ASN1_TAG_IPADDR                = 0x40
local ASN1_TAG_CNT                   = 0x41
local ASN1_TAG_GAU                   = 0x42
local ASN1_TAG_TIMETICKS             = 0x43
local ASN1_TAG_OPAQ                  = 0x44
local ASN1_TAG_NO_SUCH_OBJ           = 0x80
local ASN1_TAG_NO_SUCH_INST          = 0x81

-- Error status
-- v1
_M.SNMP_ERR_STAT_NO_ERR              = 0
_M.SNMP_ERR_STAT_TOO_BIG             = 1
_M.SNMP_ERR_STAT_NO_SUCH_NAME        = 2
_M.SNMP_ERR_STAT_BAD_VALUE           = 3
_M.SNMP_ERR_STAT_READ_ONLY           = 4
_M.SNMP_ERR_STAT_GEN_ERR             = 5

-- v2c
_M.SNMP_ERR_STAT_UNACCESS            = 6
_M.SNMP_ERR_STAT_WRONG_TYPE          = 7
_M.SNMP_ERR_STAT_WRONG_LEN           = 8
_M.SNMP_ERR_STAT_ENCODING            = 9
_M.SNMP_ERR_STAT_WRONG_VALUE         = 10
_M.SNMP_ERR_STAT_NO_CREATION         = 11
_M.SNMP_ERR_STAT_INCONSISTENT_VALUE  = 12
_M.SNMP_ERR_STAT_RESOURCE_UNAVAIL    = 13
_M.SNMP_ERR_STAT_COMMIT_FAILED       = 14
_M.SNMP_ERR_STAT_UNDO_FAILED         = 15
_M.SNMP_ERR_STAT_AUTHORIZATION       = 16
_M.SNMP_ERR_STAT_NOT_WRITABLE        = 17
_M.SNMP_ERR_STAT_INCONSISTENT_NAME   = 18

--
-- Generators for declare SNMP MIB Node
--

_M.module_methods = {}

-- Submodule method register.
function _M.module_method_register(methods)
    for k, v in pairs(methods) do
        assert(type(k) == 'string' and type(v) == 'function', 'Methods must be [name, function] pairs')
        _M.module_methods[k] = v
    end
end

-- Submodule method unregister.
function _M.module_method_unregister(name)
    assert(type(name) == 'string', 'Method name must be string')
    _M.module_methods[name] = nil
end

-- Shell command invoke.
function _M.sh_call(command, rmode)
    if type(command) ~= 'string' or type(rmode) ~= 'string' then
        return nil
    end

    local t = nil
    local f = io.popen(command)
    if f then
        t = f:read(rmode)
        f:close()
    end
    return t
end

-- Bit String get/set function.
function _M.ConstBitString(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_BITSTR, access = MIB_ACES_RO, get_f = g }
end

function _M.BitString(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_BITSTR, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Octet String get/set function.
function _M.ConstOctString(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_OCTSTR, access = MIB_ACES_RO, get_f = g }
end

function _M.OctString(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_OCTSTR, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Integer get/set function.
function _M.ConstInt(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_INT, access = MIB_ACES_RO, get_f = g }
end

function _M.Int(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_INT, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Count get/set function.
function _M.ConstCount(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_CNT, access = MIB_ACES_RO, get_f = g }
end

function _M.Count(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_CNT, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- IP address get/set function.
function _M.ConstIpaddr(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_IPADDR, access = MIB_ACES_RO, get_f = g }
end

function _M.Ipaddr(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_IPADDR, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Oid get/set function for RO.
function _M.ConstOid(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_OBJID, access = MIB_ACES_RO, get_f = g }
end

function _M.Oid(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_OBJID, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Timeticks get/set function.
function _M.ConstTimeticks(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_TIMETICKS, access = MIB_ACES_RO, get_f = g }
end

function _M.Timeticks(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_TIMETICKS, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Gauge get/set function.
function _M.ConstGauge(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { tag = ASN1_TAG_GAU, access = MIB_ACES_RO, get_f = g }
end

function _M.Gauge(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { tag = ASN1_TAG_GAU, access = MIB_ACES_RW, get_f = g, set_f = s }
end

--
-- Helper functions
--

--[[
  It is supposed to generate an index container for a mib group node in which
  several scalar nodes next to each other make up a 2 dim matrix({{obj_no, ...},{0}} )
  while a table node as 4 dim matrix({{obj_no},{entry_no, ...},{var_no, ...},{inst_no, ...}}).
  e.x If we build up a group node like this:
    group = {
      [1] = scalar1
      [2] = scalar2
      [3] = scalar3
      [4] = {
        [1] = {
          [1] = index(1, 2, 3)
          [2] = ip(...)
          [3] = port(...)
        }
      }
      [5] = scalar4
      [6] = {
        [1] = {
          [1] = index(1, 2, 3, 4)
          [2] = mac(...)
        }
      }
      [7] = scalar5
      [8] = scalar6
    }
  then we will generate a group index container like this:
    index_table = {
      {{1,2,3},{0}},
      {{4},{1},{1,2,3},{1,2,3}},
      {{5},{0}},
      {{6},{1},{1,2},{1,2,3,4}},
      {{7,8},{0}},
    }

  Note:

  1. The 2nd dim(entry_no) in matrix of a table node representing entry index
  is assumed to be at most one id number element. If you want extra entries you
  may create extra revelant tables.

  2. The 4th dim and on(inst_no) in matrix of a table node representing instance
  index is assumed to be one id number each element instead of sequence of oids.
  Currently we don't support IP address as an instance index.
]]--

local group_index_table_generator = function (group, name)
    if type(group) ~= 'table' then error(string.format('Group should be container')) end
    if type(name) ~= 'string' then error(string.format('What is the group\'s name?')) end

    local group_indexes = {}  -- result to produce
    local scalar_indexes = {{},{0}}  -- 2 dimensions matrix
    local table_indexes = {}  -- N dimensions matrix
    local oid_cmp = function (oid1, oid2)
        if type(oid1) == 'number' and type(oid2) == 'number' then
            return oid1 < oid2
        elseif type(oid1) == 'table' and type(oid2) == 'table' then
            local len = math.min(#oid1, #oid2)
            for i = 1, len do
                if oid1[i] ~= oid2[i] then
                    return oid1[i] < oid2[i]
                end
            end
            return #oid1 < #oid2
        else
            error(string.format("Group \'%s\': Invalid element type in comparision", name))
        end
    end
    for obj_no in pairs(group) do

        if type(obj_no) == 'number' then
            if group[obj_no].get_f == nil then
                -- table
                table_indexes = {}
                local dim1 = { obj_no }
                table.insert(table_indexes, dim1)

                local tab = group[obj_no]
                if tab.get_f ~= nil then error(string.format('%s[%d]: Table should be container not variable', name, obj_no)) end
                -- For simplicity, we hold at most one entry each table.
                -- If you want multiple entries you may create revelant number of tables.
                if #tab > 1 then error(string.format('%s[%d]: Sorry but for simplicity, each table can hold one entry at most', name, obj_no)) end
                local entry_no, entry = next(tab)
                if type(entry_no) == 'number' then
                    local dim2 = { entry_no }
                    table.insert(table_indexes, dim2)
                end

                if entry ~= nil then
                    if entry.get_f ~= nil then error(string.format('%s[%d][%d]: Entry should be container not variable', name, obj_no, entry_no)) end
                    -- var_no
                    local dim3 = {}
                    for var_no in pairs(entry) do
                        if type(var_no) == 'number' then
                            table.insert(dim3, var_no)
                        end
                    end
                    table.insert(table_indexes, dim3)
                    table.sort(table_indexes[3])

                    -- indexes
                    if entry.indexes == nil then error(string.format("%s[%d][%d]: What is the entry.indexes?", name, obj_no, entry_no)) end
                    if type(entry.indexes) ~= 'table' then error(string.format("%s[%d][%d]: Entry indexes must be table", name, obj_no, entry_no)) end

                    if entry.indexes.cascade == true then
                        for _, indexes in ipairs(entry.indexes) do
                            table.sort(indexes)
                            table.insert(table_indexes, indexes)
                        end
                    else
                        if entry.indexes.cascade ~= nil then
                            error(string.format("%s[%d][%d]: No need to write \'cascade == false\' if indexes not cascaded, just wipe it out!", name, obj_no, entry_no))
                        end
                        local dim4 = {}
                        for key in pairs(entry.indexes) do
                            local index
                            -- index type
                            if type(key) == 'string' then
                                -- oid array
                                index = {}
                                for id in string.gmatch(key, "%d+") do
                                    table.insert(index, tonumber(id))
                                end
                            else
                                -- id number
                                index = key
                            end
                            table.insert(dim4, index)
                        end
                        table.sort(dim4, oid_cmp)
                        table.insert(table_indexes, dim4)
                    end
                end

                -- Insertion sort by table_no.
                local inserted = false
                for i, v in ipairs(group_indexes) do
                    local table_no = v[1][1]
                    local new_no = table_indexes[1][1]
                    if new_no < table_no then
                        table.insert(group_indexes, i, table_indexes)
                        inserted = true
                        break
                    end
                end
                if inserted == false then table.insert(group_indexes, table_indexes) end
            else
                -- Considering sorted traversal (with spanned key number) of a table is not supported in lua,
                -- we collect all the scalar indexes together here and seperate them later.
                table.insert(scalar_indexes[1], obj_no)
            end
        end
    end

    -- Seperate scalar_indexes and insert into group_indexes
    table.sort(scalar_indexes[1])
    local i = 1
    for _ in ipairs(group_indexes) do
        if i > #group_indexes then break end
        local tab_no = group_indexes[i][1][1]
        local tmp_indexes = {{},{0}}
        local j = 0
        for _, sc_no in ipairs(scalar_indexes[1]) do
            if sc_no > tab_no then break end
            table.insert(tmp_indexes[1], sc_no)
            j = j + 1
        end
        if j > 0 then
            table.insert(group_indexes, i, tmp_indexes)
            i = i + 1
            for k = 1, j do
                table.remove(scalar_indexes[1], 1)
            end
        end
        i = i + 1
    end
    if next(scalar_indexes[1]) then
        table.insert(group_indexes, scalar_indexes)
    end

    return group_indexes
end

-- Only called by group_index_table_getnext
local function getnext(
    oid,          -- request oid
    offset,       -- offset indicator of request oid
    record,       -- some thing need to be recorded
    it,           -- index table
    dim           -- offset dimension of *t*
)
    local elem_len = function(e)
        if type(e) == 'table' then
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

-- Group index table iterator
local function group_index_table_getnext(oid, it)
    return getnext(oid, 1, {}, it, 1)
end

local ber_tag_match = {
    [ASN1_TAG_BOOL] = { t = 'ASN1_TAG_BOOL', m = 'number' },
    [ASN1_TAG_INT] = { t = 'ASN1_TAG_INT', m = 'number' },
    [ASN1_TAG_BITSTR] = { t = 'ASN1_TAG_BITSTR', m = 'string' },
    [ASN1_TAG_OCTSTR] = { t = 'ASN1_TAG_OCTSTR', m = 'string' },
    [ASN1_TAG_NUL] = { t = 'ASN1_TAG_NUL', m = 'nil' },
    [ASN1_TAG_OBJID] = { t = 'ASN1_TAG_OBJID', m = 'table' },
    [ASN1_TAG_IPADDR] = { t = 'ASN1_TAG_IPADDR', m = 'table' },
    [ASN1_TAG_CNT] = { t = 'ASN1_TAG_CNT', m = 'number' },
    [ASN1_TAG_GAU] = { t = 'ASN1_TAG_GAU', m = 'number' },
    [ASN1_TAG_TIMETICKS] = { t = 'ASN1_TAG_TIMETICKS', m = 'number' },
    [ASN1_TAG_OPAQ] = { t = 'ASN1_TAG_OPAQ', m = 'number' },
}
 
local function return_value_check(g, v, t)
    if ber_tag_match[t] ~= nil then
        if ber_tag_match[t].m ~= type(v) then
            error(string.format("Group \'%s\' Tag \'%s\' but value is not \'%s\'", g, ber_tag_match[t].t, ber_tag_match[t].m))
        end
    else
        error(string.format("Group \'%s\' unknown tag: %d", g, t))
    end
end

-- Search and operation
local mib_node_search = function (group, name, op, req_sub_oid, req_val, req_val_type)
    local err_stat = nil
    local rsp_sub_oid = nil
    local rsp_val = nil
    local rsp_val_type = nil
    local group_index_table = nil
    -- Search obj_id in group index table.
    local effective_object_index = function (tab, id)
        for i in ipairs(tab) do
            for _, v in ipairs(tab[i][1]) do
                if id == v then return #tab[i] end
            end
        end
        return nil
    end

    local handlers = {}
    -- set operation
    handlers[SNMP_REQ_SET] = function ()
        rsp_sub_oid = req_sub_oid
        rsp_val = req_val
        rsp_val_type = req_val_type

        local obj_no = req_sub_oid[1]
        local dim = effective_object_index(group_index_table, obj_no)
        if dim ~= nil then
            if dim == 2 then
                -- scalar
                local scalar = group[obj_no]
                -- check access
                if scalar.access == MIB_ACES_UNA or not(#req_sub_oid == 2 and req_sub_oid[2] == 0) then
                    return _M.SNMP_ERR_STAT_UNACCESS, rsp_sub_oid, rsp_val, rsp_val_type
                end
                -- check type
                if rsp_val_type ~= scalar.tag then
                    return _M.SNMP_ERR_STAT_WRONG_TYPE, rsp_sub_oid, rsp_val, rsp_val_type
                end

                err_stat = scalar.set_f(rsp_val)
            elseif dim >= 4 then
                -- table
                local table_no = obj_no
                local entry_no = req_sub_oid[2]
                local var_no = req_sub_oid[3]
                local tab = group[table_no]
                if #req_sub_oid < 3 or tab[entry_no] == nil or tab[entry_no][var_no] == nil then
                    return _M.SNMP_ERR_STAT_UNACCESS, rsp_sub_oid, rsp_val, rsp_val_type
                end
                -- check access
                local variable = tab[entry_no][var_no]
                local inst_no
                if #rsp_sub_oid == 4 then
                    inst_no = rsp_sub_oid[4]
                else
                    inst_no = {}
                    for i = 4, #rsp_sub_oid do
                        table.insert(inst_no, rsp_sub_oid[i])
                    end
                end
                if variable.access == MIB_ACES_UNA or
                   type(inst_no) == 'number' and inst_no == nil or
                   type(inst_no) == 'table' and next(inst_no) == nil then
                    return _M.SNMP_ERR_STAT_UNACCESS, rsp_sub_oid, rsp_val, rsp_val_type
                end
                -- check type
                if rsp_val_type ~= variable.tag then
                    return _M.SNMP_ERR_STAT_WRONG_TYPE, rsp_sub_oid, rsp_val, rsp_val_type
                end

                err_stat = variable.set_f(inst_no, rsp_val)
            else
                return _M.SNMP_ERR_STAT_NOT_WRITABLE, rsp_sub_oid, rsp_val, rsp_val_type
            end
       else
           return _M.SNMP_ERR_STAT_NOT_WRITABLE, rsp_sub_oid, rsp_val, rsp_val_type
       end

        return_value_check(name, rsp_val, rsp_val_type)

        if err_stat ~= nil then
            return err_stat, rsp_sub_oid, rsp_val, rsp_val_type
        else
            return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, rsp_val, rsp_val_type
        end
    end

    -- get operation
    handlers[SNMP_REQ_GET] = function ()
        rsp_sub_oid = req_sub_oid

        local obj_no = req_sub_oid[1]
        local dim = effective_object_index(group_index_table, obj_no)
        if dim ~= nil then
            if dim == 2 then
                -- Scalar
                local scalar = group[obj_no]
                -- Check access
                if scalar.access == MIB_ACES_UNA then
                    return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_OBJ
                end
                -- Check existence
                if not(#req_sub_oid == 2 and req_sub_oid[2] == 0) then
                    return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_INST
                end
                rsp_val, err_stat = scalar.get_f()
                rsp_val_type = scalar.tag
            elseif dim >= 4 then
                -- table
                local table_no = obj_no
                local entry_no = req_sub_oid[2]
                local var_no = req_sub_oid[3]
                local tab = group[table_no]
                if #req_sub_oid < 3 or tab[entry_no] == nil or tab[entry_no][var_no] == nil then
                    return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_OBJ
                end
                -- Check access
                local variable = tab[entry_no][var_no]
                if variable.access == MIB_ACES_UNA then
                    return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_OBJ
                end
                -- Check instance existence
                local inst_no
                if #rsp_sub_oid == 4 then
                    inst_no = rsp_sub_oid[4]
                else
                    inst_no = {}
                    for i = 4, #rsp_sub_oid do
                        table.insert(inst_no, rsp_sub_oid[i])
                    end
                end
                if type(inst_no) == 'number' and inst_no == nil or
                   type(inst_no) == 'table' and next(inst_no) == nil then
                    return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_INST
                end
                -- Get instance value
                rsp_val, err_stat = variable.get_f(inst_no)
                rsp_val_type = variable.tag
            else
                return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_OBJ
            end
        else
            return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_OBJ
        end

        if rsp_val == nil or rsp_val_type == nil then
            return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_INST
        end

        return_value_check(name, rsp_val, rsp_val_type)

        if err_stat ~= nil then
            return err_stat, rsp_sub_oid, rsp_val, rsp_val_type
        else
            return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, rsp_val, rsp_val_type
        end
    end

    -- get next operation
    handlers[SNMP_REQ_GETNEXT] = function ()
        rsp_sub_oid = req_sub_oid

        local i = 1
        local variable = nil
        repeat
            if next(group_index_table) == nil then
                rsp_sub_oid = {}
                break
            end

            repeat
                rsp_sub_oid = group_index_table_getnext(rsp_sub_oid, group_index_table[i])
                if next(rsp_sub_oid) == nil then
                    i = i + 1
                    if i <= #group_index_table then rsp_sub_oid = req_sub_oid end
                else
                    break
                end
            until i > #group_index_table

            if next(rsp_sub_oid) == nil then
                -- not belong to this group
                break
            elseif #rsp_sub_oid == 2 then
                -- scalar
                local scalar_no = rsp_sub_oid[1]
                variable = group[scalar_no]
                rsp_val, err_stat = variable.get_f()
                rsp_val_type = variable.tag
            elseif #rsp_sub_oid >= 4 then
                -- table
                local table_no = rsp_sub_oid[1]
                local entry_no = rsp_sub_oid[2]
                local var_no  = rsp_sub_oid[3]
                variable = group[table_no][entry_no][var_no]
                -- inst_no
                local inst_no
                if #rsp_sub_oid == 4 then
                    inst_no = rsp_sub_oid[4]
                else
                    inst_no = {}
                    for i = 4, #rsp_sub_oid do
                        table.insert(inst_no, rsp_sub_oid[i])
                    end
                end
                -- get instance value
                rsp_val, err_stat = variable.get_f(inst_no)
                rsp_val_type = variable.tag

                if rsp_val == nil or rsp_val_type == nil or variable.access == MIB_ACES_UNA then
                    rsp_sub_oid[3] = rsp_sub_oid[3] + 1
                    rsp_sub_oid[4] = 0
                end
            else
                error(string.format('Group \'%s\' Neither a scalar variable nor a table', name))
            end
        -- Unaccessable node is ignored in getnext traversal.
        until rsp_val and rsp_val_type and variable.access ~= MIB_ACES_UNA

        if next(rsp_sub_oid) == nil then
            return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, nil, ASN1_TAG_NO_SUCH_OBJ
        end

        return_value_check(name, rsp_val, rsp_val_type)

        if err_stat ~= nil then
            return err_stat, rsp_sub_oid, rsp_val, rsp_val_type
        else
            return _M.SNMP_ERR_STAT_NO_ERR, rsp_sub_oid, rsp_val, rsp_val_type
        end
    end

    group_index_table = group_index_table_generator(group, name)
    H = handlers[op]
    return H()
end

--
-- User Interface
--

-- initialize snmp agent
_M.init = function (protocol, port)
    return core.init(protocol, port)
end

-- open snmp agent
_M.open = function ()
    return core.open()
end

-- start snmp agent
_M.start = function ()
    core.run()
end

-- set read only community
_M.set_ro_community = function (community, oid)
    assert(type(community) == 'string')
    if oid ~= nil then
        assert(type(oid) == 'table')
        core.mib_community_reg(oid, community, 1)
    else
        core.mib_community_reg({}, community, 1)
    end
end

-- set read/write community
_M.set_rw_community = function (community, oid)
    assert(type(community) == 'string')
    if oid ~= nil then
        assert(type(oid) == 'table')
        core.mib_community_reg(oid, community, 2)
    else
        core.mib_community_reg({}, community, 2)
    end
end

-- set read only user
_M.set_ro_user = function (user, oid)
    assert(type(user) == 'string')
    if oid ~= nil then
        assert(type(oid) == 'table')
        core.mib_user_reg(oid, user, 1)
    else
        core.mib_user_reg({}, user, 1)
    end
end

-- set read/write user
_M.set_rw_user = function (user, oid)
    assert(type(user) == 'string')
    if oid ~= nil then
        assert(type(oid) == 'table')
        core.mib_user_reg(oid, user, 2)
    else
        core.mib_user_reg({}, user, 2)
    end
end

-- register a group of snmp mib nodes
_M.register_mib_group = function (oid, group, name)
    local mib_search_handler = function (op, req_sub_oid, req_val, req_val_type)
        return mib_node_search(group, name, op, req_sub_oid, req_val, req_val_type)
    end
    core.mib_node_reg(oid, mib_search_handler)
end

-- unregister a group of snmp mib nodes
_M.unregister_mib_group = function(oid)
    core.mib_node_unreg(oid)
end

-- print group index table through generator
_M.group_index_table_check = function (group, name)
    local it = group_index_table_generator(group, name)
    local crashed = false

    print(string.format("Checking \'%s\' indexes table...", name))
    for i, v in ipairs(it) do
        local variable
        if #v == 2 then
            variable = 'scalar'
            print("scalar indexes:")
        else
            variable = 'table'
            print("table indexes:")
        end
        for i in ipairs(v) do
            print(string.format("\tDim%d:", i))
            if next(v[i]) then
                if type(v[i][1]) == 'number' then
                    if v[i] == nil or next(v[i]) == nil then
                        crashed = true
                        print(string.format("ERROR: \'%s\' %s indexes invalid!", name, variable))
                    else
                        print("\t", unpack(v[i]))
                    end
                else
                    for _, t in ipairs(v[i]) do
                        if t == nil or next(t) == nil then
                            crashed = true
                            print(string.format("ERROR: \'%s\' %s indexes invalid!", name, variable))
                        else
                            print("\t", unpack(t))
                        end
                    end
                end
            end
        end
    end

    if crashed == true then
        print(string.format("Oops! It seems something wrong in \'%s\' indexes!", name))
    else
        print(string.format("Group \'%s\' indexes are OK!", name))
    end
end

return _M
