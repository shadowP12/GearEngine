void EvaluateDirectionalLight(const MaterialFragmentParams material_params, inout vec3 color) {
    color = material_params.base_color.xyz;

#if defined(HAS_ATTRIBUTE_NORMAL)
    color = max(dot(vertex_normal, -frame_uniforms.sun_direction.xyz), 0.0) * vec3(0.6);
#endif

#if defined(HAS_SHADOWING)
    vec3 view_position = (frame_uniforms.view_matrix * vec4(vertex_world_position, 1.0)).xyz;
    bvec4 greater_z = greaterThan(vec4(abs(view_position.z)), frame_uniforms.cascade_splits);
    uint cascade = clamp(uint(dot(vec4(greater_z), vec4(1.0))), 0u, SHADOW_CASCADE_COUNT - 1u);

    vec4 light_space_position = (frame_uniforms.sun_matrixs[cascade] * vec4(vertex_world_position, 1.0));
    vec3 proj_coords = light_space_position.xyz / light_space_position.w;
    proj_coords.xy = proj_coords.xy * 0.5 + 0.5;
    float closest_depth = texture(cascade_shadow_map, vec3(proj_coords.x, proj_coords.y, cascade)).r; 
    float current_depth = clamp(proj_coords.z, 0.0, 1.0);

    float bias = 0.0005;
#if defined(HAS_ATTRIBUTE_NORMAL)
    bias = max(0.003 * (1.0 - dot(normalize(vertex_normal), normalize(frame_uniforms.sun_direction.xyz))), 0.0005);
#endif

    float visibility = current_depth - bias > closest_depth ? 0.5 : 0.0;
    color *= (1.0 - visibility);
    color += 0.3;
#endif
}