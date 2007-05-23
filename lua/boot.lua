
function on.boot()
	-- first event.
end


function on.signal(sig)
	if (sig == 'SIGTERM' or sig == 'SIGHUP' or sig == 'SIGINT') then
		quit()
	end
end
