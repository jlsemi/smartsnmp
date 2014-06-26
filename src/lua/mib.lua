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

local matrix_find_next = require "find_next"

-- Access
MIB_ACES_UNA        = 0
MIB_ACES_RO         = 1
MIB_ACES_RW         = 2

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

mib = {}

-- String value for RO.
function mib.ConstString(s)
    assert(type(s) == 'string' or type(s) == 'function', 'Argument must be string or function type')
    return { tag = BER_TAG_OCTSTR, access = MIB_ACES_RO, value = s }
end

-- String value for RW.
function mib.String(s)
    assert(type(s) == 'string', 'Argument must be string type')
    return { tag = BER_TAG_OCTSTR, access = MIB_ACES_RW, value = s }
end

-- Integer value for RO.
function mib.ConstInt(i)
    assert(type(i) == 'number' or type(i) == 'function', 'Argument must be integer or function type')
    return { tag = BER_TAG_INT, access = MIB_ACES_RO, value = i }
end

-- Integer value for RW.
function mib.Int(i)
    assert(type(i) == 'number', 'Argument must be integer type')
    return { tag = BER_TAG_INT, access = MIB_ACES_RW, value = i }
end

-- Count value for RO.
function mib.ConstCount(c)
    assert(type(c) == 'number' or type(c) == 'functcon', 'Argument must be count or functcon type')
    return { tag = BER_TAG_CNT, access = MIB_ACES_RO, value = c }
end

-- Count value for RW.
function mib.Count(c)
    assert(type(c) == 'number', 'Argument must be count type')
    return { tag = BER_TAG_CNT, access = MIB_ACES_RW, value = c }
end

