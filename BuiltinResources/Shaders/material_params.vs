struct MaterialVertexParams {
    vec4 world_position;

#ifdef HAS_ATTRIBUTE_NORMAL
    vec3 world_normal;
#endif

#ifdef HAS_ATTRIBUTE_COLOR
    vec4 color;
#endif

#ifdef HAS_ATTRIBUTE_UV0
    vec2 uv0;
#endif

#ifdef HAS_ATTRIBUTE_UV1
    vec2 uv1;
#endif
};

void InitMaterialVertexParams(out MaterialVertexParams params) {
    params.world_position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * object_uniforms.model_matrix * mesh_position;

#ifdef HAS_ATTRIBUTE_COLOR
    params.color = mesh_color;
#endif

#ifdef HAS_ATTRIBUTE_UV0
    params.uv0 = mesh_uv0;
#endif

#ifdef HAS_ATTRIBUTE_UV1
    params.uv1 = mesh_uv1;
#endif
}
