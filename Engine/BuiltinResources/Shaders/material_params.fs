struct MaterialFragmentParams {
    vec4 base_color;
    vec3 specular_color;
    float roughness;
    float metallic;
};

void InitMaterialFragmentParams(out MaterialFragmentParams params) {
    params.base_color = vec4(1.0);
    params.specular_color = vec3(1.0);
    params.roughness = 0.5;
    params.metallic = 0.5;
}