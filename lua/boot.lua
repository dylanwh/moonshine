function on_signal(sig)
	if (sig == 'SIGTERM' or sig == 'SIGHUP' or sig == 'SIGINT') then
		quit()
	end
end

function on_keypress(c)
end
