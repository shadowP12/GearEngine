#pragma once
#include <GfxDefine.h>
#include <string>
#include <memory>

namespace gear {
    class SimpleShader;
    class Material;
    class MaterialInstance;
    class BuiltinResources {
    public:
        BuiltinResources();

        ~BuiltinResources();

        void Initialize(const std::string&);

        std::shared_ptr<Material> GetBlitMaterial() { return blit_ma; }

        std::shared_ptr<Material> GetDebugMaterial() { return debug_ma; }

        std::shared_ptr<Material> GetSkyBoxMaterial() { return skybox_ma; }

        std::shared_ptr<Material> GetFXAAMaterial() { return fxaa_ma; }

        std::shared_ptr<blast::GfxBuffer> GetQuadBuffer() { return quad_buffer; }

        std::shared_ptr<blast::GfxBuffer> GetCubeBuffer() { return cube_buffer; }

        std::shared_ptr<blast::GfxShader> GetPanoToCubeShader() { return pano_to_cube_shader; }

        std::shared_ptr<blast::GfxShader> GetComputeIrradianceMapShader() { return compute_irradiance_map_shader; }

        std::shared_ptr<blast::GfxShader> GetComputeSpecularMapShader() { return compute_specular_map_shader; }

        std::shared_ptr<blast::GfxShader> GetBRDFIntegrationShader() { return brdf_integration_shader; }

        std::shared_ptr<blast::GfxShader> GetAtmosphereCommonVS(uint32_t var = 0);

        std::shared_ptr<blast::GfxShader> GetAtmosphereComputeTransmittanceFS(uint32_t var = 0);

        std::shared_ptr<blast::GfxShader> GetAtmosphereComputeMultiScattCS(uint32_t var = 0);

        std::shared_ptr<blast::GfxShader> GetAtmosphereRayMarchingFS(uint32_t var = 0);

    private:
        void CreateQuadBuffer();

        void CreateCubeBuffer();

    private:
        std::shared_ptr<Material> blit_ma = nullptr;
        std::shared_ptr<Material> debug_ma = nullptr;
        std::shared_ptr<Material> skybox_ma = nullptr;
        std::shared_ptr<Material> fxaa_ma = nullptr;
        std::shared_ptr<blast::GfxBuffer> quad_buffer = nullptr;
        std::shared_ptr<blast::GfxBuffer> cube_buffer = nullptr;
        std::shared_ptr<blast::GfxShader> pano_to_cube_shader = nullptr;
        std::shared_ptr<blast::GfxShader> compute_irradiance_map_shader = nullptr;
        std::shared_ptr<blast::GfxShader> compute_specular_map_shader = nullptr;
        std::shared_ptr<blast::GfxShader> brdf_integration_shader = nullptr;
        std::shared_ptr<SimpleShader> atmosphere_comon_vs = nullptr;
        std::shared_ptr<SimpleShader> atmosphere_compute_transmittance_fs = nullptr;
        std::shared_ptr<SimpleShader> atmosphere_compute_multi_scatt_cs = nullptr;
        std::shared_ptr<SimpleShader> atmosphere_raymarching_fs = nullptr;
    };
}
