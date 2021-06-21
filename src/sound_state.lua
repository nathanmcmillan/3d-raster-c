-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at https://mozilla.org/MPL/2.0/.

local module = {}

function module:load()
  self.sound_edit = require 'sound_edit'
  self.sound_edit:load()
end

return module
