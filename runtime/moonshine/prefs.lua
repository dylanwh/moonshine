local ok, prefs = pcall(include, os.getenv("HOME") .. "/.moonshine/prefs")

if not ok then prefs = {} end

return prefs
