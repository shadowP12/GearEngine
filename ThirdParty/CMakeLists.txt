add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glm/include)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/stb)

add_library(vulkan_header INTERFACE)
target_include_directories(vulkan_header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/vulkan/include)

add_library(glfw_header INTERFACE)
target_include_directories(glfw_header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glfw/include)

add_library(glslang_header INTERFACE)
target_include_directories(glslang_header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/include)

add_library(spirv_cross_header INTERFACE)
target_include_directories(spirv_cross_header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/include)

add_library(vulkan INTERFACE)
target_link_libraries(vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/vulkan/lib/win32/vulkan-1.lib)

add_library(glfw INTERFACE)

add_library(glslang INTERFACE)

add_library(spirv_cross INTERFACE)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_link_libraries(glfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glfw/lib/win32/debug/glfw3.lib)

    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/debug/glslangd.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/debug/HLSLd.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/debug/OGLCompilerd.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/debug/OSDependentd.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/debug/SPIRVd.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/debug/SPVRemapperd.lib)

    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-cd.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-cored.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-cppd.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-glsld.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-hlsld.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-msld.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-reflectd.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/debug/spirv-cross-utild.lib)

else()
    target_link_libraries(glfw INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glfw/lib/win32/release/glfw3.lib)

    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/release/glslang.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/release/HLSL.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/release/OGLCompiler.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/release/OSDependent.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/release/SPIRV.lib)
    target_link_libraries(glslang INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/glslang/lib/win32/release/SPVRemapper.lib)

    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-c.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-core.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-cpp.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-glsl.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-hlsl.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-msl.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-reflect.lib)
    target_link_libraries(spirv_cross INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/GearLibs/spirv_cross/lib/win32/release/spirv-cross-util.lib)
endif()