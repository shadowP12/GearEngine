vec3 FresnelSchlickRoughness(float cos_theta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cos_theta, 5.0);
}

#if defined(HAS_IBL)
void EvaluateIBL(const MaterialFragmentParams material_params, inout vec3 color) {
#if defined(HAS_ATTRIBUTE_NORMAL)
    // Todo: Flexible f0
    vec3 F0 = vec3(material_params.metallic);
    vec3 N = normalize(vertex_normal);
	vec3 V = normalize(frame_uniforms.view_position.xyz - vertex_world_position);
	vec3 R = reflect(-V, N);
	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, material_params.roughness);
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - material_params.metallic;

	vec3 irradiance = texture(samplerCube(irradiance_map, ibl_sampler), N).rgb;
	vec3 diffuse = kD * irradiance * material_params.base_color.rgb;

	vec3 prefiltered_color = textureLod(samplerCube(prefiltered_map, ibl_sampler), R, uint(material_params.roughness * 4)).rgb;
	vec2 brdf = texture(sampler2D(brdf_lut, ibl_sampler), vec2(max(dot(N, V), 0.0), material_params.roughness)).rg;
	vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);
	vec3 ambient = diffuse + specular;
	
	color += ambient * 0.3;
#endif
}
#endif