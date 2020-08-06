add_executable (Example main.cpp)
target_link_libraries(Example GearEngine)
target_link_libraries(Example vulkan)
target_link_libraries(Example glfw)
target_link_libraries(Example glslang)
target_link_libraries(Example spirv_cross)

# builtin resources
message(${CMAKE_INSTALL_PREFIX})
message(${CMAKE_CURRENT_SOURCE_DIR}/Resource)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resource DESTINATION ${CMAKE_INSTALL_PREFIX}/Example)