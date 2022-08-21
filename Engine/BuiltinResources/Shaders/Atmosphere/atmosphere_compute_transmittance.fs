#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec3 transmittance;

struct DensityProfileLayer {
    float width;
    float exp_term;
    float exp_scale;
    float linear_term;
    float constant_term;
};

struct DensityProfile {
    DensityProfileLayer layers[2];
};

layout(push_constant) uniform AtmosphereParameters {
    float bottom_radius;
    float top_radius;
    vec3 solar_irradiance;
    vec3 ground_albedo;
    vec3 rayleigh_scattering;
    vec3 mie_scattering;
    vec3 mie_extinction;
    vec3 absorption_extinction;
    DensityProfile rayleigh_density;
    DensityProfile mie_density;
    DensityProfile absorption_density;
} atmosphere_parameters;

#define TRANSMITTANCE_TEXTURE_WIDTH 256
#define TRANSMITTANCE_TEXTURE_HEIGHT 64

// [0, 1] <-> [0.5/n, (n-0.5)/n]
float GetTextureCoordFromUnitRange(float x, int texture_size) {
    return 0.5 / float(texture_size) + x * (1.0 - 1.0 / float(texture_size));
}

float GetUnitRangeFromTextureCoord(float u, int texture_size) {
    return (u - 0.5 / float(texture_size)) / (1.0 - 1.0 / float(texture_size));
}

void GetRMuFromTransmittanceTextureUv(in AtmosphereParameters atmosphere, in vec2 uv, out float r, out float mu) {
    float x_mu = GetUnitRangeFromTextureCoord(uv.x, TRANSMITTANCE_TEXTURE_WIDTH);
    float x_r = GetUnitRangeFromTextureCoord(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT);

    float H = sqrt(atmosphere.top_radius * atmosphere.top_radius - atmosphere.bottom_radius * atmosphere.bottom_radius);
    float rho = H * x_r;
    r = sqrt(rho * rho + atmosphere.bottom_radius * atmosphere.bottom_radius);

    float d_min = atmosphere.top_radius - r;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    mu = d == 0.0 ? 1.0 : (H * H - rho * rho - d * d) / (2.0 * r * d);

    // [-1, 1]
    mu = clamp(mu, -1.0, 1.0);
}

float DistanceToTopAtmosphereBoundary(in AtmosphereParameters atmosphere, float r, float mu) {
    float discriminant = r * r * (mu * mu - 1.0) + atmosphere.top_radius * atmosphere.top_radius;
    return max(-r * mu + sqrt(max(discriminant, 0.0)), 0.0);
}

float GetLayerDensity(in DensityProfileLayer layer, float altitude) {
    float density = layer.exp_term * exp(layer.exp_scale * altitude) + layer.linear_term * altitude + layer.constant_term;
    return clamp(density, 0.0, 1.0);
}

float GetProfileDensity(in DensityProfile profile, float altitude) {
    return altitude < profile.layers[0].width ?
    GetLayerDensity(profile.layers[0], altitude) :
    GetLayerDensity(profile.layers[1], altitude);
}

float ComputeOpticalLengthToTopAtmosphereBoundary(in AtmosphereParameters atmosphere, in DensityProfile profile, float r, float mu) {
    const int SAMPLE_COUNT = 500;
    float dx = DistanceToTopAtmosphereBoundary(atmosphere, r, mu) / float(SAMPLE_COUNT);

    float result = 0.0;
    for (int i = 0; i <= SAMPLE_COUNT; ++i) {
        float d_i = float(i) * dx;
        float r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
        float y_i = GetProfileDensity(profile, r_i - atmosphere.bottom_radius);
        float weight_i = i == 0 || i == SAMPLE_COUNT ? 0.5 : 1.0;
        result += y_i * weight_i * dx;
    }
    return result;
}

vec3 ComputeTransmittanceToTopAtmosphereBoundary(in AtmosphereParameters atmosphere, float r, float mu) {
    // rayleigh_scattering = rayleigh_extinction
    return exp(-(
    atmosphere.rayleigh_scattering *
    ComputeOpticalLengthToTopAtmosphereBoundary(
    atmosphere, atmosphere.rayleigh_density, r, mu) +
    atmosphere.mie_extinction *
    ComputeOpticalLengthToTopAtmosphereBoundary(
    atmosphere, atmosphere.mie_density, r, mu) +
    atmosphere.absorption_extinction *
    ComputeOpticalLengthToTopAtmosphereBoundary(
    atmosphere, atmosphere.absorption_density, r, mu)));
}

vec3 ComputeTransmittanceToTopAtmosphereBoundaryTexture(in AtmosphereParameters atmosphere, in vec2 frag_coord) {
    vec2 TRANSMITTANCE_TEXTURE_SIZE = vec2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
    flaot r;
    float mu;
    GetRMuFromTransmittanceTextureUv(atmosphere, frag_coord / TRANSMITTANCE_TEXTURE_SIZE, r, mu);
    return ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);
}

void main() {
    transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(atmosphere_parameters, gl_FragCoord.xy);
}