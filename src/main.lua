-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at https://mozilla.org/MPL/2.0/.

game = {}

canvas = nil
graphics = nil

x = 10

function rgb(r,g,b)
  return (r << 16) | (g << 8) | b
end

function load()
  print('load')
  package.path = 'src\\?.lua;' .. package.path
  game.state = require 'sound_state'
  game.state:load()

  -- local image = graphics.new_image('res/tic-80-wide-font.wad')

  local Thing = require 'thing'
  thing = Thing:new(nil, nil, 4, 0)
  thing:update()
  print(thing.x)

  local wad = require 'wad'
  print(wad.parse('  foo'))
end

function update()
  x = x + 2
end

function draw()
  graphics.rect(canvas, rgb(255, 0, 255), x, 60, x + 32, 92)
end
