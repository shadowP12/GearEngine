add_executable (GearEditor main.cpp)
target_include_directories(GearEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(GearEditor PRIVATE GearEngine)

# builtin resources
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Resource DESTINATION ${CMAKE_CURRENT_BINARY_DIR})