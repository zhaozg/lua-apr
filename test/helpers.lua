--[[

 Test infrastructure for the Lua/APR binding.

 Author: Peter Odding <peter@peterodding.com>
 Last Change: February 18, 2011
 Homepage: http://peterodding.com/code/lua/apr/
 License: MIT

--]]

local apr = require 'apr'
local helpers = {}

function print(...)
  local t = {}
  for i = 1, select('#', ...) do
    t[#t + 1] = tostring(select(i, ...))
  end
  io.stderr:write(table.concat(t, ' ') .. '\n')
end

function helpers.message(s, ...) -- {{{1
  io.stderr:write('\r', string.format(s, ...))
  io.stderr:flush()
end

function helpers.warning(s, ...) -- {{{1
  io.stderr:write("\nWarning: ", string.format(s, ...))
  io.stderr:flush()
end

function helpers.filedefined() -- {{{1
  local info = assert(debug.getinfo(2, 'S'))
  return info.source:sub(2)
end

function helpers.deepequal(a, b) -- {{{1
  if type(a) ~= 'table' or type(b) ~= 'table' then
    return a == b
  else
    for k, v in pairs(a) do
      if not helpers.deepequal(v, b[k]) then
        return false
      end
    end
    for k, v in pairs(b) do
      if not helpers.deepequal(v, a[k]) then
        return false
      end
    end
    return true
  end
end

function helpers.checktuple(expected, ...) -- {{{1
  assert(select('#', ...) == #expected)
  for i = 1, #expected do assert(expected[i] == select(i, ...)) end
end

function helpers.scriptpath(name) -- {{{1
  local directory = apr.filepath_parent(helpers.filedefined())
  return assert(apr.filepath_merge(directory, name))
end

function helpers.wait_for(signalfile, timeout) -- {{{1
  local starttime = apr.time_now()
  while apr.time_now() - starttime < timeout do
    apr.sleep(0.25)
    if apr.stat(signalfile, 'type') == 'file' then
      return true
    end
  end
end

local tmpnum = 1
local tmpdir = assert(apr.temp_dir_get())

function helpers.tmpname() -- {{{1
  local name = 'lua-apr-tempfile-' .. tmpnum
  local file = apr.filepath_merge(tmpdir, name)
  apr.file_remove(file)
  tmpnum = tmpnum + 1
  return file
end

function helpers.readfile(path) -- {{{1
  local handle = assert(io.open(path, 'r'))
  local data = assert(handle:read '*all')
  assert(handle:close())
  return data
end

function helpers.writefile(path, data) -- {{{1
  local handle = assert(io.open(path, 'w'))
  assert(handle:write(data))
  assert(handle:close())
end

function helpers.writable(directory) -- {{{1
  local entry = apr.filepath_merge(directory, 'io_dir_writable_check')
  local status = pcall(helpers.writefile, entry, 'something')
  if status then os.remove(entry) end
  return status
end

-- }}}1

return helpers