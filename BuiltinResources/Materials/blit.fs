void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    params.base_color.rgba = texture(sampler2D(src_texture, blit_sampler), vertex_uv01.xy);
}