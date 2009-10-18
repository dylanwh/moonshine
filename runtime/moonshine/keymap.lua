-- API
-- keymap:bind('^[1', '/window goto 1')
--
-- keymap:keypress('^[')
-- keymap:keypress('1')
--
-- Results of above two calls to keypress():
-- /window goto 1
local bit     = require "bit"

local function keyspec(spec)
    local function ctrl(c)
        return string.char(bit.bxor(string.byte(string.upper(c)), 64))
    end
    return spec:gsub("%^(.)", ctrl)
end

local Keymap = new "moonshine.object"

function Keymap:__init(attr)
    self._shell  = attr.shell or new "moonshine.shell"
    self._tree   = new "moonshine.tree"
    self._keybuf = ""
end

function Keymap:keypress(key)
    local tree  = self._tree
    local shell = self._shell
    self._keybuf = self._keybuf .. key

    local found_key, text, index, dirn = tree:find_near(self._keybuf)
    if found_key == nil then
        -- No keys defined??
        return false
    end

    if dirn == 0 then
        self._keybuf = ""
        shell:eval(text)
        return true
    elseif dirn > 0 then
        index = index + 1
        found_key, text = tree:lookup_index(index)
    end

    if found_key == nil then
        -- should only ever happen if there are no keys registered at all
        found_key = ""
    end

    if string.sub(found_key, 1, string.len(self._keybuf)) ~= self._keybuf then
        -- not a prefix of anything
        self._keybuf = ""
        return false
    else
        -- prefix of something, keep going
        return true
    end
end

function Keymap:bind(spec, text)
    self._tree:insert(keyspec(spec), text)
end

return Keymap
