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

-- scalar index
local eocVLANVersionNumber      = 1
local eocVLANMaxVID             = 2
local eocVLANMaxSupportedVLANs  = 3
local eocVLANCreatedVLANNumber  = 4
local eocVLANVIDList            = 5
local eocVLANNextFreeVID        = 6

local eocVLANTable          = 7
local eocVLANEntry          = 1

-- table index
local eocVLANPortTable      = 8
-- entry index
local eocVLANPortEntry      = 1
-- list index
local eocPortCBATCardIndex   = 1
local eocPortCNUIndex        = 2
local eocVLANPortIndex           = 3
local eocVLANPortPVID            = 4
local eocVLANPortTPID            = 5
local eocVLANPortPrio            = 6
local eocVLANPortVIDList         = 7
local eocVLANPortUntaggedVIDList = 8
local eocVLANPortMode            = 9

-- value
local eocVLANVersionNumber_      = 1
local eocVLANMaxVID_             = 101
local eocVLANMaxSupportedVLANs_  = 10
local eocVLANCreatedVLANNumber_  = 8
local eocVLANVIDList_            = "0123456789"
local eocVLANNextFreeVID_        = 2

local eocVLANIndex_         = {1, 2, 3, 4, 5, 6, 7}
local eocVLANName_          = {
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
}
local eocMulticastVLANFlag_ = {1, 2, 1, 2, 1, 2, 1}
local eocVLANRowStatus_     = {1, 1, 1, 1, 4, 5, 6}

local eocVLANGroup = {}

local row_status_set = function (inst_no, status)
    if status == 1 then
        eocVLANIndex_[inst_no] = inst_no
        if eocVLANRowStatus_[inst_no] == nil then
            table.insert(eocVLANIndex_, inst_no, inst_no)
            table.insert(eocVLANName_, inst_no, "")
            table.insert(eocMulticastVLANFlag_, inst_no, 0)
        end
    elseif status == 0 then
        if eocVLANRowStatus_[inst_no] ~= nil then
            table.remove(eocVLANIndex_, inst_no)
            table.insert(eocVLANIndex_, inst_no, nil)
            --table.remove(eocVLANName_, inst_no)
            --table.remove(eocMulticastVLANFlag_, inst_no)
        end
    end
    eocVLANRowStatus_[inst_no] = status
    -- Regenerate group index table.
    mib.dictionary_indexes_generate(eocVLANGroup, 'eoc_vlan')
end

eocVLANGroup = {
    rocommunity = 'public',
    [eocVLANVersionNumber]     = mib.ConstInt(function () return eocVLANVersionNumber_ end),
    [eocVLANMaxVID]            = mib.ConstInt(function () return eocVLANMaxVID_ end),
    [eocVLANMaxSupportedVLANs] = mib.ConstInt(function () return eocVLANMaxSupportedVLANs_ end),
    [eocVLANCreatedVLANNumber] = mib.ConstInt(function () return eocVLANCreatedVLANNumber_ end),
    [eocVLANVIDList]           = mib.ConstString(function () return eocVLANVIDList_ end),
    [eocVLANNextFreeVID]       = mib.ConstInt(function () return eocVLANNextFreeVID_ end),
    [eocVLANTable]      = {
        [eocVLANEntry]  = {
            [1] = mib.ConstIndex(function () return eocVLANIndex_ end),
            [2] = mib.String(function (i) return eocVLANName_[i] end, function (i, v) eocVLANName_[i] = v end),
            [3] = mib.Int(function (i) return eocMulticastVLANFlag_[i] end, function (i, v) eocMulticastVLANFlag_[i] = v end),
            [4] = mib.Int(function (i) return eocVLANRowStatus_[i] end, row_status_set),
        }
    },
--[[
    [eocVLANPortTable] = {
        [eocVLANPortEntry] = {
            [eocVLANCBATCardIndex]
            [eocVLANCNUIndex]
            [eocVLANPortIndex]
            [eocVLANPortPVID]
            --[eocVLANPortTPID]
            --[eocVLANPortPrio]
            [eocVLANPortVIDList]
            [eocVLANPortUntaggedVIDList]
            --[eocVLANPortMode]
        }
    }
]]
}

return eocVLANGroup
