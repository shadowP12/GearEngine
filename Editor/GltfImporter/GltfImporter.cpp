#include "GltfImporter.h"
#include "GltfMaterialTemplate.h"
#include "../GearEditor.h"
#include "../TextureImporter.h"
#include <Engine/Renderer/RenderScene.h>
#include <Engine/Utility/Log.h>
#include <Engine/Utility/Hash.h>
#include <Engine/Resource/GpuBuffer.h>
#include <Engine/Resource/Material.h>
#include <Engine/Scene/Scene.h>
#include <Engine/Scene/Entity.h>
#include <Engine/Scene/Components/CCamera.h>
#include <Engine/Scene/Components/CTransform.h>
#include <Engine/Scene/Components/CRenderable.h>
#include <Engine/GearEngine.h>
#include <unordered_map>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

int getCNodeInxFromCData(const cgltf_node* node, const cgltf_data* data);
bool hasGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type);
cgltf_attribute* getGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type);
glm::mat4 getLocalMatrix(cgltf_node* node);
glm::mat4 getWorldMatrix(cgltf_node* node);
void combindVertexData(void* dst, void* src, uint32_t vertexCount, uint32_t attributeSize, uint32_t offset, uint32_t stride, uint32_t size);

bool GltfMaterialConfig::Eq::operator()(const GltfMaterialConfig& c0, const GltfMaterialConfig& c1) const {
    if (c0.blendingMode != c1.blendingMode) return false;
    if (c0.hasBaseColorTex != c1.hasBaseColorTex) return false;
    if (c0.hasNormalTex != c1.hasNormalTex) return false;
    if (c0.hasMetallicRoughnessTex != c1.hasMetallicRoughnessTex) return false;
    return true;
}

GltfImporter::GltfImporter() {

}

GltfImporter::~GltfImporter() {

}

