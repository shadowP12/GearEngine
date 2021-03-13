add_library(GearEngine SHARED)
message(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(GearEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE UTILITY_INC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Utility/*.h)
file(GLOB_RECURSE UTILITY_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/Utility/*.cpp)
target_sources(GearEngine PRIVATE ${UTILITY_INC} ${UTILITY_SRC})

target_compile_definitions(GearEngine PRIVATE GEAR_EXPORTS)