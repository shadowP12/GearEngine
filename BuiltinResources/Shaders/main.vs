void main() {
    MaterialVertexParams material_params;

    InitMaterialVertexParams(material_params);

    ProcessMaterialVertexParams(material_params);

    vertex_position = frame_uniforms.view_matrix * material_params.world_position;

    vertex_world_position = material_params.world_position.xyz;

#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material_params.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material_params.uv1;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
    vertex_color = material_params.color;
#endif

#if defined(HAS_ATTRIBUTE_NORMAL)
    vertex_normal = material_params.world_normal;
#endif

    gl_Position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * material_params.world_position;
}
