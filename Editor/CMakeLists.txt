add_executable (GearEditor main.cpp)
target_include_directories(GearEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(GearEditor PRIVATE GearEngine)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../External/glfw EXCLUDE_FROM_ALL glfw.out)
target_link_libraries(GearEditor PRIVATE glfw)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../External/stb)
target_link_libraries(GearEditor PRIVATE stb)

# cgltf
add_library(cgltf INTERFACE)
target_include_directories(filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../External/cgltf)
target_link_libraries(GearEditor PRIVATE cgltf)

# builtin resources
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resource DESTINATION ${CMAKE_CURRENT_BINARY_DIR})