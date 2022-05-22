#if defined(HAS_SHADOWING)

float SampleCascadeShadowMap(const uint cascade, vec2 base, vec2 dudv, float depth, float bias) {
    return depth - bias > texture(sampler2DArray(cascade_shadow_map, cascade_shadow_sampler), vec3(base + dudv, cascade)).r ? 0.5 : 0.0;
}

float SampleCascadeShadowMapWithPCF() {
    vec3 view_position = (frame_uniforms.main_view_matrix * vec4(vertex_world_position, 1.0)).xyz;
    bvec3 greater_z = greaterThan(vec3(abs(view_position.z)), frame_uniforms.cascade_splits.xyz);
    uint cascade = clamp(uint(dot(vec3(greater_z), vec3(1.0))), 0u, SHADOW_CASCADE_COUNT - 1u);

    vec4 light_space_position = (frame_uniforms.sun_matrixs[cascade] * vec4(vertex_world_position, 1.0));
    vec3 proj_coords = light_space_position.xyz / light_space_position.w;
    proj_coords.xy = proj_coords.xy * 0.5 + 0.5;

    vec2 size = vec2(textureSize(sampler2DArray(cascade_shadow_map, cascade_shadow_sampler), 0));
    vec2 texel_size = vec2(1.0) / size;

	vec2 uv = proj_coords.xy;
    float depth = proj_coords.z;

    float bias = 0.01;
#if defined(HAS_ATTRIBUTE_NORMAL)
    bias = max(0.06 * (1.0 - dot(normalize(vertex_normal), normalize(frame_uniforms.sun_direction.xyz))), 0.01);
#endif

    float sum = 0.0;

    sum += SampleCascadeShadowMap(cascade, uv, vec2(0.0, 0.0), depth, bias);

    sum += SampleCascadeShadowMap(cascade, uv, vec2(texel_size.x, 0.0), depth, bias);
    sum += SampleCascadeShadowMap(cascade, uv, vec2(-texel_size.x, 0.0), depth, bias);

    sum += SampleCascadeShadowMap(cascade, uv, vec2(0.0, texel_size.y), depth, bias);
    sum += SampleCascadeShadowMap(cascade, uv, vec2(0.0, -texel_size.y), depth, bias);

    sum += SampleCascadeShadowMap(cascade, uv, vec2(texel_size.x, texel_size.y), depth, bias);
    sum += SampleCascadeShadowMap(cascade, uv, vec2(texel_size.x, -texel_size.y), depth, bias);

    sum += SampleCascadeShadowMap(cascade, uv, vec2(-texel_size.x, texel_size.y), depth, bias);
    sum += SampleCascadeShadowMap(cascade, uv, vec2(-texel_size.x, -texel_size.y), depth, bias);

    return sum * (1.0 / 9.0);
}
#endif

void EvaluateDirectionalLight(const MaterialFragmentParams material_params, inout vec3 color) {
#if !defined(HAS_ATTRIBUTE_NORMAL)
    return;
#endif

    float visibility = 1.0;
#if defined(HAS_SHADOWING)
    visibility = SampleCascadeShadowMapWithPCF();
#endif

    Light light;
    light.color_intensity.rgb = frame_uniforms.sun_color_intensity.rgb;
    light.color_intensity.w = frame_uniforms.sun_color_intensity.w * frame_uniforms.exposure;
    light.attenuation = 1.0;
    light.L = -frame_uniforms.sun_direction.xyz;

    color.rgb += SurfaceShading(material_params, light, visibility);
}