GltfAsset* GltfImporter::import(const std::string& filePath) {
    // 初始化资源容器
    std::vector<gear::Texture*> textures;
    std::vector<gear::VertexBuffer*> vertexBuffers;
    std::vector<gear::IndexBuffer*> indexBuffers;
    std::vector<gear::Material*> materials;
    std::vector<gear::MaterialInstance*> materialInstances;
    std::vector<gear::Entity*> entities;
    std::unordered_map<GltfMaterialConfig, gear::Material*, MurmurHash<GltfMaterialConfig>, GltfMaterialConfig::Eq> materialMap;

    cgltf_options options = {static_cast<cgltf_file_type>(0)};
    cgltf_data* data = NULL;
    if (cgltf_parse_file(&options, filePath.c_str(), &data) != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    if (cgltf_load_buffers(&options, data, filePath.c_str()) != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    if (cgltf_validate(data) != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    // 加载图片资源
    std::map<cgltf_image*, gear::Texture*> imageHelper;
    for (int i = 0; i < data->images_count; ++i) {
        cgltf_image* cimage = &data->images[i];
        std::string imagePath = filePath + "/" + cimage->uri;
        gear::Texture* texture = gEditor.getTextureImporter()->importTexture2D(imagePath);
        imageHelper[cimage] = texture;
        textures.push_back(texture);
    }

    // 加载材质
    std::map<cgltf_material*, gear::MaterialInstance*> materialHelper;
    for (int i = 0; i < data->materials_count; ++i) {
        cgltf_material* cmaterial = &data->materials[i];
        GltfMaterialConfig config;
        if (cmaterial->alpha_mode == cgltf_alpha_mode_opaque) {
            config.blendingMode = gear::BlendingMode::BLENDING_MODE_OPAQUE;
        } else if(cmaterial->alpha_mode == cgltf_alpha_mode_mask) {
            config.blendingMode = gear::BlendingMode::BLENDING_MODE_MASKED;
        } else {
            config.blendingMode = gear::BlendingMode::BLENDING_MODE_TRANSPARENT;
        }

        if(cmaterial->pbr_metallic_roughness.base_color_texture.texture) {
            config.hasBaseColorTex = true;
        }

        if(cmaterial->normal_texture.texture) {
            config.hasNormalTex = true;
        }

        if(cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture) {
            config.hasMetallicRoughnessTex = true;
        }

        gear::Material* material = nullptr;
        auto iter = materialMap.find(config);
        if (iter != materialMap.end()) {
            material = iter->second;
        } else {
            GltfMaterialTemplate materialTemplate;
            material = materialTemplate.gen(&config);
            materialMap[config] = material;
            materials.push_back(material);
        }

        gear::MaterialInstance* materialInstance = material->createInstance();
        materialHelper[cmaterial] = materialInstance;
        materialInstances.push_back(materialInstance);
        Blast::GfxShaderVariable variable = material->getVariable("base_color");
        glm::vec4 baseColorValue = glm::make_vec4(cmaterial->pbr_metallic_roughness.base_color_factor);
        materialInstance->setParameter("base_color", &baseColorValue, variable.offset, variable.size);

        variable = material->getVariable("metallic_roughness");
        glm::vec4 metallicRoughnessValue = glm::vec4(0.0f);
        metallicRoughnessValue.x = cmaterial->pbr_metallic_roughness.metallic_factor;
        metallicRoughnessValue.y = cmaterial->pbr_metallic_roughness.roughness_factor;
        materialInstance->setParameter("metallic_roughness", &metallicRoughnessValue, variable.offset, variable.size);

        if(cmaterial->pbr_metallic_roughness.base_color_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->pbr_metallic_roughness.base_color_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->pbr_metallic_roughness.base_color_texture.texture->sampler;
            Blast::GfxSamplerDesc samplerDesc;
            materialInstance->setParameter("base_color_texture", imageHelper[cimage], samplerDesc);
        }

        if(cmaterial->normal_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->normal_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->normal_texture.texture->sampler;
            Blast::GfxSamplerDesc samplerDesc;
            materialInstance->setParameter("normal_texture", imageHelper[cimage], samplerDesc);
        }

        if(cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture->sampler;
            Blast::GfxSamplerDesc samplerDesc;
            materialInstance->setParameter("metallic_roughness_texture", imageHelper[cimage], samplerDesc);
        }
    }

    // 加载Entity
    std::map<cgltf_node*, gear::Entity*> nodeHelper;
    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* cnode = &data->nodes[i];
        glm::vec3 translation = glm::vec3(0.0f);
        if (cnode->has_translation) {
            translation.x = cnode->translation[0];
            translation.y = cnode->translation[1];
            translation.z = cnode->translation[2];
        }

        glm::quat rotation = glm::quat(1, 0, 0, 0);
        if (cnode->has_rotation) {
            rotation.x = cnode->rotation[0];
            rotation.y = cnode->rotation[1];
            rotation.z = cnode->rotation[2];
            rotation.w = cnode->rotation[3];
        }

        glm::vec3 scale = glm::vec3(1.0f);
        if (cnode->has_scale) {
            scale.x = cnode->scale[0];
            scale.y = cnode->scale[1];
            scale.z = cnode->scale[2];
        }

        if (cnode->has_matrix) {
            glm::mat4 mat = glm::make_mat4(cnode->matrix);
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(mat, scale, rotation, translation, skew, perspective);
        }
        gear::Entity* entity = gear::gEngine.getScene()->createEntity();
        entities.push_back(entity);
        glm::mat4 r, t, s;
        r = glm::toMat4(rotation);
        t = glm::translate(glm::mat4(1.0), translation);
        s = glm::scale(glm::mat4(1.0), scale);
        entity->addComponent<gear::CTransform>()->setTransform(t * r * s);
        nodeHelper[cnode] = entity;
    }

    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* cnode = &data->nodes[i];
        if(cnode->parent != nullptr) {
            gear::Entity* child = nodeHelper[cnode];
            gear::Entity* parent = nodeHelper[cnode->parent];
            child->getComponent<gear::CTransform>()->setParent(parent);
        }
    }

    // 加载网格
    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* cnode = &data->nodes[i];
        cgltf_mesh* cmesh = cnode->mesh;

        if(!cmesh) {
            continue;
        }

        gear::CRenderable* crenderable = nodeHelper[cnode]->addComponent<gear::CRenderable>();

        for (int i = 0; i < cmesh->primitives_count; i++) {
            cgltf_primitive* cprimitive = &cmesh->primitives[i];
            cgltf_material* cmaterial = cprimitive->material;

            gear::VertexBuffer::Builder vertexBufferBuilder;
            uint32_t vertexCount = getGltfAttribute(cprimitive, cgltf_attribute_type_position)->data->count;
            vertexBufferBuilder.vertexCount(vertexCount);
            vertexBufferBuilder.attribute(Blast::SEMANTIC_POSITION, Blast::FORMAT_R32G32B32_FLOAT);

            if(hasGltfAttribute(cprimitive, cgltf_attribute_type_texcoord)) {
                vertexBufferBuilder.attribute(Blast::SEMANTIC_TEXCOORD0, Blast::FORMAT_R32G32_FLOAT);
            }

            if(hasGltfAttribute(cprimitive, cgltf_attribute_type_normal)) {
                vertexBufferBuilder.attribute(Blast::SEMANTIC_NORMAL, Blast::FORMAT_R32G32B32_FLOAT);
                vertexBufferBuilder.attribute(Blast::SEMANTIC_TANGENT, Blast::FORMAT_R32G32B32_FLOAT);
                vertexBufferBuilder.attribute(Blast::SEMANTIC_BITANGENT, Blast::FORMAT_R32G32B32_FLOAT);
            }

            if(hasGltfAttribute(cprimitive, cgltf_attribute_type_color)) {
                vertexBufferBuilder.attribute(Blast::SEMANTIC_COLOR, Blast::FORMAT_R32G32B32_FLOAT);
            }

            if(hasGltfAttribute(cprimitive, cgltf_attribute_type_joints)) {
                // TODO: joints and weights
            }

            gear::VertexBuffer* vertexBuffer = vertexBufferBuilder.build();
            uint8_t* vertexData = new uint8_t[vertexBuffer->getSize()];
            uint32_t vertexStride = 0;

            uint32_t positionOffset = -1;
            uint32_t colorOffset = -1;
            uint32_t uvOffset = -1;
            uint32_t normalOffset = -1;
            uint32_t tangentOffset = -1;
            uint32_t bitangentOffset = -1;
            uint32_t jointOffset = -1;
            uint32_t weightOffset = -1;

            uint32_t positionAttributeSize = 0;
            uint32_t colorAttributeSize = 0;
            uint32_t uvAttributeSize = 0;
            uint32_t normalAttributeSize = 0;
            uint32_t tangentAttributeSize = 0;
            uint32_t bitangentAttributeSize = 0;
            uint32_t jointAttributeSize = 0;
            uint32_t weightAttributeSize = 0;

            uint32_t positionDataSize = 0;
            uint32_t colorDataSize = 0;
            uint32_t uvDataSize = 0;
            uint32_t normalDataSize = 0;
            uint32_t tangentDataSize = 0;
            uint32_t bitangentDataSize = 0;
            uint32_t jointDataSize = 0;
            uint32_t weightDataSize = 0;

            Blast::GfxVertexLayout vertexLayout = vertexBuffer->getVertexLayout();
            for (int j = 0; j < vertexLayout.attribCount; ++j) {
                if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_POSITION) {
                    positionOffset = vertexLayout.attribs[j].offset;
                    positionAttributeSize = vertexLayout.attribs[j].size;
                    positionDataSize = vertexLayout.attribs[j].size * vertexCount;
                } else if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_COLOR) {
                    colorOffset = vertexLayout.attribs[j].offset;
                    colorAttributeSize = vertexLayout.attribs[j].size;
                    colorDataSize = vertexLayout.attribs[j].size * vertexCount;
                } else if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_TEXCOORD0) {
                    uvOffset = vertexLayout.attribs[j].offset;
                    uvAttributeSize = vertexLayout.attribs[j].size;
                    uvDataSize = vertexLayout.attribs[j].size * vertexCount;
                } else if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_NORMAL) {
                    normalOffset = vertexLayout.attribs[j].offset;
                    normalAttributeSize = vertexLayout.attribs[j].size;
                    normalDataSize = vertexLayout.attribs[j].size * vertexCount;
                } else if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_TANGENT) {
                    tangentOffset = vertexLayout.attribs[j].offset;
                    tangentAttributeSize = vertexLayout.attribs[j].size;
                    tangentDataSize = vertexLayout.attribs[j].size * vertexCount;
                } else if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_BITANGENT) {
                    bitangentOffset = vertexLayout.attribs[j].offset;
                    bitangentAttributeSize = vertexLayout.attribs[j].size;
                    bitangentDataSize = vertexLayout.attribs[j].size * vertexCount;
                } else if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_JOINTS) {
                    jointOffset = vertexLayout.attribs[j].offset;
                    jointAttributeSize = vertexLayout.attribs[j].size;
                    jointDataSize = vertexLayout.attribs[j].size * vertexCount;
                } else if (vertexLayout.attribs[j].semantic == Blast::SEMANTIC_WEIGHTS) {
                    weightOffset = vertexLayout.attribs[j].offset;
                    weightAttributeSize = vertexLayout.attribs[j].size;
                    weightDataSize = vertexLayout.attribs[j].size * vertexCount;
                }
                vertexStride += vertexLayout.attribs[j].size;
            }

            // 获取顶点数据
            uint8_t* positionData = nullptr;
            uint8_t* uvData = nullptr;
            uint8_t* colorData = nullptr;
            uint8_t* normalData = nullptr;
            uint8_t* tangentData = nullptr;
            uint8_t* bitangentData = nullptr;
            uint8_t* jointData = nullptr;
            uint8_t* weightData = nullptr;

            if (positionOffset != -1) {
                cgltf_attribute* positionAttribute = getGltfAttribute(cprimitive, cgltf_attribute_type_position);
                cgltf_accessor* posAccessor = positionAttribute->data;
                cgltf_buffer_view* posView = posAccessor->buffer_view;
                positionData = (uint8_t*)(posView->buffer->data) + posAccessor->offset + posView->offset;
                vertexCount = posAccessor->count;
            }

            if (colorOffset != -1) {
                cgltf_attribute* colorAttribute = getGltfAttribute(cprimitive, cgltf_attribute_type_color);
                cgltf_accessor* colorAccessor = colorAttribute->data;
                cgltf_buffer_view* colorView = colorAccessor->buffer_view;
                colorData = (uint8_t*)(colorView->buffer->data) + colorAccessor->offset + colorView->offset;
            }

            if (uvOffset != -1) {
                cgltf_attribute* texcoordAttribute = getGltfAttribute(cprimitive, cgltf_attribute_type_texcoord);
                cgltf_accessor* texcoordAccessor = texcoordAttribute->data;
                cgltf_buffer_view* texcoordView = texcoordAccessor->buffer_view;
                uvData = (uint8_t *) (texcoordView->buffer->data) + texcoordAccessor->offset + texcoordView->offset;
            }

            if (normalOffset != -1) {
                cgltf_attribute* normalAttribute = getGltfAttribute(cprimitive, cgltf_attribute_type_normal);
                cgltf_accessor* normalAccessor = normalAttribute->data;
                cgltf_buffer_view* normalView = normalAccessor->buffer_view;
                normalData = (uint8_t*)(normalView->buffer->data) + normalAccessor->offset + normalView->offset;
            }

            if (tangentOffset != -1) {
                cgltf_attribute* tangentAttribute = getGltfAttribute(cprimitive, cgltf_attribute_type_tangent);
                cgltf_accessor *tangentAccessor = tangentAttribute->data;
                cgltf_buffer_view *tangentView = tangentAccessor->buffer_view;
                tangentData = (uint8_t*)(tangentView->buffer->data) + tangentAccessor->offset + tangentView->offset;
            }

            if (bitangentOffset != -1) {
                // 计算副切线
                bitangentData = new uint8_t[vertexCount * sizeof(glm::vec3)];
                float* normals = (float*)normalData;
                float* tangents = (float*)tangentData;
                float* bitangents = (float*)bitangentData;
                for (int j = 0; j < vertexCount; ++j) {
                    glm::vec3 noraml = glm::vec3(normals[j * 3], normals[j * 3 + 1], normals[j * 3 + 2]);
                    glm::vec3 tangent = glm::vec3(tangents[j * 3], tangents[j * 3 + 1], tangents[j * 3 + 2]);
                    glm::vec3 bitangent = glm::cross(noraml, tangent);
                    bitangents[j * 3] = bitangent.x;
                    bitangents[j * 3 + 1] = bitangent.y;
                    bitangents[j * 3 + 2] = bitangent.z;
                }
            }

            if (jointOffset != -1) {
                cgltf_attribute* jointsAttribute = getGltfAttribute(cprimitive, cgltf_attribute_type_joints);
                cgltf_accessor* jointsAccessor = jointsAttribute->data;
                cgltf_buffer_view* jointsView = jointsAccessor->buffer_view;
                jointData = (uint8_t*)(jointsView->buffer->data) + jointsAccessor->offset + jointsView->offset;
            }

            if (weightOffset != -1) {
                cgltf_attribute* weightsAttributes = nullptr;
                cgltf_accessor* weightsAccessor = weightsAttributes->data;
                cgltf_buffer_view* weightsView = weightsAccessor->buffer_view;
                weightData = (uint8_t*)(weightsView->buffer->data) + weightsAccessor->offset + weightsView->offset;
            }

            // 组装数据
            if (positionOffset != -1) {
                combindVertexData(vertexData, positionData, vertexCount, positionAttributeSize, positionOffset, vertexStride, positionDataSize);
            }

            // 测试
