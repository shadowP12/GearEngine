void main() {
    MaterialVertexInputs material;
    initMaterialVertex(material);

    materialVertex(material);

    vertex_position = mesh_position;

#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material.uv1;
#endif

    gl_Position = material.world_position;
}
