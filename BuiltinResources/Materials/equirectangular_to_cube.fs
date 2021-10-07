vec2 SampleSphericalMap(vec3 v) {
    vec2 inv_atan = vec2(0.1591, 0.3183);
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    return uv;
}

void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {
    
    vec2 uv = SampleSphericalMap(normalize(vertex_world_position));
    params.base_color.rgb = texture(equirectangular_map, uv).rgb;
    params.base_color.a = 1.0;
}