//            float* t = (float*)vertexData;
//            for (int j = 0; j < vertexCount; ++j) {
//                float g = t[j];
//                LOGI("%f\n", g);
//            }

            if (colorOffset != -1) {
                combindVertexData(vertexData, colorData, vertexCount, colorAttributeSize, colorOffset, vertexStride, colorDataSize);
            }

            if (uvOffset != -1) {
                combindVertexData(vertexData, uvData, vertexCount, uvAttributeSize, uvOffset, vertexStride, uvDataSize);
            }

            if (normalOffset != -1) {
                combindVertexData(vertexData, normalData, vertexCount, normalAttributeSize, normalOffset, vertexStride, normalDataSize);
            }

            if (tangentOffset != -1) {
                combindVertexData(vertexData, tangentData, vertexCount, tangentAttributeSize, tangentOffset, vertexStride, tangentDataSize);
            }

            if (bitangentOffset != -1) {
                combindVertexData(vertexData, bitangentData, vertexCount, bitangentAttributeSize, bitangentOffset, vertexStride, bitangentDataSize);
            }

            if (jointOffset != -1) {
                combindVertexData(vertexData, jointData, vertexCount, jointAttributeSize, jointOffset, vertexStride, jointDataSize);
            }

            if (weightOffset != -1) {
                combindVertexData(vertexData, weightData, vertexCount, weightAttributeSize, weightOffset, vertexStride, weightDataSize);
            }

            vertexBuffer->update(vertexData, 0, vertexBuffer->getSize());
            SAFE_DELETE(bitangentData);
            SAFE_DELETE(vertexData);

            // Indices
            cgltf_accessor* cIndexAccessor = cprimitive->indices;
            cgltf_buffer_view* cIndexBufferView = cIndexAccessor->buffer_view;
            cgltf_buffer* cIndexBuffer = cIndexBufferView->buffer;

            gear::IndexBuffer::Builder indexBufferBuild;
            uint32_t indexCount = cIndexAccessor->count;
            indexBufferBuild.indexCount(indexCount);
            if (cIndexAccessor->component_type == cgltf_component_type_r_16u) {
                indexBufferBuild.indexType(Blast::INDEX_TYPE_UINT16);
            } else if (cIndexAccessor->component_type == cgltf_component_type_r_32u) {
                indexBufferBuild.indexType(Blast::INDEX_TYPE_UINT32);
            } else {
                LOGE("Index component type not supported!\n");
            }
            gear::IndexBuffer* indexBuffer = indexBufferBuild.build();
            uint8_t* indexData = (uint8_t*)cIndexBuffer->data + cIndexAccessor->offset + cIndexBufferView->offset;
            indexBuffer->update(indexData, 0, indexBuffer->getSize());

            vertexBuffers.push_back(vertexBuffer);
            indexBuffers.push_back(indexBuffer);

            gear::RenderPrimitive primitive;
            primitive.count = indexCount;
            primitive.offset = 0;
            primitive.type = Blast::PRIMITIVE_TOPO_TRI_LIST;
            primitive.materialInstance = materialHelper[cmaterial];
            primitive.vertexBuffer = vertexBuffer;
            primitive.indexBuffer = indexBuffer;
            crenderable->addPrimitive(primitive);
        }
    }

    cgltf_free(data);
    GltfAsset* asset = new GltfAsset();
    asset->textures = std::move(textures);
    asset->vertexBuffers = std::move(vertexBuffers);
    asset->indexBuffers = std::move(indexBuffers);
    asset->materials = std::move(materials);
    asset->materialInstances = std::move(materialInstances);
    asset->entities = std::move(entities);
    return asset;
}

