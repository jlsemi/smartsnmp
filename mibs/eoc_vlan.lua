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
eocVLANVersionNumber      = 1
eocVLANMaxVID             = 2
eocVLANMaxSupportedVLANs  = 3
eocVLANCreatedVLANNumber  = 4
eocVLANVIDList            = 5
eocVLANNextFreeVID        = 6

eocVLANTable          = 7
eocVLANEntry          = 1

-- table index
eocVLANPortTable      = 8
-- entry index
eocVLANPortEntry      = 1
-- list index
eocVLANPortCBATCardIndex   = 1
eocVLANPortCNUIndex        = 2
eocVLANPortIndex           = 3
eocVLANPortPVID            = 4
eocVLANPortTPID            = 5
eocVLANPortPrio            = 6
eocVLANPortVIDList         = 7
eocVLANPortUntaggedVIDList = 8
eocVLANPortMode            = 9

-- value
eocVLANVersionNumber_      = 1
eocVLANMaxVID_             = 101
eocVLANMaxSupportedVLANs_  = 10
eocVLANCreatedVLANNumber_  = 8
eocVLANVIDList_            = "0123456789"
eocVLANNextFreeVID_        = 2

eocVLANIndex_         = {1, 2, 3, 4, 5, 6, 7}
eocVLANName_          = {
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
    '0123456789987654321012345678998765432100123456789987654321001234567899876543210123',
}
eocMulticastVLANFlag_ = {1, 2, 1, 2, 1, 2, 1}
eocVLANRowStatus_     = {1, 1, 1, 1, 4, 5, 6}

eocVLANGroup = {}

local function row_status_set(inst_no, status)
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
}

return eocVLANGroup
