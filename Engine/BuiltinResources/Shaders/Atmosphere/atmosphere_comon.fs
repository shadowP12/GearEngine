#version 450
#extension GL_ARB_separate_shader_objects : enable

uint TRANSMITTANCE_TEXTURE_WIDTH = 256;
uint TRANSMITTANCE_TEXTURE_HEIGHT = 64;

uint MULTI_SCATTERING_TEXTURE_SIZE = 32;

#define PI 3.1415926535897932384626433832795

#define saturate(x)        clamp(x, 0.0, 1.0)

layout(std140, set = 0, binding = 0) uniform AtmosphereCB {
    mat4 sky_inv_view_proj_mat;
    vec4 resolution;
    vec4 rayleigh_scattering;
    vec4 mie_scattering;
    vec4 mie_extinction;
    vec4 mie_absorption;
    vec4 absorption_extinction;
    vec4 ground_albedo;
    vec4 sun_direction;
    vec4 view_direction;
    vec4 view_position;
    float bottom_radius;
    float top_radius;
    float rayleigh_density_exp_scale;
    float mie_density_exp_scale;
    float mie_phase_g;
    float absorption_density0_layer_width;
    float absorption_density0_constant_term;
    float absorption_density0_linear_term;
    float absorption_density1_constant_term;
    float absorption_density1_linear_term;
} atmosphere_cb;

struct AtmosphereParameters {
    float bottom_radius;
    float top_radius;

    float rayleigh_density_exp_scale;
    vec3 rayleigh_scattering;

    float mie_density_exp_scale;
    vec3 mie_scattering;
    vec3 mie_extinction;
    vec3 mie_absorption;
    float mie_phase_g;

    float absorption_density0_layer_width;
    float absorption_density0_constant_term;
    float absorption_density0_linear_term;
    float absorption_density1_constant_term;
    float absorption_density1_linear_term;
    vec3 absorption_extinction;

    vec3 ground_albedo;

    vec3 sun_direction;
    vec3 view_direction;
    vec3 view_position;

    vec2 resolution;
    mat4 sky_inv_view_proj_mat;
};

AtmosphereParameters GetAtmosphereParameters()
{
    AtmosphereParameters atmosphere_parameters;
    atmosphere_parameters.bottom_radius = atmosphere_cb.bottom_radius;
    atmosphere_parameters.top_radius = atmosphere_cb.top_radius;

    atmosphere_parameters.rayleigh_density_exp_scale = atmosphere_cb.rayleigh_density_exp_scale;
    atmosphere_parameters.rayleigh_scattering = atmosphere_cb.rayleigh_scattering.xyz;

    atmosphere_parameters.mie_density_exp_scale = atmosphere_cb.mie_density_exp_scale;
    atmosphere_parameters.mie_scattering = atmosphere_cb.mie_scattering.xyz;
    atmosphere_parameters.mie_extinction = atmosphere_cb.mie_extinction.xyz;
    atmosphere_parameters.mie_absorption = atmosphere_cb.mie_absorption.xyz;
    atmosphere_parameters.mie_phase_g = atmosphere_cb.mie_phase_g;

    atmosphere_parameters.absorption_density0_layer_width = atmosphere_cb.absorption_density0_layer_width;
    atmosphere_parameters.absorption_density0_constant_term = atmosphere_cb.absorption_density0_constant_term;
    atmosphere_parameters.absorption_density0_linear_term = atmosphere_cb.absorption_density0_linear_term;
    atmosphere_parameters.absorption_density1_constant_term = atmosphere_cb.absorption_density1_constant_term;
    atmosphere_parameters.absorption_density1_linear_term = atmosphere_cb.absorption_density1_linear_term;
    atmosphere_parameters.absorption_extinction = atmosphere_cb.absorption_extinction.xyz;

    atmosphere_parameters.ground_albedo = atmosphere_cb.ground_albedo.xyz;

    atmosphere_parameters.sun_direction = atmosphere_cb.sun_direction.xyz;
    atmosphere_parameters.view_direction = atmosphere_cb.view_direction.xyz;
    atmosphere_parameters.view_position = atmosphere_cb.view_position.xyz;

    atmosphere_parameters.resolution = atmosphere_cb.resolution.xy;
    atmosphere_parameters.sky_inv_view_proj_mat = atmosphere_cb.sky_inv_view_proj_mat;

    return atmosphere_parameters;
}

float RaySphereIntersectNearest(vec3 r0, vec3 rd, vec3 s0, float sr)
{
    float a = dot(rd, rd);
    vec3 s0_r0 = r0 - s0;
    float b = 2.0 * dot(rd, s0_r0);
    float c = dot(s0_r0, s0_r0) - (sr * sr);
    float delta = b * b - 4.0*a*c;
    if (delta < 0.0 || a == 0.0)
    {
        return -1.0;
    }
    float sol0 = (-b - sqrt(delta)) / (2.0*a);
    float sol1 = (-b + sqrt(delta)) / (2.0*a);
    if (sol0 < 0.0 && sol1 < 0.0)
    {
        return -1.0;
    }
    if (sol0 < 0.0)
    {
        return max(0.0, sol1);
    }
    else if (sol1 < 0.0)
    {
        return max(0.0, sol0);
    }
    return max(0.0, min(sol0, sol1));
}

