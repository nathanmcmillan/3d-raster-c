-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at https://mozilla.org/MPL/2.0/.

local Thing = {}
Thing.__index = Thing

function Thing:new(world, entity, x, z)
  local t = {}
  setmetatable(t, Thing)
  t.world = world
  t.entity = entity
  t.sector = nil
  t.floor = 0.0
  t.ceiling = 0.0
  t.x = x
  t.z = z
  t.previousX = x
  t.previousZ = z
  t.y = 0.0
  t.delta_x = 0.0
  t.delta_y = 0.0
  t.delta_z = 0.0
  return t
end

function Thing:update()
  self.x = self.x + 10
  self.y = self.y + 12
end

return Thing
