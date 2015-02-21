------------------------------------------------------------------------------
-- getopt_alt.lua from http://lua-users.org/wiki/AlternativeGetOpt

-- getopt, POSIX style command line argument parser
-- param arg contains the command line arguments in a standard table.
-- param options is a string with the letters that expect string values.
-- returns a table where associated keys are true, nil, or a string value.
-- The following example styles are supported
--   -a one  ==> opts["a"]=="one"
--   -bone   ==> opts["b"]=="one"
--   -c      ==> opts["c"]==true
--   --c=one ==> opts["c"]=="one"
--   -cdaone ==> opts["c"]==true opts["d"]==true opts["a"]=="one"
-- note POSIX demands the parser ends at the first non option
--      this behavior isn't implemented.
------------------------------------------------------------------------------

local utils = {}

utils.getopt = function (arg, options)
  local tab = {}
  for k, v in ipairs(arg) do
    if string.sub( v, 1, 2) == "--" then
      local x = string.find( v, "=", 1, true )
      if x then tab[ string.sub( v, 3, x-1 ) ] = string.sub( v, x+1 )
      else      tab[ string.sub( v, 3 ) ] = true
      end
    elseif string.sub( v, 1, 1 ) == "-" then
      local y = 2
      local l = string.len(v)
      local jopt
      while ( y <= l ) do
        jopt = string.sub( v, y, y )
        if string.find( options, jopt, 1, true ) then
          if y < l then
            tab[ jopt ] = string.sub( v, y+1 )
            y = l
          else
            tab[ jopt ] = arg[ k + 1 ]
          end
        else
          tab[ jopt ] = true
        end
        y = y + 1
      end
    end
  end
  return tab
end

-----------------------------------------
-- convert oid string to oid lua table
-----------------------------------------
utils.str2oid = function (s)
	local oid = {}
	for n in string.gmatch(s, '%d+') do
		table.insert(oid, tonumber(n))
	end
	return oid
end

--------------------------------------------------------------------------------
-- iter
--
-- Return an iterable function. If _o_ is an array table, it will return a
-- function that return the table element one by one. If _o_ is a function,
-- the _o_ will be return directly.
--------------------------------------------------------------------------------
utils.iter = function(o)
	if type(o) == 'function' then
		return o
	else
		local i = 0
		local n = table.getn(o)
		return function ()
			i = i + 1
			if i <= n then return o[i] end
		end
	end
end

-------------------------------------------------------------------------------
-- map
--
-- Apply function to every item of iterable and return an array of the results.
-------------------------------------------------------------------------------
utils.map = function(fun, iter)
	local mapped_list = {}

	iter = utils.iter(iter)
	
	for v in iter do
		table.insert(mapped_list, fun(v))
	end

	return mapped_list
end

-------------------------------------------------------------------------------
-- reduce
--
-- Apply function of two arguments cumulatively to the items of iterable,
-- from left to right, so as to reduce the iterable to a single value.
--
-- iter could be a table or iterable function.
-------------------------------------------------------------------------------
utils.reduce = function(fun, iter, init)
	local accum_val

	iter = utils.iter(iter)

	if init == nil then
		init = iter()
		if init == nil then
			error('reduce() of empty sequence with no initial value')
		end
	end
	
	accum_val = init
	for v in iter do
		accum_val = fun(accum_val, v)
	end

	return accum_val
end

-------------------------------------------------------------------------------
-- filter
--
-- Construct an array table from those elements of array table or iterable
-- function for which function returns true.
-------------------------------------------------------------------------------
utils.filter = function(fun, iter)
	local filtered_list = {}

	iter = utils.iter(iter)
	
	for v in iter do
		if fun(v) then
			table.insert(filtered_list, v)
		end
	end

	return filtered_list
end

--------------------------------------------------------------------------------
-- str2mac
--
-- convert MAC address string to table of number
--------------------------------------------------------------------------------
utils.str2mac = function(mac)
	assert(type(mac) == 'string')
	local mtab = {mac:match("(%x%x):(%x%x):(%x%x):(%x%x):(%x%x):(%x%x)")}
	if #mtab ~= 6 then
		error("invaild MAC address.")
	end
	return utils.map(function(x) return tonumber(x, 16) end, mtab)
end

--------------------------------------------------------------------------------
-- mac2oct
--
-- convert MAC address array table or mac string to encoded octstring
--------------------------------------------------------------------------------
utils.mac2oct = function(mac)
	if type(mac) == 'string' then
		mac = utils.str2mac(mac)
	end

	return utils.reduce(
		function(s, n)
			return s..string.char(n)
			end,
			mac,
			''
		)
end

--------------------------------------------------------------------------------
-- print_table
--
-- pretty print lua table, php print_r like.
-- based on https://gist.github.com/nrk/31175
--------------------------------------------------------------------------------
utils.print_table = function(t)
	local print_r_cache = {}
	local sub_print_r = function(t, indent)
		if print_r_cache[tostring(t)] then
			print(indent.."*"..tostring(t))
		else
			print_r_cache[tostring(t)]=true
			if type(t) == "table" then
				pos = tostring(pos)
				for pos,val in pairs(t) do
					if type(val) == "table" then
						print(indent.."["..pos.."] => "..tostring(t).." {")
						sub_print_r(val,indent..string.rep(" ",string.len(pos)+8))
						print(indent..string.rep(" ",string.len(pos)+6).."}")
					else
						print(indent.."["..pos.."] => "..tostring(val))
					end
				end
			else
				print(indent..tostring(t))
			end
		end
	end
	sub_print_r(t,"\t")
end

return utils
