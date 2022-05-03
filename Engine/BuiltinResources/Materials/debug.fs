void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    params.base_color = vertex_color;
    params.base_color.rgb *= params.base_color.a;
}