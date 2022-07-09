void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    params.metallic = 0.0;
    params.roughness = 1.0;
    params.base_color = vec4(0.1, 0.22, 0.6, 0.6);
    params.base_color.rgb *= params.base_color.a;
}