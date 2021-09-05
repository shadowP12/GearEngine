struct MaterialFragmentParams {
    vec4  base_color;
};

void InitMaterialFragmentParams(out MaterialFragmentParams params) {
    params.base_color = vec4(1.0);
}