-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at https://mozilla.org/MPL/2.0/.

local Thing = {}
Thing.__index = Thing

function Thing:new(world, entity, x, z)
  local this = {}
  setmetatable(this, Thing)
  this.world = world
  this.entity = entity
  this.sector = nil
  this.floor = 0.0
  this.ceiling = 0.0
  this.x = x
  this.z = z
  this.previousX = x
  this.previousZ = z
  this.y = 0.0
  this.delta_x = 0.0
  this.delta_y = 0.0
  this.delta_z = 0.0
  return this
end

function Thing:update()
  this.x = this.x + 10
  this.y = this.y + 12
end

return Thing
