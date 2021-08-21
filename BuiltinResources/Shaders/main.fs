layout(location = 0) out vec4 fragColor;

void main() {
    MaterialFragmentParams material_params;

    InitMaterialFragmentParams(material_params);

    ProcessMaterialFragmentParams(material_params);
    
    fragColor = material_params.base_color;
}
