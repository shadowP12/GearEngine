#pragma once
#include <Blast/Gfx/GfxDefine.h>

#include <string>

namespace gear {
	class SimpleShader;
    class Material;
    class VertexBuffer;
    class MaterialInstance;
    class BuiltinResources {
    public:
        BuiltinResources();

        ~BuiltinResources();

        void Initialize(const std::string&);

        Material* GetBlitMaterial() { return blit_ma; }

        Material* GetDebugMaterial() { return debug_ma; }

        Material* GetSkyBoxMaterial() { return skybox_ma; }

        Material* GetFXAAMaterial() { return fxaa_ma; }

        VertexBuffer* GetQuadBuffer() { return quad_buffer; }

        VertexBuffer* GetCubeBuffer() { return cube_buffer; }

        blast::GfxShader* GetPanoToCubeShader() { return pano_to_cube_shader; }

        blast::GfxShader* GetComputeIrradianceMapShader() { return compute_irradiance_map_shader; }

        blast::GfxShader* GetComputeSpecularMapShader() { return compute_specular_map_shader; }

        blast::GfxShader* GetBRDFIntegrationShader() { return brdf_integration_shader; }

		blast::GfxShader* GetAtmosphereCommonVS(uint32_t var = 0);

		blast::GfxShader* GetAtmosphereComputeTransmittanceFS(uint32_t var = 0);

    private:
        void CreateQuadBuffer();

        void CreateCubeBuffer();

    private:
        Material* blit_ma = nullptr;
        Material* debug_ma = nullptr;
        Material* skybox_ma = nullptr;
        Material* fxaa_ma = nullptr;
        VertexBuffer* quad_buffer = nullptr;
        VertexBuffer* cube_buffer = nullptr;
        blast::GfxShader* pano_to_cube_shader = nullptr;
        blast::GfxShader* compute_irradiance_map_shader = nullptr;
        blast::GfxShader* compute_specular_map_shader = nullptr;
        blast::GfxShader* brdf_integration_shader = nullptr;
		SimpleShader* atmosphere_comon_vs = nullptr;
		SimpleShader* atmosphere_compute_transmittance_fs = nullptr;
    };
}