-- IP address value for RO.
function mib.ConstIpaddr(ip)
    assert((type(ip) == 'table' and (#ip == 4 or #ip == 6)) or type(ip) == 'functcon', 'Argument must be IP address or functcon type')
    for k in pairs(ip) do assert(type(ip[k]) == 'number', 'Argument must be IP address') end
    return { tag = BER_TAG_IPADDR, access = MIB_ACES_RO, value = ip }
end

-- IP address value for RW.
function mib.Ipaddr(ip)
    assert(type(ip) == 'table' and (#ip == 4 or #ip == 6), 'Argument must be IP address type')
    for k in pairs(ip) do assert(type(ip[k]) == 'number', 'Argument must be IP address') end
    return { tag = BER_TAG_IPADDR, access = MIB_ACES_RW, value = ip }
end

-- Oid value for RO.
function mib.ConstOid(o)
    assert(type(o) == 'table' or type(o) == 'function', 'Argument must be oid array or function type')
    return { tag = BER_TAG_OBJID, access = MIB_ACES_RO, value = o }
end

-- Oid value for RW.
function mib.Oid(o)
    assert(type(o) == 'table', 'Argument must be oid array type')
    return { tag = BER_TAG_OBJID, access = MIB_ACES_RW, value = o }
end

-- Timeticks value for RO.
function mib.ConstTimeticks(t)
    assert(type(t) == 'number' or type(t) == 'function', 'Argument must be timeticks or function type')
    return { tag = BER_TAG_TIMETICKS, access = MIB_ACES_RO, value = t }
end

-- Timeticks value for RW.
function mib.Timeticks(t)
    assert(type(t) == 'number', 'Argument must be timeticks type')
    return { tag = BER_TAG_TIMETICKS, access = MIB_ACES_RW, value = t }
end

-- Gauge value for RO.
function mib.ConstGauge(g)
    assert(type(g) == 'number' or type(g) == 'function', 'Argument must be gauge or function type')
    return { tag = BER_TAG_GAU, access = MIB_ACES_RO, value = g }
end

-- Gauge value for RW.
function mib.Gauge(g)
    assert(type(g) == 'number', 'Argument must be gauge type')
    return { tag = BER_TAG_GAU, access = MIB_ACES_RW, value = g }
end

-- Auto incremented indexes.
function mib.AutoIndex(n)
    assert(type(n) == 'number', 'Argument must be integer type')
    local it = {}
    for i = 1, n do it[i] = i end
    return { tag = BER_TAG_INT, access = MIB_ACES_UNA, value = it }
end

-- Integer list for RO.
function mib.ConstIntList(it)
    assert(type(it) == 'table', 'Argument must be array format')
    for k in pairs(it) do
        if type(it[k]) == 'function' then it[k] = it[k]() end
        assert(type(it[k]) == 'number', 'Each element must be integer type')
    end
    return { tag = BER_TAG_INT, access = MIB_ACES_RO, value = it }
end

-- Integer list for RW.
function mib.IntList(it)
    assert(type(it) == 'table', 'Argument must be array format')
    for k in pairs(it) do
        if type(it[k]) == 'function' then it[k] = it[k]() end
        assert(type(it[k]) == 'number', 'Each element must be integer type')
    end
    return { tag = BER_TAG_INT, access = MIB_ACES_RW, value = it }
end

-- Count list for RO.
function mib.ConstCountList(ct)
    assert(type(ct) == 'table', 'Argument must be array format')
    for k in pairs(ct) do
        if type(ct[k]) == 'function' then ct[k] = ct[k]() end
        assert(type(ct[k]) == 'number', 'Each element must be count type')
    end
    return { tag = BER_TAG_CNT, access = MIB_ACES_RO, value = ct }
end

-- Count list for RW.
function mib.CountList(ct)
    assert(type(ct) == 'table', 'Argument must be array format')
    for k in pairs(ct) do
        if type(ct[k]) == 'function' then ct[k] = ct[k]() end
        assert(type(ct[k]) == 'number', 'Each element must be count type')
    end
    return { tag = BER_TAG_CNT, access = MIB_ACES_RW, value = ct }
end

-- String list for RO.
function mib.ConstStringList(st)
    assert(type(st) == 'table', 'Argument must be array format')
    for k in pairs(st) do
        if type(st[k]) == 'function' then st[k] = st[k]() end
        assert(type(st[k]) == 'string', 'Each element must be string type')
    end
    return { tag = BER_TAG_OCTSTR, access = MIB_ACES_RO, value = st }
end

-- String list for RW.
function mib.StringList(st)
    assert(type(st) == 'table', 'Argument must be array format')
    for k in pairs(st) do
        if type(st[k]) == 'function' then st[k] = st[k]() end
        assert(type(st[k]) == 'string', 'Each element must be string type')
    end
    return { tag = BER_TAG_OCTSTR, access = MIB_ACES_RW, value = st }
end

-- Oid list for RO.
function mib.ConstOidList(ot)
    assert(type(ot) == 'table', 'Argument must be array format')
    for k in pairs(ot) do
        if type(ot[k]) == 'function' then ot[k] = ot[k]() end
        assert(type(ot[k]) == 'table', 'Each element must be oid array type')
    end
    return { tag = BER_TAG_OBJID, access = MIB_ACES_RO, value = ot }
end

-- Oid list for RW.
function mib.OidList(ot)
    assert(type(ot) == 'table', 'Argument must be array format')
    for k in pairs(ot) do
        if type(ot[k]) == 'function' then ot[k] = ot[k]() end
        assert(type(ot[k]) == 'table', 'Each element must be oid array type')
    end
    return { tag = BER_TAG_OBJID, access = MIB_ACES_RW, value = ot }
end

-- Ip address list for RO.
function mib.ConstIpaddrList(ipt)
    assert(type(ipt) == 'table', 'Argument must be array format')
    for _, ip in pairs(ipt) do
        if type(ip) == 'function' then ip = ip() end
        assert(type(ip) == 'table' and (#ip == 4 or #ip == 6), 'Each element must be ip address type')
        for k in pairs(ip) do assert(type(ip[k]) == 'number', 'Each element must be ip address type') end
    end
    return { tag = BER_TAG_IPADDR, access = MIB_ACES_RO, value = ipt }
end

-- Ip address list for RW.
function mib.IpaddrList(ipt)
    assert(type(ipt) == 'table', 'Argument must be array format')
    for _, ip in pairs(ipt) do
        if type(ip) == 'function' then ip = ip() end
        assert(type(ip) == 'table' and (#ip == 4 or #ip == 6), 'Each element must be ip address type')
        for k in pairs(ip) do assert(type(ip[k]) == 'number', 'Each element must be ip address type') end
    end
    return { tag = BER_TAG_IPADDR, access = MIB_ACES_RW, value = ipt }
end

-- Gauge list for RO.
function mib.ConstGaugeList(gt)
    assert(type(gt) == 'table', 'Argument must be array format')
    for k in pairs(gt) do
        if type(gt[k]) == 'function' then gt[k] = gt[k]() end
        assert(type(gt[k]) == 'number', 'Each element must be gauge type')
    end
    return { tag = BER_TAG_GAU, access = MIB_ACES_RO, value = gt }
end

-- Gauge list for RW.
function mib.GaugeList(gt)
    assert(type(gt) == 'table', 'Argument must be array format')
    for k in pairs(gt) do
        if type(gt[k]) == 'function' then gt[k] = gt[k]() end
        assert(type(gt[k]) == 'number', 'Each element must be gauge type')
    end
    return { tag = BER_TAG_GAU, access = MIB_ACES_RW, value = gt }
end

-- Timeticks list for RO.
function mib.ConstTimeticksList(tt)
    assert(type(tt) == 'table', 'Argument must be array format')
    for k in pairs(tt) do
        if type(tt[k]) == 'function' then tt[k] = tt[k]() end
        assert(type(tt[k]) == 'number', 'Each element must be timeticks type')
    end
    return { tag = BER_TAG_TIMETICKS, access = MIB_ACES_RO, value = tt }
end

-- Timeticks list for RW.
function mib.TimeticksList(tt)
    assert(type(tt) == 'table', 'Argument must be array format')
    for k in pairs(tt) do
        if type(tt[k]) == 'function' then tt[k] = tt[k]() end
        assert(type(tt[k]) == 'number', 'Each element must be timeticks type')
    end
    return { tag = BER_TAG_TIMETICKS, access = MIB_ACES_RW, value = tt }
end

-- Writing algorithm in lua really sucks, though we still have lots of jobs to do ...
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

  2. The 4th dim(inst_no) in matrix of a table node representing instance
  index is assumed to be one id number each element instead of sequence of oids.
  Currently we don't support IP address as an instance index.
]]--

function check_group_index_table(it)
    for i, v in ipairs(it) do
        if #v == 2 then
           print(unpack(v[1]))
        elseif #v == 4 then
           print(unpack(v[1]))
           print(unpack(v[2]))
           print(unpack(v[3]))
           print(unpack(v[4]))
        else assert(false) end
    end
end

function group_index_table_generator(group, name)
    assert(type(group) == 'table', string.format('Group should be container'))
    assert(type(name) == 'string', string.format('What is the group\'s name?'))

    local group_indexes = {}  -- result to produce
    local scalar_indexes = {{},{0}}  -- 2 dimensions matrix
    local table_indexes = {{},{},{},{}}  -- 4 dimensions matrix

    for obj_no in pairs(group) do
        if type(obj_no) == 'number' then
            if group[obj_no].value == nil then
                -- table
                table_indexes = {{},{},{},{}}
                table.insert(table_indexes[1], obj_no)

                local tab = group[obj_no]
                assert(tab.value == nil, string.format('%s[%d]: Table should be container not variable', name, obj_no))
                -- For simplicity, we hold at most one entry each table.
                -- If you want multiple entries you may create revelant number of tables.
                assert(#tab <= 1, string.format('%s[%d]: Sorry but for simplicity, each table can hold one entry at most', name, obj_no))
                local entry_no, entry = next(tab)
                assert(entry.value == nil, string.format('%s[%d][%d]: Entry should be container not variable', name, obj_no, entry_no))
                if type(entry_no) == 'number' then
                    table.insert(table_indexes[2], entry_no)
                end

                if entry ~= nil then
                    -- list
                    local _, index_list = next(entry)
                    if index_list ~= nil then
                        -- Check if all variables in the same entry have the same value number.
                        for list_no, list in pairs(entry) do
                            if type(list_no) == 'number' then
                                assert(list.value ~= nil, string.format('%s[%d][%d][%d]: List should be variable not container', name, obj_no, entry_no, list_no))
                                assert(#index_list.value == #list.value, string.format('%s[%d][%d][%d]: All variables in a entry must have the same value count', name, obj_no, entry_no, list_no))
                                table.insert(table_indexes[3], list_no)
                            end
                        end

                        -- instance
                        for inst_no in pairs(index_list.value) do
                            if type(inst_no) == 'number' then
                                table.insert(table_indexes[4], inst_no)
                            end
                        end
                    end
                end

                table.sort(table_indexes[2])
                table.sort(table_indexes[3])
                table.sort(table_indexes[4])

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
function mib_node_search(group, group_index_table, op, req_sub_oid, req_val, req_val_type)
    local rsp_sub_oid = nil
    local rsp_val = nil
    local rsp_val_type = nil
    -- Search obj_id in group index table.
    local effective_object_index = function (tab, id)
        for i in ipairs(tab) do
            for _, v in ipairs(tab[i][1]) do
                if id == v then return #tab[i] end
            end
        end
        return nil
    end

    handlers = {}
    -- set operation
    handlers[SNMP_REQ_SET] = function ()
        rsp_sub_oid = req_sub_oid
        rsp_val = req_val
        rsp_val_type = req_val_type

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

            scalar.value = rsp_val
        elseif dim == 4 then
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
            if variable.access == MIB_ACES_UNA or inst_no == nil or variable.value[inst_no] == nil then
                return SNMP_ERR_STAT_ON_ACCESS, rsp_sub_oid, rsp_val, rsp_val_type
            end
            -- check type
            if rsp_val_type ~= variable.tag then
                return SNMP_ERR_STAT_WRONG_TYPE, rsp_sub_oid, rsp_val, rsp_val_type
            end
            variable.value[inst_no] = rsp_val
        else
            return SNMP_ERR_STAT_ON_ACCESS, rsp_sub_oid, rsp_val, rsp_val_type
        end

        return 0, rsp_sub_oid, rsp_val, rsp_val_type
    end

    -- get operation
    handlers[SNMP_REQ_GET] = function ()
        rsp_sub_oid = req_sub_oid

        local obj_no = req_sub_oid[1]
        local dim = effective_object_index(group_index_table, obj_no)
        if dim == 2 then
            -- scalar
            local scalar = group[obj_no]
            -- check access
            if scalar.access == MIB_ACES_UNA then
                return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
            end
            -- check existence
            if not(#req_sub_oid == 2 and req_sub_oid[2] == 0) then
                return BER_TAG_NO_SUCH_INST, rsp_sub_oid, nil, nil
            end
            rsp_val = scalar.value
            rsp_val_type = scalar.tag
        elseif dim == 4 then
            -- table
            local table_no = obj_no
            local entry_no = req_sub_oid[2]
            local list_no = req_sub_oid[3]
            local tab = group[table_no]
            if #req_sub_oid < 3 or tab[entry_no] == nil or tab[entry_no][list_no] == nil then
                return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
            end
            -- check access
            local variable = tab[entry_no][list_no]
            if variable.access == MIB_ACES_UNA then
                return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
            end
            -- check existence
            local inst_no = req_sub_oid[4]
            if inst_no == nil or variable.value[inst_no] == nil then
                return BER_TAG_NO_SUCH_INST, rsp_sub_oid, nil, nil
            end

            rsp_val = variable.value[inst_no]
            rsp_val_type = variable.tag
        else
            return BER_TAG_NO_SUCH_OBJ, rsp_sub_oid, nil, nil
        end

        if rsp_val == nil or rsp_val_type == nil then
            return BER_TAG_NO_SUCH_INST, rsp_sub_oid, nil, nil
        end

        if (type(rsp_val) == 'function') then rsp_val = rsp_val() end
        return 0, rsp_sub_oid, rsp_val, rsp_val_type
    end

    -- get next operation
    handlers[SNMP_REQ_GETNEXT] = function ()
        rsp_sub_oid = req_sub_oid

        local i = 1
        local variable = nil
        repeat
            -- Why Lua not support 'continue' statement?
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
                rsp_val = variable.value
                rsp_val_type = variable.tag
            elseif #rsp_sub_oid == 4 then
                -- table
                local table_no = rsp_sub_oid[1]
                local entry_no = rsp_sub_oid[2]
                local list_no  = rsp_sub_oid[3]
                local inst_no  = rsp_sub_oid[4]
                variable = group[table_no][entry_no][list_no]
                rsp_val = variable.value[inst_no]
                rsp_val_type = variable.tag
                if variable.access == MIB_ACES_UNA then
                    rsp_sub_oid[3] = rsp_sub_oid[3] + 1
                    rsp_sub_oid[4] = 0
                end
            else
                assert(false, 'Neigther a scalar variable nor a table')
            end
        -- Unaccessable node is ignored in getnext traversal.
        until variable.access ~= MIB_ACES_UNA

        if next(rsp_sub_oid) == nil then
            return BER_TAG_NO_SUCH_OBJ, req_sub_oid, nil, nil
        end

        if (type(rsp_val) == 'function') then rsp_val = rsp_val() end
        return 0, rsp_sub_oid, rsp_val, rsp_val_type
    end

    H = handlers[op]
    return H()
end

-- Register interfaces
function mib_group_register(oid, group, name)
    local group_indexes = group_index_table_generator(group, name)
    local mib_search_handler = function (op, req_sub_oid, req_val, req_val_type)
    	return mib_node_search(group, group_indexes, op, req_sub_oid, req_val, req_val_type)
    end
    _G[name] = mib_search_handler
    mib_lib.mib_node_reg(oid, name)
end

-- Unregister interfaces
function mib_group_unregister(oid)
    mib_lib.mib_node_unreg(oid)
end

return mib
