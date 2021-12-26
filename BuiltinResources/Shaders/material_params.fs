struct MaterialFragmentParams {
    vec4  base_color;
    float roughness;
    float metallic;
};

void InitMaterialFragmentParams(out MaterialFragmentParams params) {
    params.base_color = vec4(1.0);
    params.roughness = 1.0;
    params.metallic = 0.5;
}