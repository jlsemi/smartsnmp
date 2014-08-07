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
local matrix_find_next = require "smartsnmp.find_next"

local _M = {}
_M.core = core
_M._NAME = "smartsnmp"
_M._VERSION = "dev"

-- 
-- Constant Values for SNMP
--

-- Access
MIB_ACES_UNA        = 0
MIB_ACES_RO         = 1
MIB_ACES_RW         = 2
MIB_ACES_RC         = 2

-- SNMP request
SNMP_REQ_GET        = 0xA0
SNMP_REQ_GETNEXT    = 0xA1
SNMP_RESP           = 0xA2
SNMP_REQ_SET        = 0xA3
SNMP_REQ_GET_BLK    = 0xA5
SNMP_REQ_INF        = 0xA6
SNMP_TRAP           = 0xA7
SNMP_REPO           = 0xA8

-- BER tag
BER_TAG_BOOL              = 0x01
BER_TAG_INT               = 0x02
BER_TAG_BITSTR            = 0x03
BER_TAG_OCTSTR            = 0x04
BER_TAG_NUL               = 0x05
BER_TAG_OBJID             = 0x06
BER_TAG_SEQ               = 0x30
BER_TAG_IPADDR            = 0x40
BER_TAG_CNT               = 0x41
BER_TAG_GAU               = 0x42
BER_TAG_TIMETICKS         = 0x43
BER_TAG_OPAQ              = 0x44
BER_TAG_NO_SUCH_OBJ       = 0x80
BER_TAG_NO_SUCH_INST      = 0x81

-- Error status
-- v1
SNMP_ERR_STAT_NO_ERR              = 0
SNMP_ERR_STAT_TOO_BIG             = 1
SNMP_ERR_STAT_NO_SUCH_NAME        = 2
SNMP_ERR_STAT_BAD_VALUE           = 3
SNMP_ERR_STAT_READ_ONLY           = 4
SNMP_ERR_STAT_GEN_ERR             = 5

-- v2c
SNMP_ERR_STAT_ON_ACCESS           = 6
SNMP_ERR_STAT_WRONG_TYPE          = 7
SNMP_ERR_STAT_WRONG_LEN           = 8
SNMP_ERR_STAT_ENCODING            = 9
SNMP_ERR_STAT_WRONG_VALUE         = 10
SNMP_ERR_STAT_NO_CREATION         = 11
SNMP_ERR_STAT_INCONSISTENT_VALUE  = 12
SNMP_ERR_STAT_RESOURCE_UNAVAIL    = 13
SNMP_ERR_STAT_COMMIT_FAILED       = 14
SNMP_ERR_STAT_UNDO_FAILED         = 15
SNMP_ERR_STAT_AUTHORIZATION       = 16
SNMP_ERR_STAT_NOT_WRITABLE        = 17
SNMP_ERR_STAT_INCONSISTENT_NAME   = 18

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
function _M.sh_call(command)
    if type(command) ~= 'string' then
        return nil
    end

    local t = nil
    local f = io.popen(command)
    if f ~= nil then
        t = f:read("*line")
        f:close()
    end
    return t
end

