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
