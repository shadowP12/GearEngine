void main() {
    MaterialVertexParams material_params;

    InitMaterialVertexParams(material_params);

    ProcessMaterialVertexParams(material_params);

    vertex_position = mesh_position;

#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material_params.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material_params.uv1;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
    vertex_color = material_params.color;
#endif

    gl_Position = material_params.world_position;
}
