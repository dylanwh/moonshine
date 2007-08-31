-- GLOBALS: cmd, eval(). 


cmd = {}

function eval(text)
	ui:print("<you> %|%1", text)
	ui:render()
end
