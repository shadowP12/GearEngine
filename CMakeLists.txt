cmake_minimum_required (VERSION 3.10)
project ("GearEngine")
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	message("Debug")
else()
	message("Release")
endif()

add_subdirectory (GearEngine)