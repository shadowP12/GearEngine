void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    params.base_color.rgb = texture(samplerCube(skybox_map, skybox_sampler), vertex_world_position).rgb;
    params.base_color.a = 1.0;
}