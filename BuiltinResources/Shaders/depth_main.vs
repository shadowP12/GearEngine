void main() {
    MaterialVertexParams material_params;

    InitMaterialVertexParams(material_params);

    ProcessMaterialVertexParams(material_params);

    gl_Position = material_params.world_position;
}
