add_library(GearEngine STATIC)

target_include_directories(GearEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE RENDERER_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Renderer/*.h)
file(GLOB_RECURSE RENDERER_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Renderer/*.cpp)
target_sources(GearEngine PRIVATE ${RENDERER_INC} ${RENDERER_SRC})

file(GLOB_RECURSE UTILITY_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Utility/*.h)
file(GLOB_RECURSE UTILITY_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Utility/*.cpp)
target_sources(GearEngine PRIVATE ${UTILITY_INC} ${UTILITY_SRC})

target_compile_definitions(GearEngine PRIVATE GEAR_EXPORTS)
target_compile_definitions(GearEngine PRIVATE CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON)
target_compile_definitions(GearEngine PRIVATE BUILD_SHARED_LIBS=TRUE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../External/Blast Blast.out)
target_link_libraries(GearEngine PRIVATE Blast)