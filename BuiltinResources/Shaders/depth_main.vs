void main() {
    MaterialVertexParams material_params;

    InitMaterialVertexParams(material_params);

    ProcessMaterialVertexParams(material_params);

    gl_Position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * material_params.world_position;
}
