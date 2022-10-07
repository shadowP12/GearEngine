layout(binding = 1000) uniform texture2D transmittance_texture;
layout(binding = 1001) uniform texture2D multi_scatt_texture;
layout(binding = 1002) uniform texture2D depth_texture;
layout(binding = 3000) uniform sampler default_sampler;

layout(location = 0) out vec4 out_color;

vec3 GetTransmittance(AtmosphereParameters atmosphere, vec3 world_pos, float view_zenith_cos_angle) {
    float view_height = length(world_pos);
    vec2 uv;
    LutTransmittanceParamsToUv(atmosphere, view_height, view_zenith_cos_angle, uv);
    vec3 transmittance = textureLod(sampler2D(transmittance_texture, default_sampler), uv, 0.0).rgb;
    return transmittance;
}

vec3 GetMultipleScattering(AtmosphereParameters atmosphere, vec3 world_pos, float view_zenith_cos_angle) {
	vec2 uv = vec2(view_zenith_cos_angle * 0.5 + 0.5, (length(world_pos) - atmosphere.bottom_radius) / (atmosphere.top_radius - atmosphere.bottom_radius));
	uv = vec2(clamp(uv.x, 0.0, 1.0), clamp(uv.y, 0.0, 1.0));
	uv = vec2(FromUnitToSubUvs(uv.x, MULTI_SCATTERING_TEXTURE_SIZE), FromUnitToSubUvs(uv.y, MULTI_SCATTERING_TEXTURE_SIZE));
	vec3 multi_scattered_luminance = textureLod(sampler2D(multi_scatt_texture, default_sampler), uv, 0.0).rgb;
	return multi_scattered_luminance;
}

struct SingleScatteringResult
{
	vec3 luminance;
};

SingleScatteringResult IntegrateScatteredLuminance(
    in vec2 pix_pos, in vec3 world_pos, in vec3 world_dir, in vec3 sun_dir,
    in AtmosphereParameters atmosphere, in float sample_count, in float t_max_max)
{
    SingleScatteringResult result;

    vec3 planet0 = vec3(0.0, 0.0, 0.0);
    float t_bottom = RaySphereIntersectNearest(world_pos, world_dir, planet0, atmosphere.bottom_radius);
    float t_top = RaySphereIntersectNearest(world_pos, world_dir, planet0, atmosphere.top_radius);
    float t_max = 0.0;
    if (t_bottom < 0.0)
    {
        if (t_top < 0.0f)
        {
            // No intersect
            t_max = 0.0f;
            return result;
        }
        else
        {
            t_max = t_top;
        }
    }
    else
    {
        if (t_top > 0.0f)
        {
            t_max = min(t_top, t_bottom);
        }
    }

    t_max = min(t_max, t_max_max);

    // Phase functions
	vec3 wi = sun_dir;
	vec3 wo = world_dir;
	float cos_theta = dot(wi, wo);
	float mie_phase_value = MiePhase(atmosphere.mie_phase_g, -cos_theta);
	float rayleigh_phase_value = RayleighPhase(cos_theta);

    // Placeholder
    vec3 global_l = vec3(1.0);

    // Ray march
    vec3 luminance = vec3(0.0);
    vec3 throughput = vec3(1.0);
    float dt = t_max / sample_count;
    float t = 0.0;
    float t_prev = 0.0;
    for (float s = 0.0f; s < sample_count; s += 1.0)
    {
        t = dt * s;
        vec3 p = world_pos + t * world_dir;
        float p_height = length(p);
        vec3 up_vector = p / p_height;
        float sun_zenith_cos_angle = dot(sun_dir, up_vector);

        // Sample medium
        MediumSampleResult medium = SampleMedium(p, atmosphere);
        vec3 sample_optical_depth = medium.extinction * dt;
        vec3 sample_transmittance = exp(-sample_optical_depth);

        // Transmittance
        vec3 transmittance = GetTransmittance(atmosphere, p, sun_zenith_cos_angle);

        // Multi scatt
        vec3 multi_scattered_luminance = GetMultipleScattering(atmosphere, p, sun_zenith_cos_angle);

        // Earth shadow
        float t_earth = RaySphereIntersectNearest(p, sun_dir, planet0 + 0.01 * up_vector, atmosphere.bottom_radius);
        float earth_shadow = t_earth >= 0.0 ? 0.0 : 1.0;

        // Todo: Sample shadowmap
        float shadow = 1.0;

        vec3 phase_times_scattering = medium.scattering_mie * mie_phase_value + medium.scattering_ray * rayleigh_phase_value;
        luminance += global_l * (earth_shadow * shadow * transmittance * phase_times_scattering + multi_scattered_luminance * medium.scattering) * dt;

        throughput *= sample_transmittance;

        t_prev = t;
    }
    result.luminance = luminance;
    return result;
}

void main() {
    AtmosphereParameters atmosphere = GetAtmosphereParameters();

    vec2 pix_pos = gl_FragCoord.xy;
    vec2 uv = pix_pos / atmosphere.resolution;
	vec3 ndc = vec3((pix_pos / atmosphere.resolution) * vec2(2.0, 2.0) - vec2(1.0, 1.0), 1.0);
	vec4 target_pos = atmosphere.sky_inv_view_proj_mat * vec4(ndc, 1.0);
    target_pos = target_pos / target_pos.w;

    vec3 sun_dir = -atmosphere.sun_direction;
	vec3 world_dir = normalize(target_pos.xyz - atmosphere.view_position);
	vec3 world_pos = atmosphere.view_position + vec3(0.0, atmosphere.bottom_radius, 0.0);
    float view_height = length(world_pos);

    vec3 luminance = vec3(0.0);

    float depth_buffer_value = textureLod(sampler2D(depth_texture, default_sampler), uv, 0.0).r;
    if (depth_buffer_value >= 0.99) {
        luminance += GetSunLuminance(world_pos, world_dir, sun_dir, atmosphere.bottom_radius);
    }

    float sample_count = 30.0;
    float t_max_max = 9000000;
    SingleScatteringResult result = IntegrateScatteredLuminance(pix_pos, world_pos, world_dir, sun_dir, atmosphere, sample_count, t_max_max);
    luminance += result.luminance;

	vec3 white_point = vec3(1.08241, 0.96756, 0.95003);
	float exposure = 10.0;
	luminance = pow(vec3(1.0) - exp(-luminance / white_point * exposure), vec3(1.0 / 2.2));

    out_color = vec4(luminance, 1.0);
}