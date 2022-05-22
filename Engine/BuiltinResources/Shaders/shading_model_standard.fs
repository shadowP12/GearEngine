
vec3 SpecularLobe(const MaterialFragmentParams params, vec3 H, float NoV, float NoL, float NoH, float LoH) {
    // Todo: Flexible f0
    float D = Distribution(params.roughness, NoH, H);
    float G = Visibility(params.roughness, NoV, NoL);
    vec3  F = Fresnel(vec3(params.metallic), LoH);
    return (D * G * F) / (4.0 * max(NoV, 0.0) * max(NoL, 0.0) + 0.001);
}

vec3 DiffuseLobe(const MaterialFragmentParams params, float NoV, float NoL, float LoH) {
    vec3 diffuse_color = params.base_color.rgb * (1.0 - params.metallic);
    return diffuse_color * Diffuse(params.roughness, NoV, NoL, LoH);
}

vec3 SurfaceShading(const MaterialFragmentParams params, const Light light, float visibility) {
    vec3 H = normalize(shading_view + light.L);
    float NoV = saturate(dot(shading_normal, shading_view));
    float NoL = saturate(dot(shading_normal, light.L));
    float NoH = saturate(dot(shading_normal, H));
    float LoH = saturate(dot(light.L, H));

    vec3 Fr = SpecularLobe(params, H, NoV, NoL, NoH, LoH);
    vec3 Fd = DiffuseLobe(params, NoV, NoL, LoH);

    vec3 color = Fd + Fr;

    return color * light.color_intensity.rgb * light.color_intensity.w * light.attenuation * NoL * (1.0 - visibility);
}