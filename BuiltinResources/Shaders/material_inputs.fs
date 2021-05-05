struct MaterialFragmentInputs {
    vec4  base_color;
};

void initMaterialFragment(out MaterialFragmentInputs material) {
    material.base_color = vec4(1.0);
}