void GltfImporter::destroyGltfAsset(GltfAsset* asset) {
    for (int i = 0; i < asset->textures.size(); ++i) {
        SAFE_DELETE(asset->textures[i]);
    }

    for (int i = 0; i < asset->materialInstances.size(); ++i) {
        SAFE_DELETE(asset->materialInstances[i]);
    }

    for (int i = 0; i < asset->materials.size(); ++i) {
        SAFE_DELETE(asset->materials[i]);
    }

    for (int i = 0; i < asset->vertexBuffers.size(); ++i) {
        SAFE_DELETE(asset->vertexBuffers[i]);
    }

    for (int i = 0; i < asset->indexBuffers.size(); ++i) {
        SAFE_DELETE(asset->indexBuffers[i]);
    }

    for (int i = 0; i < asset->entities.size(); ++i) {
        gear::gEngine.getScene()->destroyEntity(asset->entities[i]);
    }

    asset->textures.clear();
    asset->materialInstances.clear();
    asset->materials.clear();
    asset->vertexBuffers.clear();
    asset->indexBuffers.clear();
    asset->entities.clear();
    SAFE_DELETE(asset);
}

int getCNodeInxFromCData(const cgltf_node* node, const cgltf_data* data)
{
    for (size_t i = 0; i < data->nodes_count; ++i)
    {
        if(&data->nodes[i] == node)
        {
            return i;
        }
    }
    return -1;
}

