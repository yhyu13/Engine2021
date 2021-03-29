function preferred_path(path)
    return path:gsub("\\", "/")
end

function str_split (inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={}
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                table.insert(t, str)
        end
        return t
end

--http://lua-users.org/wiki/SimpleStack
-- Stack Table
-- Uses a table as stack, use <table>:push(value) and <table>:pop()
-- Lua 5.1 compatible
-- GLOBAL
Stack = {}
-- Create a Table with stack functions
function Stack:Create()

  -- stack table
  local t = {}
  -- entry table
  t._et = {}

  -- push a value on to the stack
  function t:push(...)
    if ... then
      local targs = {...}
      -- add values
      for _,v in ipairs(targs) do
        table.insert(self._et, v)
      end
    end
  end

  -- pop a value from the stack
  function t:pop(num)

    -- get num values from stack
    local num = num or 1

    -- return table
    local entries = {}

    -- get values into entries
    for i = 1, num do
      -- get last entry
      if #self._et ~= 0 then
        table.insert(entries, self._et[#self._et])
        -- remove last value
        table.remove(self._et)
      else
        break
      end
    end
    -- return unpacked entries
    return (entries)
  end

  -- get size
  function t:size()
    return #self._et
  end

  -- get entries
  function t:get()
    return self._et
  end

  -- print values
  function t:print()
    for i,v in pairs(self._et) do
      print(i, v)
    end
  end
  return t
end

function compact_path(path)
	local ret = ""
	local tokens = str_split(path, "/")
	local stack = Stack:Create()

	for i = 1, #tokens do
		--print(tokens[i])
    	if (tokens[i] ~= "..") then
    		stack:push(tokens[i])
    	else
    		stack:pop()
    	end
    end

	local stack_table = stack:get()
    for i = 1, #stack_table do
    	if (i == 1) then
    		ret = stack_table[i]
    	else
    		ret = ret .. "/" .. stack_table[i]
    	end
    end
    --print(ret)
    return ret
end
