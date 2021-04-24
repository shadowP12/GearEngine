add_library(GearEngine STATIC)

target_include_directories(GearEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(GearEngine PRIVATE GearEngine.h GearEngine.cpp)

file(GLOB_RECURSE RENDERER_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Renderer/*.h)
file(GLOB_RECURSE RENDERER_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Renderer/*.cpp)
target_sources(GearEngine PRIVATE ${RENDERER_INC} ${RENDERER_SRC})

file(GLOB_RECURSE RESOURCE_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Resource/*.h)
file(GLOB_RECURSE RESOURCE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Resource/*.cpp)
target_sources(GearEngine PRIVATE ${RESOURCE_INC} ${RESOURCE_SRC})

file(GLOB_RECURSE UTILITY_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Utility/*.h)
file(GLOB_RECURSE UTILITY_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Utility/*.cpp)
target_sources(GearEngine PRIVATE ${UTILITY_INC} ${UTILITY_SRC})

file(GLOB_RECURSE MATERIAL_COMPILER_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/MaterialCompiler/*.h)
file(GLOB_RECURSE MATERIAL_COMPILER_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/MaterialCompiler/*.cpp)
target_sources(GearEngine PRIVATE ${MATERIAL_COMPILER_INC} ${MATERIAL_COMPILER_SRC})

target_compile_definitions(GearEngine PRIVATE GEAR_EXPORTS)
target_compile_definitions(GearEngine PRIVATE CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON)
target_compile_definitions(GearEngine PRIVATE BUILD_SHARED_LIBS=TRUE)

# blast
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../External/Blast Blast.out)
target_link_libraries(GearEngine PUBLIC Blast)

# window's uuid
target_link_libraries(GearEngine PUBLIC rpcrt4)

# file system
add_library(filesystem INTERFACE)
target_include_directories(filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../External/filesystem)
target_link_libraries(GearEngine PUBLIC filesystem)

# glm
add_library(glm INTERFACE)
target_include_directories(filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../External/glm)
target_link_libraries(GearEngine PUBLIC glm)