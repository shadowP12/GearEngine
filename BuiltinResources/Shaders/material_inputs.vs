struct MaterialVertexInputs {
    vec4 world_position;

#ifdef HAS_ATTRIBUTE_NORMAL
    vec3 world_normal;
#endif

#ifdef HAS_ATTRIBUTE_COLOR
    vec4 color;
#endif

#ifdef HAS_ATTRIBUTE_UV0
    vec2 uv0;
#endif

#ifdef HAS_ATTRIBUTE_UV1
    vec2 uv1;
#endif
};

vec4 getWorldPosition(const MaterialVertexInputs material) {
    return material.world_position;
}

void initMaterialVertex(out MaterialVertexInputs material) {
}
