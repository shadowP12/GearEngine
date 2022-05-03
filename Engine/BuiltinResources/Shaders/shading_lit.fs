vec4 EvaluateLights(const MaterialFragmentParams material_params) {
    vec3 color = vec3(0.0);

#if defined(HAS_DIRECTIONAL_LIGHTING)
    EvaluateDirectionalLight(material_params, color);
#endif

#if defined(HAS_DYNAMIC_LIGHTING)
    EvaluatePunctualLights(material_params, color);
#endif

#if defined(HAS_IBL)
    EvaluateIBL(material_params, color);
#endif

    return vec4(color, material_params.base_color.a);
}

vec4 EvaluateMaterial(const MaterialFragmentParams material_params) {
    vec4 color = EvaluateLights(material_params);
    return color;
}