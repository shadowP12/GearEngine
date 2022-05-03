layout(location = LOCATION_POSITION) in vec4 mesh_position;

#if defined(HAS_ATTRIBUTE_NORMAL)
layout(location = LOCATION_NORMAL) in vec3 mesh_normal;
layout(location = LOCATION_TANGENT) in vec3 mesh_tangent;
layout(location = LOCATION_BITANGENT) in vec3 mesh_bitangent;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
layout(location = LOCATION_COLOR) in vec4 mesh_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0)
layout(location = LOCATION_UV0) in vec2 mesh_uv0;
#endif

#if defined(HAS_ATTRIBUTE_UV1)
layout(location = LOCATION_UV1) in vec2 mesh_uv1;
#endif

#if defined(HAS_ATTRIBUTE_BONE_INDICES)
layout(location = LOCATION_BONE_INDICES) in uvec4 mesh_bone_indices;
layout(location = LOCATION_BONE_WEIGHTS) in vec4 mesh_bone_weights;
#endif


#if defined(HAS_ATTRIBUTE_CUSTOM0)
layout(location = LOCATION_CUSTOM0) in vec4 mesh_custom0;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM1)
layout(location = LOCATION_CUSTOM1) in vec4 mesh_custom1;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM2)
layout(location = LOCATION_CUSTOM2) in vec4 mesh_custom2;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM3)
layout(location = LOCATION_CUSTOM3) in vec4 mesh_custom3;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM4)
layout(location = LOCATION_CUSTOM4) in vec4 mesh_custom4;
#endif

#if defined(HAS_ATTRIBUTE_CUSTOM5)
layout(location = LOCATION_CUSTOM5) in vec4 mesh_custom5;
#endif

layout(location = 0) out highp vec3 vertex_world_position;

layout(location = 1) out highp vec4 vertex_position;

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
layout(location = 5) out highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
layout(location = 5) out highp vec4 vertex_uv01;
#endif

#if defined(HAS_ATTRIBUTE_NORMAL)
layout(location = 6) out highp vec3 vertex_normal;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
layout(location = 7) out mediump vec4 vertex_color;
#endif
