layout(location = 4) in highp vec3 vertex_world_position;

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
layout(location = 5) in highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
layout(location = 5) in highp vec4 vertex_uv01;
#endif