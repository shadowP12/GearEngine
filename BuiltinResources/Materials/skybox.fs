void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    params.base_color.rgb = texture(skybox, vertex_world_position).rgb;
    params.base_color.a = 1.0;
}