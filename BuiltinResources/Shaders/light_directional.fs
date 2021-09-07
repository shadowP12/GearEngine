void EvaluateDirectionalLight(const MaterialFragmentParams material_params, inout vec3 color) {
    color = material_params.base_color.xyz;

#if defined(HAS_SHADOWING)
    //color *= texture(shadow_map, vertex_uv01.xy).xyz;
#endif
}