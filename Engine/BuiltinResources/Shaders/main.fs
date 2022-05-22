layout(location = 0) out vec4 fragColor;

void PrepareShadingParams() {
    shading_view = normalize(frame_uniforms.view_position.xyz - vertex_world_position);

    #if defined(HAS_ATTRIBUTE_NORMAL)
    shading_normal = vertex_normal;
    #endif
}

void main() {
    PrepareShadingParams();

    MaterialFragmentParams material_params;

    InitMaterialFragmentParams(material_params);

    ProcessMaterialFragmentParams(material_params);
    
    fragColor = EvaluateMaterial(material_params);
}
