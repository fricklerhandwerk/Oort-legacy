if N ~= 1 then
	error("this scenario only supports 1 team")
end

team("blue",  0x0000FF00)
team("green", 0x00FF0000)

ship("mothership", AI[0], "green", -10, 0)
ship("mothership", "examples/target.lua", "blue", 10, 0)
