#if defined(HAS_SHADOWING)
float SampleCascadeShadowMap(const uint cascade, vec2 base, vec2 dudv, float depth, float bias) {
    return depth - bias > texture(sampler2DArray(cascade_shadow_map, cascade_shadow_sampler), vec3(base + dudv, cascade)).r ? 0.5 : 0.0;
}

float SampleCascadeShadowMapWithPCF() {
    vec3 view_position = (frame_uniforms.main_view_matrix * vec4(vertex_world_position, 1.0)).xyz;
    bvec4 greater_z = greaterThan(vec4(abs(view_position.z)), frame_uniforms.cascade_splits);
    uint cascade = clamp(uint(dot(vec4(greater_z), vec4(1.0))), 0u, SHADOW_CASCADE_COUNT - 1u);

    vec4 light_space_position = (frame_uniforms.sun_matrixs[cascade] * vec4(vertex_world_position, 1.0));
    vec3 proj_coords = light_space_position.xyz / light_space_position.w;
    proj_coords.xy = proj_coords.xy * 0.5 + 0.5;

    vec2 size = vec2(textureSize(sampler2DArray(cascade_shadow_map, cascade_shadow_sampler), 0));
    vec2 texel_size = vec2(1.0) / size;

    vec2 offset = vec2(0.5);
    vec2 uv = (proj_coords.xy * size) + offset;
    vec2 base = (floor(uv) - offset) * texel_size;
    vec2 st = fract(uv);

    vec2 uw = vec2(3.0 - 2.0 * st.x, 1.0 + 2.0 * st.x);
    vec2 vw = vec2(3.0 - 2.0 * st.y, 1.0 + 2.0 * st.y);

    vec2 u = vec2((2.0 - st.x) / uw.x - 1.0, st.x / uw.y + 1.0);
    vec2 v = vec2((2.0 - st.y) / vw.x - 1.0, st.y / vw.y + 1.0);

    u *= texel_size.x;
    v *= texel_size.y;

    float depth = proj_coords.z;

    float bias = 0.0001;
#if defined(HAS_ATTRIBUTE_NORMAL)
    bias = max(0.003 * (1.0 - dot(normalize(vertex_normal), normalize(frame_uniforms.sun_direction.xyz))), 0.0005);
#endif

    float sum = 0.0;

    sum += uw.x * vw.x * SampleCascadeShadowMap(cascade, base, vec2(u.x, v.x), depth, bias);
    sum += uw.y * vw.x * SampleCascadeShadowMap(cascade, base, vec2(u.y, v.x), depth, bias);

    sum += uw.x * vw.y * SampleCascadeShadowMap(cascade, base, vec2(u.x, v.y), depth, bias);
    sum += uw.y * vw.y * SampleCascadeShadowMap(cascade, base, vec2(u.y, v.y), depth, bias);

    return sum * (1.0 / 16.0);
}
#endif

void EvaluateDirectionalLight(const MaterialFragmentParams material_params, inout vec3 color) {
    color = material_params.base_color.xyz;

#if defined(HAS_ATTRIBUTE_NORMAL)
    color = max(dot(vertex_normal, -frame_uniforms.sun_direction.xyz), 0.0) * vec3(0.6);
#endif

#if defined(HAS_SHADOWING)
    float visibility = SampleCascadeShadowMapWithPCF();
    color *= (1.0 - visibility);
    color += 0.3;
#endif
}