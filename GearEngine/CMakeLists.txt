include_directories(Source)
include_directories(ThirdParty/glm)
include_directories(ThirdParty/glfw/include)
include_directories(ThirdParty/Utility)

set(GEAR_ENGINE_SRC
	Source/main.cpp
	)
add_executable (GearEngine 
				${GEAR_ENGINE_SRC}
				)