bool hasGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type) {
    for (int i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute* att = &primitive->attributes[i];
        if(att->type == type) {
            return true;
        }
    }
    return false;
}

cgltf_attribute* getGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type) {
    for (int i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute* att = &primitive->attributes[i];
        if(att->type == type) {
            return att;
        }
    }
    return nullptr;
}

glm::mat4 getLocalMatrix(cgltf_node* node)
{
    glm::vec3 translation = glm::vec3(0.0f);
    if (node->has_translation)
    {
        translation.x = node->translation[0];
        translation.y = node->translation[1];
        translation.z = node->translation[2];
    }

    glm::quat rotation = glm::quat(1, 0, 0, 0);
    if (node->has_rotation)
    {
        rotation.x = node->rotation[0];
        rotation.y = node->rotation[1];
        rotation.z = node->rotation[2];
        rotation.w = node->rotation[3];
    }

    glm::vec3 scale = glm::vec3(1.0f);
    if (node->has_scale)
    {
        scale.x = node->scale[0];
        scale.y = node->scale[1];
        scale.z = node->scale[2];
    }

    glm::mat4 r, t, s;
    r = glm::toMat4(rotation);
    t = glm::translate(glm::mat4(1.0), translation);
    s = glm::scale(glm::mat4(1.0), scale);
    return t * r * s;
}

glm::mat4 getWorldMatrix(cgltf_node* node)
{
    cgltf_node* curNode = node;
    glm::mat4 out = getLocalMatrix(curNode);

    while (curNode->parent != nullptr)
    {
        curNode = node->parent;
        out = getLocalMatrix(curNode) * out;
    }
    return out;
}

void combindVertexData(void* dst, void* src, uint32_t vertexCount, uint32_t attributeSize, uint32_t offset, uint32_t stride, uint32_t size) {
    uint8_t* dstData = (uint8_t*)dst + offset;
    uint8_t* srcData = (uint8_t*)src;
    for (uint32_t i = 0; i < vertexCount; i++) {
        memcpy(dstData, srcData, attributeSize);
        dstData += stride;
        srcData += attributeSize;
    }
}