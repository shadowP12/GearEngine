void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    params.base_color = vertex_color * texture(sampler2D(albedo_texture, albedo_sampler), vertex_uv01.xy);
    params.base_color.rgb *= params.base_color.a;
    params.base_color.rgba = vec4(1.0);
}