/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

canvas = nil
draw = nil

x = 10

function rgb(r,g,b)
  return (r << 16) | (g << 8) | b
end

function update()
  x = x + 2
  draw.rect(canvas, rgb(255, 0, 255), x, 60, x + 32, 92)
end
