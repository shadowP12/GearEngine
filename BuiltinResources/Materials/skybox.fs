void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    params.base_color = vertex_color * texture(skybox, vertex_world_position);
    params.base_color.rgb *= params.base_color.a;
    params.base_color.rgba = vec4(1.0);
}