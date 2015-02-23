local lunit = require "lunit"

package.path = 'lualib/?/init.lua;lualib/?.lua;'..package.path

local utils = require "smartsnmp.utils"

module("test_utils", lunit.testcase, package.seeall)

test_iter1 = function()
	-- test iter
	local i = 0
	for v in utils.iter({1, 2, 3, 4, 5}) do
		i = i + 1
		assert_true(v == i)
	end
	assert_true(i == 5)
end

test_iter2 = function()
	-- test iter
	local i = utils.iter(function() return nil end)

	local x = true
	for v in i do
		x = false
	end
	assert_true(x)
end

-- helper: compare 2 array table
local array_compare = function(ta, tb)
	if #ta ~= #tb then
		return false
	end
	for i, a in ipairs(ta) do
		if a ~= tb[i] then
			return false
		end
	end
	return true
end

-- test map
test_map = function()
	assert_true(array_compare(
		utils.map(
			function (x)
				return x + 1
			end,
			{1, 2, 3, 4, 5}
		),
		{2, 3, 4, 5, 6}
	))
end

-- test filter
test_filter = function()
	assert_true(array_compare(
		utils.filter(
			function (x)
				return (x % 2) == 1
			end,
			{1, 2, 3, 4, 5}
		),
		{1, 3, 5}
	))
end

-- test reduce w/o init
test_reduce1 = function()
	assert_true(
		utils.reduce(
			function (x, y)
				return x * y
			end,
			{1, 2, 3, 4, 5}
		) == 120
	)
end

-- test reduce with init
test_reduce2 = function()
	assert_true(
		utils.reduce(
			function (x, y)
				return x * y
			end,
			{1, 2, 3, 4, 5},
			0
		) == 0
	)
end
	
-- test str2mac
test_str2mac = function()
	assert_true(array_compare(utils.str2mac("00:00:00:00:00:00"), {0, 0, 0, 0, 0, 0}))
	assert_true(array_compare(utils.str2mac("ff:ff:ff:ff:ff:ff"), {255, 255, 255, 255, 255, 255}))
	assert_true(array_compare(utils.str2mac("FF:FF:FF:FF:FF:FF"), {255, 255, 255, 255, 255, 255}))
	assert_true(array_compare(utils.str2mac("11:22:33:44:55:66"), {0x11, 0x22, 0x33, 0x44, 0x55, 0x66}))
end
	
-- test mac2oct
test_mac2oct = function()
	assert_true(utils.mac2oct({0x61, 0x62, 0x63, 0x64, 0x65, 0x66}) == 'abcdef')
	assert_true(utils.mac2oct("61:62:63:64:65:66") == 'abcdef')
end

lunit.main()
