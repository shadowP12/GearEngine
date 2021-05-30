struct MaterialVertexInputs {
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

void initMaterialVertex(out MaterialVertexInputs material) {
    material.world_position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * object_uniforms.model_matrix * vec4(mesh_position, 1.0);
}
