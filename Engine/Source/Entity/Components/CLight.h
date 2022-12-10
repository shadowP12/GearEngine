#pragma once
#include "Component.h"
#include "Math/Math.h"
#include <GfxDefine.h>

namespace gear {
    class Entity;
    class Texture;
    class CLight : public Component {
    public:
        enum class LightType {
            POINT,
            SPOT,
            DIRECTION,
            IBL,
        };

    public:
        CLight(Entity* entity);

        virtual ~CLight();

        static ComponentType GetClassType() { return ComponentType::Light; }

        ComponentType GetType() override { return ComponentType::Light; }

        void SetLightType(LightType type) { this->type = type; }

        LightType GetLightType() { return type; }

        void SetIntensity(float intensity) { this->intensity = intensity; }

        float GetIntensity() { return intensity; }

        void SetColor(glm::vec3 color) { this->color = color; }

        glm::vec3 GetColor() { return color; }

        void SetIrradianceMap(std::shared_ptr<blast::GfxTexture> irradiance_map) { this->irradiance_map = irradiance_map; }

        std::shared_ptr<blast::GfxTexture> GetIrradianceMap() { return irradiance_map; }

        void SetPrefilteredMap(std::shared_ptr<blast::GfxTexture> prefiltered_map) { this->prefiltered_map = prefiltered_map; }

        std::shared_ptr<blast::GfxTexture> GetPrefilteredMap() { return prefiltered_map; }

        void SetBRDFLut(std::shared_ptr<blast::GfxTexture> lut) { brdf_lut = lut; }

        std::shared_ptr<blast::GfxTexture> GetBRDFLut() { return brdf_lut; }

    private:
        LightType type = LightType::DIRECTION;
        std::shared_ptr<blast::GfxTexture> irradiance_map = nullptr;
        std::shared_ptr<blast::GfxTexture> prefiltered_map = nullptr;
        std::shared_ptr<blast::GfxTexture> brdf_lut = nullptr;
        /*
         * Direction : lux
        */
        float intensity = 0.0f;
        glm::vec3 color = glm::vec3(0.0f);
    };
}