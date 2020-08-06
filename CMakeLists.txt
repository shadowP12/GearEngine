cmake_minimum_required (VERSION 3.10)
project ("GearEngine")

add_subdirectory(ThirdParty)
add_subdirectory (GearEngine)

if(DEFINED EnableExample)
    add_subdirectory(Example)
endif()