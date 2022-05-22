#pragma once
#include "Component.h"
#include "Math/Math.h"

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

        void SetIrradianceMap(Texture* irradiance_map) { this->irradiance_map = irradiance_map; }

        Texture* GetIrradianceMap() { return irradiance_map; }

        void SetPrefilteredMap(Texture* prefiltered_map) { this->prefiltered_map = prefiltered_map; }

        Texture* GetPrefilteredMap() { return prefiltered_map; }

        void SetBRDFLut(Texture* lut) { brdf_lut = lut; }

        Texture* GetBRDFLut() { return brdf_lut; }

    private:
        LightType type = LightType::DIRECTION;
        Texture* irradiance_map = nullptr;
        Texture* prefiltered_map = nullptr;
        Texture* brdf_lut = nullptr;
        /*
         * DIRECTION : lux
        */
        float intensity = 0.0f;
        glm::vec3 color = glm::vec3(0.0f);
    };
}