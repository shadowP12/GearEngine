include_directories(Source)

include(Source/Sources.cmake)
#add_definitions(-D VK_PROTOTYPES)
#add_definitions(-D VK_USE_PLATFORM_WIN32_KHR)
#add_definitions(-D NOMINMAX)
add_compile_options(/wd26812)
add_library(GearEngine STATIC ${GEAR_ENGINE_SRC})

# builtin resources
message(${CMAKE_INSTALL_PREFIX})
message(${CMAKE_CURRENT_SOURCE_DIR}/Resource)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resource DESTINATION ${CMAKE_INSTALL_PREFIX})