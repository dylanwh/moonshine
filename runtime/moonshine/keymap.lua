-- API
-- keymap:bind('^[1', 'window goto 1')
--
-- keymap:keypress('^[')
-- keymap:keypress('1')
--
-- Results of above two calls to keypress():
-- /window goto 1
local bit     = require "bit"

local function ctrl(c)
    return string.char(bit.bxor(string.byte(string.upper(c)), 64))
end

local function keyspec(spec)
    local term = require "moonshine.ui.term"
    return spec:gsub("%^(.)", ctrl):gsub('{([%w_]+)}', term.tigetstr)
end

local Keymap = new "moonshine.object"

function Keymap:__init()
    self._tree   = new "moonshine.tree"
    self._keybuf = ""
end

function Keymap:process(key)
    local tree  = self._tree
    self._keybuf = self._keybuf .. key

    local found_key, func, index, dirn = tree:find_near(self._keybuf)
    if found_key == nil then
        -- No keys defined??
        return false
    end

    if dirn == 0 then
        self._keybuf = ""
        func()
        return true
    elseif dirn > 0 then
        index     = index + 1
        found_key = tree:lookup_index(index)
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

function Keymap:bind(spec, func)
    if spec == nil then
        return
    end
    self._tree:insert(keyspec(spec), func)
end

return Keymap
