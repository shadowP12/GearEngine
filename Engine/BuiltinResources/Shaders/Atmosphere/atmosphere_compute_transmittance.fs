layout(location = 0) out vec4 out_color;

struct SingleScatteringResult
{
    vec3 optical_depth;
};

SingleScatteringResult IntegrateScatteredLuminance(
    in vec2 pix_pos, in vec3 world_pos, in vec3 world_dir,
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

    // Ray march
    vec3 optical_depth = vec3(0.0);
    float dt = t_max / sample_count;
    float t = 0.0;
    float t_prev = 0.0;
    for (float s = 0.0f; s < sample_count; s += 1.0)
    {
        t = dt * s;
        vec3 p = world_pos + t * world_dir;

        // Sample medium
        MediumSampleResult medium = SampleMedium(p, atmosphere);
        vec3 sample_optical_depth = medium.extinction * dt;
        optical_depth += sample_optical_depth;

        t_prev = t;
    }
    result.optical_depth = optical_depth;
    return result;
}

void main() {
    AtmosphereParameters atmosphere = GetAtmosphereParameters();

    vec2 pix_pos = gl_FragCoord.xy;
    vec2 uv = (pix_pos) / vec2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);

    float view_height;
    float view_zenith_cos_angle;
    UvToLutTransmittanceParams(atmosphere, view_height, view_zenith_cos_angle, uv);

    vec3 world_pos = vec3(0.0, view_height, 0.0);
    vec3 world_dir = vec3(sqrt(1.0 - view_zenith_cos_angle * view_zenith_cos_angle), view_zenith_cos_angle, 0.0);
    float sample_count = 40.0;
    float t_max_max = 9000000;
    vec3 transmittance = exp(-IntegrateScatteredLuminance(pix_pos, world_pos, world_dir, atmosphere, sample_count, t_max_max).optical_depth);

    out_color = vec4(transmittance, 1.0);
}