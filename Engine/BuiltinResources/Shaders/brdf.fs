float D_GGX(float roughness, float NoH) {
    float a = NoH * roughness;
    float k = roughness / (1.0 - NoH * NoH + a * a);
    float d = k * k * (1.0 / PI);
    return saturate(d);
}

float V_SmithGGXCorrelated(float roughness, float NoV, float NoL) {
    float a2 = roughness * roughness;
    float GGXV = NoL * sqrt((NoV - a2 * NoV) * NoV + a2);
    float GGXL = NoV * sqrt((NoL - a2 * NoL) * NoL + a2);
    float v = 0.5 / (GGXV + GGXL);
    return saturate(v);
}

vec3 F_Schlick(const vec3 f0, float VoH) {
    // f90 = 1.0
    float f = pow(1.0 - VoH, 5.0);
    return f + f0 * (1.0 - f);
}

float Distribution(float roughness, float NoH, const vec3 H) {
    return D_GGX(roughness, NoH);
}

float Visibility(float roughness, float NoV, float NoL) {
    return V_SmithGGXCorrelated(roughness, NoV, NoL);
}

vec3 Fresnel(const vec3 f0, float LoH) {
    return F_Schlick(f0, LoH);
}

float Diffuse(float roughness, float NoV, float NoL, float LoH) {
    // Todo: Burley Diffuse
    return 1.0 / PI;
}