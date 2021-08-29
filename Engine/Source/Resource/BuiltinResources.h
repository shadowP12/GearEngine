#pragma once

namespace gear {
    class Material;
    class MaterialInstance;
    class BuiltinResources {
    public:
        BuiltinResources();

        ~BuiltinResources();

        void Prepare();

        Material* GetDebugMaterial() { return _debug_ma; }

    private:
        void CreateDebugMaterial();

    private:
        Material* _debug_ma;
        MaterialInstance* _debug_mi;
    };
}