-- String get/set function.
function _M.ConstString(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = false, tag = BER_TAG_OCTSTR, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

-- String get/set function.
function _M.String(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { index_key = false, tag = BER_TAG_OCTSTR, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Integer get/set function.
function _M.ConstInt(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = false, tag = BER_TAG_INT, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

-- Integer get/set function.
function _M.Int(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { index_key = false, tag = BER_TAG_INT, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Count get/set function.
function _M.ConstCount(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = false, tag = BER_TAG_CNT, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

-- Count get/set function.
function _M.Count(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { index_key = false, tag = BER_TAG_CNT, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- IP address get/set function.
function _M.ConstIpaddr(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = false, tag = BER_TAG_IPADDR, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

-- IP address get/set function.
function _M.Ipaddr(ip)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { index_key = false, tag = BER_TAG_IPADDR, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Oid get/set function for RO.
function _M.ConstOid(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = false, tag = BER_TAG_OBJID, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

-- Oid get/set function.
function _M.Oid(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { index_key = false, tag = BER_TAG_OBJID, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Timeticks get/set function.
function _M.ConstTimeticks(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = false, tag = BER_TAG_TIMETICKS, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

-- Timeticks get/set function.
function _M.Timeticks(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { index_key = false, tag = BER_TAG_TIMETICKS, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Gauge get/set function.
function _M.ConstGauge(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = false, tag = BER_TAG_GAU, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

-- Gauge get/set function.
function _M.Gauge(g, s)
    assert(type(g) == 'function' and type(s) == 'function', 'Arguments must be function type')
    return { index_key = false, tag = BER_TAG_GAU, access = MIB_ACES_RW, get_f = g, set_f = s }
end

-- Index get/set function
function _M.UnaIndex(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = true, tag = BER_TAG_INT, access = MIB_ACES_UNA, get_f = g, set_f = nil }
end

-- Index get/set function
function _M.ConstIndex(g)
    assert(type(g) == 'function', 'Argument must be function type')
    return { index_key = true, tag = BER_TAG_INT, access = MIB_ACES_RO, get_f = g, set_f = nil }
end

--
-- Helper functions
--

--[[
  It is supposed to generate an index container for a mib group node in which
  several scalar nodes next to each other make up a 2 dim matrix({{obj_no, ...},{0}} )
  while a table node as 4 dim matrix({{obj_no},{entry_no, ...},{list_no, ...},{inst_no, ...}}).
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

-- For debug print
local check_group_index_table = function (it)
    for i, v in ipairs(it) do
        for i in ipairs(v) do print(unpack(v[i])) end
    end
end

local group_index_table_generator = function (group, name)
    assert(type(group) == 'table', string.format('Group should be container'))
    assert(type(name) == 'string', string.format('What is the group\'s name?'))

    local group_indexes = {}  -- result to produce
    local scalar_indexes = {{},{0}}  -- 2 dimensions matrix
    local table_indexes = {}  -- N dimensions matrix

    for obj_no in pairs(group) do

        if type(obj_no) == 'number' then
            if group[obj_no].get_f == nil then
                -- table
                table_indexes = {}
                local dim1 = { obj_no }
                table.insert(table_indexes, dim1)

                local tab = group[obj_no]
                assert(tab.get_f == nil, string.format('%s[%d]: Table should be container not variable', name, obj_no))
                -- For simplicity, we hold at most one entry each table.
                -- If you want multiple entries you may create revelant number of tables.
                assert(#tab <= 1, string.format('%s[%d]: Sorry but for simplicity, each table can hold one entry at most', name, obj_no))
                local entry_no, entry = next(tab)
                assert(entry.get_f == nil, string.format('%s[%d][%d]: Entry should be container not variable', name, obj_no, entry_no))
                if type(entry_no) == 'number' then
                    local dim2 = { entry_no }
                    table.insert(table_indexes, dim2)
                end

                if entry ~= nil then
                    -- list_no
                    local dim3 = {}
                    for list_no in pairs(entry) do
                        if type(list_no) == 'number' then
                            table.insert(dim3, list_no)
                        end
                    end
                    table.insert(table_indexes, dim3)

                    -- index list
                    for list_no, list in ipairs(entry) do
                        if list.index_key == true then
                            -- instance
                            local dimN = {}
                            local it = list.get_f()
                            assert(type(it) == 'table', string.format('%s[%d][%d][%d]: Index list must be table', name, obj_no, entry_no, list_no))
                            for _, inst_no in pairs(it) do
                                if type(inst_no) == 'number' then
                                    table.insert(dimN, inst_no)
                                end
                            end
                            table.insert(table_indexes, dimN)
                        end
                    end
                end

                -- Sort group index table
                for i in ipairs(table_indexes) do
                    table.sort(table_indexes[i])
                end

                -- Insertion sort.
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
    if next(scalar_indexes[1]) ~= nil then
        table.insert(group_indexes, scalar_indexes)
    end

    return group_indexes
end

-- Search and operation
local mib_node_search = function (group, name, op, community, req_sub_oid, req_val, req_val_type)
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

        -- Priority for local group community string
        if group.rwcommunity ~= community then
            -- Global community
            if _M.rwcommunity ~= nil and _M.rwcommunity ~= '' and _M.rwcommunity ~= community then
                return SNMP_ERR_STAT_AUTHORIZATION, rsp_sub_oid, rsp_val, rsp_val_type
            end
            -- Local community
            if group.rwcommunity ~= nil and group.rwcommunity ~= '' then
                return SNMP_ERR_STAT_AUTHORIZATION, rsp_sub_oid, rsp_val, rsp_val_type
            end
        end

        local obj_no = req_sub_oid[1]
        local dim = effective_object_index(group_index_table, obj_no)
        if dim == 2 then
            -- scalar
            local scalar = group[obj_no]
            -- check access
            if scalar.access == MIB_ACES_UNA or not(#req_sub_oid == 2 and req_sub_oid[2] == 0) then
                return SNMP_ERR_STAT_ON_ACCESS, rsp_sub_oid, rsp_val, rsp_val_type
            end
            -- check type
            if rsp_val_type ~= scalar.tag then
                return SNMP_ERR_STAT_WRONG_TYPE, rsp_sub_oid, rsp_val, rsp_val_type
            end

            err_stat = scalar.set_f(rsp_val)
        elseif dim >= 4 then
            -- table
            local table_no = obj_no
            local entry_no = req_sub_oid[2]
            local list_no = req_sub_oid[3]
            local tab = group[table_no]
            if #req_sub_oid < 3 or tab[entry_no] == nil or tab[entry_no][list_no] == nil then
                return SNMP_ERR_STAT_ON_ACCESS, rsp_sub_oid, rsp_val, rsp_val_type
            end
            -- check access
            local variable = tab[entry_no][list_no]
            local inst_no = req_sub_oid[4]
            if variable.access == MIB_ACES_UNA or inst_no == nil then
                return SNMP_ERR_STAT_ON_ACCESS, rsp_sub_oid, rsp_val, rsp_val_type
            end
            -- check type
            if rsp_val_type ~= variable.tag then
                return SNMP_ERR_STAT_WRONG_TYPE, rsp_sub_oid, rsp_val, rsp_val_type
            end

            err_stat = variable.set_f(inst_no, rsp_val)
        else
            return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, rsp_val, rsp_val_type
        end

        if err_stat ~= nil then
            return err_stat, rsp_sub_oid, rsp_val, rsp_val_type
        else
            return 0, rsp_sub_oid, rsp_val, rsp_val_type
        end
    end

    -- get operation
    handlers[SNMP_REQ_GET] = function ()
        rsp_sub_oid = req_sub_oid

        -- priority for local group community string
        if group.rocommunity ~= community then
            -- Global community
            if _M.rocommunity ~= nil and _M.rocommunity ~= '' and _M.rocommunity ~= community then
                return SNMP_ERR_STAT_AUTHORIZATION, rsp_sub_oid, rsp_val, rsp_val_type
            end
            -- Local community
            if group.rocommunity ~= nil and group.rocommunity ~= '' then
                return SNMP_ERR_STAT_AUTHORIZATION, rsp_sub_oid, rsp_val, rsp_val_type
            end
        end

        local obj_no = req_sub_oid[1]
        local dim = effective_object_index(group_index_table, obj_no)
        if dim == 2 then
            -- Scalar
            local scalar = group[obj_no]
            -- Check access
            if scalar.access == MIB_ACES_UNA then
                return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
            end
            -- Check existence
            if not(#req_sub_oid == 2 and req_sub_oid[2] == 0) then
                return BER_TAG_NO_SUCH_INST, rsp_sub_oid, nil, nil
            end
            rsp_val, err_stat = scalar.get_f()
            rsp_val_type = scalar.tag
        elseif dim >= 4 then
            -- table
            local table_no = obj_no
            local entry_no = req_sub_oid[2]
            local list_no = req_sub_oid[3]
            local tab = group[table_no]
            if #req_sub_oid < 3 or tab[entry_no] == nil or tab[entry_no][list_no] == nil then
                return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
            end
            -- Check access
            local variable = tab[entry_no][list_no]
            if variable.access == MIB_ACES_UNA then
                return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
            end
            -- Check instance existence
            local inst_no = req_sub_oid[4]
            if inst_no == nil then
                return BER_TAG_NO_SUCH_INST, rsp_sub_oid, nil, nil
            end
            -- Get instance value
            if variable.index_key == true then
                local it
                it, err_stat = variable.get_f(varible)
                rsp_val = it[inst_no]
            else
                rsp_val, err_stat = variable.get_f(inst_no)
            end

            rsp_val_type = variable.tag
        else
            return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
        end

        if rsp_val == nil or rsp_val_type == nil then
            return BER_TAG_NO_SUCH_INST, rsp_sub_oid, nil, nil
        end

        if err_stat ~= nil then
            return err_stat, rsp_sub_oid, rsp_val, rsp_val_type
        else
            return 0, rsp_sub_oid, rsp_val, rsp_val_type
        end
    end

    -- get next operation
    handlers[SNMP_REQ_GETNEXT] = function ()
        rsp_sub_oid = req_sub_oid

        -- Priority for local group community string
        if group.rocommunity ~= community then
            -- Global community
            if _M.rocommunity ~= nil and _M.rocommunity ~= '' and _M.rocommunity ~= community then
                return SNMP_ERR_STAT_AUTHORIZATION, rsp_sub_oid, rsp_val, rsp_val_type
            end
            -- Local community
            if group.rocommunity ~= nil and group.rocommunity ~= '' then
                return SNMP_ERR_STAT_AUTHORIZATION, rsp_sub_oid, rsp_val, rsp_val_type
            end
        end

        local i = 1
        local variable = nil
        repeat
            if next(group_index_table) == nil then
                rsp_sub_oid = {}
                break
            end

            repeat
                rsp_sub_oid = matrix_find_next(group_index_table[i], rsp_sub_oid, 1, #group_index_table[i])
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
                local list_no  = rsp_sub_oid[3]
                local inst_no  = rsp_sub_oid[4]

                variable = group[table_no][entry_no][list_no]
                -- get instance value
                if variable.index_key == true then
                    local it
                    it, err_stat = variable.get_f()
                    rsp_val = it[inst_no]
                    if rsp_val == nil then rsp_val = inst_no end
                else
                    rsp_val, err_stat = variable.get_f(inst_no)
                end
                rsp_val_type = variable.tag

                if rsp_val == nil or rsp_val_type == nil or variable.access == MIB_ACES_UNA then
                    rsp_sub_oid[3] = rsp_sub_oid[3] + 1
                    rsp_sub_oid[4] = 0
                end
            else
                assert(false, 'Neighter a scalar variable nor a table')
            end
        -- Unaccessable node is ignored in getnext traversal.
        until rsp_val ~= nil and rsp_val_type ~= nil and variable.access ~= MIB_ACES_UNA

        if next(rsp_sub_oid) == nil then
            return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
        end

        if err_stat ~= nil then
            return err_stat, rsp_sub_oid, rsp_val, rsp_val_type
        else
            return 0, rsp_sub_oid, rsp_val, rsp_val_type
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
_M.init = function (port)
    core.init(port)
end

-- start snmp agent
_M.start = function ()
    core.run()
end

-- set read only community
_M.set_ro_community = function (s)
    assert(type(s) == 'string')
    _M.rocommunity = s
end

-- set read/write community
_M.set_rw_community = function (s)
    assert(type(s) == 'string')
    _M.rwcommunity = s
end

-- register a group of snmp mib nodes
_M.register_mib_group = function (oid, group, name)
    local mib_search_handler = function (op, community, req_sub_oid, req_val, req_val_type)
        return mib_node_search(group, name, op, community, req_sub_oid, req_val, req_val_type)
    end
    core.mib_node_reg(oid, mib_search_handler)
end

-- unregister a group of snmp mib nodes
_M.unregister_mib_group = function(oid)
    core.mib_node_unreg(oid)
end

return _M
