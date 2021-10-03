add_library(GearEngine STATIC)

target_include_directories(GearEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_sources(GearEngine PRIVATE Source/GearEngine.h Source/GearEngine.cpp)

file(GLOB_RECURSE RENDERER_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Renderer/*.h)
file(GLOB_RECURSE RENDERER_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Renderer/*.cpp)
target_sources(GearEngine PRIVATE ${RENDERER_INC} ${RENDERER_SRC})

file(GLOB_RECURSE RENDER_PIPELINE_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/RenderPipeline/*.h)
file(GLOB_RECURSE RENDER_PIPELINE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/RenderPipeline/*.cpp)
target_sources(GearEngine PRIVATE ${RENDER_PIPELINE_INC} ${RENDER_PIPELINE_SRC})

file(GLOB_RECURSE RENDER_UTILITY_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/RenderUtility/*.h)
file(GLOB_RECURSE RENDER_UTILITY_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/RenderUtility/*.cpp)
target_sources(GearEngine PRIVATE ${RENDER_UTILITY_INC} ${RENDER_UTILITY_SRC})

file(GLOB_RECURSE ENTITY_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Entity/*.h)
file(GLOB_RECURSE ENTITY_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Entity/*.cpp)
target_sources(GearEngine PRIVATE ${ENTITY_INC} ${ENTITY_SRC})

file(GLOB_RECURSE RESOURCE_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Resource/*.h)
file(GLOB_RECURSE RESOURCE_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Resource/*.cpp)
target_sources(GearEngine PRIVATE ${RESOURCE_INC} ${RESOURCE_SRC})

file(GLOB_RECURSE UTILITY_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Utility/*.h)
file(GLOB_RECURSE UTILITY_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Utility/*.cpp)
target_sources(GearEngine PRIVATE ${UTILITY_INC} ${UTILITY_SRC})

file(GLOB_RECURSE MATERIAL_COMPILER_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/MaterialCompiler/*.h)
file(GLOB_RECURSE MATERIAL_COMPILER_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/MaterialCompiler/*.cpp)
target_sources(GearEngine PRIVATE ${MATERIAL_COMPILER_INC} ${MATERIAL_COMPILER_SRC})

file(GLOB_RECURSE INPUT_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Input/*.h)
file(GLOB_RECURSE INPUT_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Source/Input/*.cpp)
target_sources(GearEngine PRIVATE ${INPUT_INC} ${INPUT_SRC})

# blast
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/External/Blast Blast.out)
target_link_libraries(GearEngine PUBLIC Blast)

# window's uuid
target_link_libraries(GearEngine PUBLIC rpcrt4)

# file system
add_library(filesystem INTERFACE)
target_include_directories(filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/External/filesystem)
target_link_libraries(GearEngine PUBLIC filesystem)

# glm
add_library(glm INTERFACE)
target_compile_definitions(glm INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE=1)
target_include_directories(filesystem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/External/glm)
target_link_libraries(GearEngine PUBLIC glm)

# rapidjson
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/External/rapidjson/include)
target_link_libraries(GearEngine PUBLIC rapidjson)