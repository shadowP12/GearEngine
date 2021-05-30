layout(location = 0) out vec4 fragColor;

void main() {
    MaterialFragmentInputs inputs;
    initMaterialFragment(inputs);

    materialFragment(inputs);
    // todo
    fragColor = inputs.base_color;
}
