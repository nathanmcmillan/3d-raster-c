-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at https://mozilla.org/MPL/2.0/.

local module = {}

local function skip(str, i)
  local i = i + 1
  print('[' .. str .. ']')
  print('1: ' .. str:sub(1, 1) .. ' 2: ' .. str:sub(2, 2) .. ' 3: ' .. str:sub(3, 3))
  local c = str:sub(i, i)
  if (c ~= '\n' and c ~= ' ') then
    return i - 1
  end
  local size = #str
  repeat
    i = i + 1
    if (i == size) then return i end
    c = str:sub(i, i)
  until c == '\n' or c == ' '
  return i - 1
end

function module.parse(str)
  return skip(str, 1)
end

return module