struct MediumSampleResult
{
    vec3 scattering;
    vec3 absorption;
    vec3 extinction;

    vec3 scattering_mie;
    vec3 absorption_mie;
    vec3 extinction_mie;

    vec3 scattering_ray;
    vec3 absorption_ray;
    vec3 extinction_ray;

    vec3 scattering_ozo;
    vec3 absorption_ozo;
    vec3 extinction_ozo;

    vec3 albedo;
};

vec3 GetAlbedo(vec3 scattering, vec3 extinction)
{
    return scattering / max(vec3(0.001), extinction);
}

MediumSampleResult SampleMedium(in vec3 world_pos, in AtmosphereParameters atmosphere)
{
    float view_height = length(world_pos) - atmosphere.bottom_radius;

    float density_mie = exp(atmosphere.mie_density_exp_scale * view_height);
    float density_ray = exp(atmosphere.rayleigh_density_exp_scale * view_height);
    float density_ozo = saturate(view_height < atmosphere.absorption_density0_layer_width ?
    atmosphere.absorption_density0_linear_term * view_height + atmosphere.absorption_density0_constant_term :
    atmosphere.absorption_density1_linear_term * view_height + atmosphere.absorption_density1_constant_term);

    MediumSampleResult s;

    s.scattering_mie = density_mie * atmosphere.mie_scattering;
    s.absorption_mie = density_mie * atmosphere.mie_absorption;
    s.extinction_mie = density_mie * atmosphere.mie_extinction;

    s.scattering_ray = density_ray * atmosphere.rayleigh_scattering;
    s.absorption_ray = vec3(0.0);
    s.extinction_ray = s.scattering_ray + s.absorption_ray;

    s.scattering_ozo = vec3(0.0);
    s.absorption_ozo = density_ozo * atmosphere.absorption_extinction;
    s.extinction_ozo = s.scattering_ozo + s.absorption_ozo;

    s.scattering = s.scattering_mie + s.scattering_ray + s.scattering_ozo;
    s.absorption = s.absorption_mie + s.absorption_ray + s.absorption_ozo;
    s.extinction = s.extinction_mie + s.extinction_ray + s.extinction_ozo;
    s.albedo = GetAlbedo(s.scattering, s.extinction);

    return s;
}

float RayleighPhase(float cos_theta)
{
	float factor = 3.0 / (16.0 * PI);
	return factor * (1.0 + cos_theta * cos_theta);
}

float MiePhase(float g, float cos_theta)
{
	float numer = 1.0f - g * g;
	float denom = 1.0f + g * g + 2.0f * g * cos_theta;
	return numer / (4.0 * PI * denom * sqrt(denom));
}

float FromUnitToSubUvs(float u, float resolution) { return (u + 0.5 / resolution) * (resolution / (resolution + 1.0)); }
float FromSubUvsToUnit(float u, float resolution) { return (u - 0.5 / resolution) * (resolution / (resolution - 1.0)); }

void UvToLutTransmittanceParams(AtmosphereParameters atmosphere, out float view_height, out float view_zenith_cos_angle, in vec2 uv)
{
    float x_mu = uv.x;
    float x_r = uv.y;

    float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
    float rho = H * x_r;
    view_height = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);

    float d_min = atmosphere.top_radius - view_height;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    view_zenith_cos_angle = d == 0.0 ? 1.0f : (H * H - rho * rho - d * d) / (2.0 * view_height * d);
    view_zenith_cos_angle = clamp(view_zenith_cos_angle, -1.0, 1.0);
}

void LutTransmittanceParamsToUv(AtmosphereParameters atmosphere, in float view_height, in float view_zenith_cos_angle, out vec2 uv)
{
	float H = sqrt(max(0.0, atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius));
	float rho = sqrt(max(0.0, view_height * view_height - atmosphere.bottom_radius * atmosphere.bottom_radius));

	float discriminant = view_height * view_height * (view_zenith_cos_angle * view_zenith_cos_angle - 1.0) + atmosphere.top_radius * atmosphere.top_radius;
	float d = max(0.0, (-view_height * view_zenith_cos_angle + sqrt(discriminant)));

	float d_min = atmosphere.top_radius - view_height;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;

	uv = vec2(x_mu, x_r);
}

vec3 GetSunLuminance(vec3 world_pos, vec3 world_dir, vec3 sun_dir, float planet_radius) {
	if (dot(world_dir, sun_dir) > cos(0.5 * 0.505 * 3.14159 / 180.0))
	{
		float t = RaySphereIntersectNearest(world_pos, world_dir, vec3(0.0, 0.0, 0.0), planet_radius);
		if (t < 0.0)
		{
		    // Todo: Blend
			vec3 sun_luminance = vec3(1000000.0);
			return sun_luminance;
		}
	}
	return vec3(0);
}