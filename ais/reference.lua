while true do
	print(position())
	print(velocity())
	print(heading())
	print(angular_velocity())
	thrust_main(10)
	thrust_lateral(5)
	thrust_angular(0.1)
	drive_towards(100, 0, 0)
	yield()
end
