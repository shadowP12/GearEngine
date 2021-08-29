#include "GltfImporter.h"
#include "TextureImporter.h"
#include <Utility/Log.h>
#include <Utility/Hash.h>
#include <Resource/GpuBuffer.h>
#include <Resource/Material.h>
#include <Entity/Entity.h>
#include <Entity/EntityManager.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CRenderable.h>
#include <GearEngine.h>
#include <MaterialCompiler/MaterialCompiler.h>
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <unordered_map>

uint32_t GetCNodeInxFromCData(const cgltf_node* node, const cgltf_data* data);

bool HasGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type);

cgltf_attribute* GetGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type);

glm::mat4 GetLocalMatrix(cgltf_node* node);

glm::mat4 GetWorldMatrix(cgltf_node* node);

void CombindVertexData(void* dst, void* src, uint32_t vertex_count, uint32_t attribute_size, uint32_t offset, uint32_t stride, uint32_t size);

bool GltfMaterialConfig::Eq::operator()(const GltfMaterialConfig& c0, const GltfMaterialConfig& c1) const {
    if (c0.blending_mode != c1.blending_mode) return false;
    if (c0.has_base_color_tex != c1.has_base_color_tex) return false;
    if (c0.has_normal_tex != c1.has_normal_tex) return false;
    if (c0.has_metallic_roughness_tex != c1.has_metallic_roughness_tex) return false;
    return true;
}

static char* g_gltf_material_code =
        "{\n"
        "   \"state\": {\n"
        "       \"shading_model\": \"lit\",\n"
        "       \"blending_mode\": \"${BLENDING_MODE}\"\n"
        "   },\n"
        "   \"require\": [\n"
        "       \"uv0\"\n"
        "   ],\n"
        "   ${UNIFORMS}\n"
        "   ${SAMPLERS}\n"
        "   \"fragment_code\": \"${FRAG_SHADER}\"\n"
        "}\n";

bool Replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

gear::Material* GenMaterial(GltfMaterialConfig& config) {
    std::string code = g_gltf_material_code;

    // 设置混合模式
    if (config.blending_mode == gear::BlendingMode::BLENDING_MODE_OPAQUE) {
        Replace(code, "${BLENDING_MODE}", "opaque");
    } else if (config.blending_mode == gear::BlendingMode::BLENDING_MODE_MASKED) {
        Replace(code, "${BLENDING_MODE}", "masked");
    } else {
        Replace(code, "${BLENDING_MODE}", "transparent");
    }

    // 设置uniform变量
    std::string uniforms_code = "";
    uniforms_code += "\"uniforms\": [\n";
    uniforms_code += "{\n";
    uniforms_code += "\"name\": \"base_color\",\n";
    uniforms_code += "\"type\": \"float4\"\n";
    uniforms_code += "},\n";
    uniforms_code += "{\n";
    uniforms_code += "\"name\": \"metallic_roughness\",\n";
    uniforms_code += "\"type\": \"float4\"\n";
    uniforms_code += "}\n";
    uniforms_code += "],";

    Replace(code, "${UNIFORMS}", uniforms_code);

    // 设置samplers变量
    std::string samplers_code = "";
    if (config.has_base_color_tex || config.has_normal_tex || config.has_metallic_roughness_tex) {
        samplers_code += "samplers [\n";
    }
    if (config.has_base_color_tex) {
        samplers_code += "{\n";
        samplers_code += "\"name\": \"base_color_texture\",\n";
        samplers_code += "\"type\": \"sampler_2d\"\n";
        samplers_code += "},\n";
    }

    if (config.has_normal_tex) {
        samplers_code += "{\n";
        samplers_code += "\"name\": \"normal_texture\",\n";
        samplers_code += "\"type\": \"sampler_2d\"\n";
        samplers_code += "},\n";
    }

    if (config.has_metallic_roughness_tex) {
        samplers_code += "{\n";
        samplers_code += "\"name\": \"metallic_roughness_texture\",\n";
        samplers_code += "\"type\": \"sampler_2d\"\n";
        samplers_code += "},\n";
    }
    if (config.has_base_color_tex || config.has_normal_tex || config.has_metallic_roughness_tex) {
        samplers_code += "],";
    }
    Replace(code, "${SAMPLERS}", samplers_code);

    // 设置shader代码
    std::string frag_shader_code = "";
    frag_shader_code += "void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {\\n";
    if (config.has_base_color_tex) {
        frag_shader_code += "params.base_color = texture(base_color_texture, vertex_uv01.xy);\\n";
    } else {
        frag_shader_code += "params.base_color = material_uniforms.base_color;\\n";
        frag_shader_code += "params.base_color.rgb *= params.base_color.a;\\n";
    }

    if (config.has_normal_tex) {
        // TODO
    }

    if (config.has_metallic_roughness_tex) {
        // TODO
    }
    frag_shader_code += "}\\n";
    Replace(code, "${FRAG_SHADER}", frag_shader_code);

    gear::Material* material = gear::gEngine.GetMaterialCompiler()->Compile(code);
    return material;
}

GltfAsset* ImportGltfAsset(const std::string& path) {
    // 初始化资源容器
    std::vector<gear::Texture*> textures;
    std::vector<gear::VertexBuffer*> vertex_buffers;
    std::vector<gear::IndexBuffer*> index_buffers;
    std::vector<gear::Material*> materials;
    std::vector<gear::MaterialInstance*> material_instances;
    std::vector<gear::Entity*> entities;
    std::unordered_map<GltfMaterialConfig, gear::Material*, MurmurHash<GltfMaterialConfig>, GltfMaterialConfig::Eq> material_map;

    cgltf_options options = {static_cast<cgltf_file_type>(0)};
    cgltf_data* data = NULL;
    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    if (cgltf_validate(data) != cgltf_result_success) {
        cgltf_free(data);
        return nullptr;
    }

    // 加载图片资源
    std::map<cgltf_image*, gear::Texture*> image_helper;
    for (int i = 0; i < data->images_count; ++i) {
        cgltf_image* cimage = &data->images[i];
        std::string image_path = path + "/" + cimage->uri;
        gear::Texture* texture = ImportTexture2D(image_path);
        image_helper[cimage] = texture;
        textures.push_back(texture);
    }

    // 加载材质
    std::map<cgltf_material*, gear::MaterialInstance*> material_helper;
    for (int i = 0; i < data->materials_count; ++i) {
        cgltf_material* cmaterial = &data->materials[i];
        GltfMaterialConfig config;
        if (cmaterial->alpha_mode == cgltf_alpha_mode_opaque) {
            config.blending_mode = gear::BlendingMode::BLENDING_MODE_OPAQUE;
        } else if(cmaterial->alpha_mode == cgltf_alpha_mode_mask) {
            config.blending_mode = gear::BlendingMode::BLENDING_MODE_MASKED;
        } else {
            config.blending_mode = gear::BlendingMode::BLENDING_MODE_TRANSPARENT;
        }

        if(cmaterial->pbr_metallic_roughness.base_color_texture.texture) {
            config.has_base_color_tex = true;
        }

        if(cmaterial->normal_texture.texture) {
            config.has_normal_tex = true;
        }

        if(cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture) {
            config.has_metallic_roughness_tex = true;
        }

        gear::Material* material = nullptr;
        auto iter = material_map.find(config);
        if (iter != material_map.end()) {
            material = iter->second;
        } else {
            material = GenMaterial(config);
            material_map[config] = material;
            materials.push_back(material);
        }

        gear::MaterialInstance* material_instance = material->CreateInstance();
        material_helper[cmaterial] = material_instance;
        material_instances.push_back(material_instance);

        glm::vec4 base_color_value = glm::make_vec4(cmaterial->pbr_metallic_roughness.base_color_factor);
        material_instance->SetFloat4("base_color", base_color_value);
        
        glm::vec4 metallic_roughness_value = glm::vec4(0.0f);
        metallic_roughness_value.x = cmaterial->pbr_metallic_roughness.metallic_factor;
        metallic_roughness_value.y = cmaterial->pbr_metallic_roughness.roughness_factor;
        material_instance->SetFloat4("metallic_roughness", metallic_roughness_value);

        if(cmaterial->pbr_metallic_roughness.base_color_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->pbr_metallic_roughness.base_color_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->pbr_metallic_roughness.base_color_texture.texture->sampler;
            blast::GfxSamplerDesc sampler_desc;
            material_instance->SetTexture("base_color_texture", image_helper[cimage], sampler_desc);
        }

        if(cmaterial->normal_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->normal_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->normal_texture.texture->sampler;
            blast::GfxSamplerDesc sampler_desc;
            material_instance->SetTexture("normal_texture", image_helper[cimage], sampler_desc);
        }

        if(cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture->sampler;
            blast::GfxSamplerDesc sampler_desc;
            material_instance->SetTexture("metallic_roughness_texture", image_helper[cimage], sampler_desc);
        }
    }

    // 加载Entity
    std::map<cgltf_node*, gear::Entity*> node_helper;
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
        gear::Entity* entity = gear::gEngine.GetEntityManager()->CreateEntity();
        entities.push_back(entity);
        glm::mat4 r, t, s;
        r = glm::toMat4(rotation);
        t = glm::translate(glm::mat4(1.0), translation);
        s = glm::scale(glm::mat4(1.0), scale);
        entity->AddComponent<gear::CTransform>()->SetTransform(t * r * s);
        node_helper[cnode] = entity;
    }

    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* cnode = &data->nodes[i];
        if(cnode->parent != nullptr) {
            gear::Entity* child = node_helper[cnode];
            gear::Entity* parent = node_helper[cnode->parent];
            child->GetComponent<gear::CTransform>()->SetParent(parent);
        }
    }

    // 加载网格
    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* cnode = &data->nodes[i];
        cgltf_mesh* cmesh = cnode->mesh;

        if(!cmesh) {
            continue;
        }

        gear::CRenderable* crenderable = node_helper[cnode]->AddComponent<gear::CRenderable>();

        for (int i = 0; i < cmesh->primitives_count; i++) {
            cgltf_primitive* cprimitive = &cmesh->primitives[i];
            cgltf_material* cmaterial = cprimitive->material;

            gear::VertexBuffer::Builder vb_builder;
            uint32_t vertexCount = GetGltfAttribute(cprimitive, cgltf_attribute_type_position)->data->count;
            vb_builder.SetVertexCount(vertexCount);
            vb_builder.SetAttribute(blast::SEMANTIC_POSITION, blast::FORMAT_R32G32B32_FLOAT);

            if(HasGltfAttribute(cprimitive, cgltf_attribute_type_texcoord)) {
                vb_builder.SetAttribute(blast::SEMANTIC_TEXCOORD0, blast::FORMAT_R32G32_FLOAT);
            }

            if(HasGltfAttribute(cprimitive, cgltf_attribute_type_normal)) {
                vb_builder.SetAttribute(blast::SEMANTIC_NORMAL, blast::FORMAT_R32G32B32_FLOAT);
                vb_builder.SetAttribute(blast::SEMANTIC_TANGENT, blast::FORMAT_R32G32B32_FLOAT);
                vb_builder.SetAttribute(blast::SEMANTIC_BITANGENT, blast::FORMAT_R32G32B32_FLOAT);
            }

            if(HasGltfAttribute(cprimitive, cgltf_attribute_type_color)) {
                vb_builder.SetAttribute(blast::SEMANTIC_COLOR, blast::FORMAT_R32G32B32_FLOAT);
            }

            if(HasGltfAttribute(cprimitive, cgltf_attribute_type_joints)) {
                // TODO: joints and weights
            }

            gear::VertexBuffer* vertex_buffer = vb_builder.Build();
            uint8_t* vertexData = new uint8_t[vertex_buffer->GetSize()];
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

            blast::GfxVertexLayout vertex_layout = vertex_buffer->GetVertexLayout();
            for (int j = 0; j < vertex_layout.num_attributes; ++j) {
                if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_POSITION) {
                    positionOffset = vertex_layout.attributes[j].offset;
                    positionAttributeSize = vertex_layout.attributes[j].size;
                    positionDataSize = vertex_layout.attributes[j].size * vertexCount;
                } else if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_COLOR) {
                    colorOffset = vertex_layout.attributes[j].offset;
                    colorAttributeSize = vertex_layout.attributes[j].size;
                    colorDataSize = vertex_layout.attributes[j].size * vertexCount;
                } else if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_TEXCOORD0) {
                    uvOffset = vertex_layout.attributes[j].offset;
                    uvAttributeSize = vertex_layout.attributes[j].size;
                    uvDataSize = vertex_layout.attributes[j].size * vertexCount;
                } else if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_NORMAL) {
                    normalOffset = vertex_layout.attributes[j].offset;
                    normalAttributeSize = vertex_layout.attributes[j].size;
                    normalDataSize = vertex_layout.attributes[j].size * vertexCount;
                } else if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_TANGENT) {
                    tangentOffset = vertex_layout.attributes[j].offset;
                    tangentAttributeSize = vertex_layout.attributes[j].size;
                    tangentDataSize = vertex_layout.attributes[j].size * vertexCount;
                } else if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_BITANGENT) {
                    bitangentOffset = vertex_layout.attributes[j].offset;
                    bitangentAttributeSize = vertex_layout.attributes[j].size;
                    bitangentDataSize = vertex_layout.attributes[j].size * vertexCount;
                } else if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_JOINTS) {
                    jointOffset = vertex_layout.attributes[j].offset;
                    jointAttributeSize = vertex_layout.attributes[j].size;
                    jointDataSize = vertex_layout.attributes[j].size * vertexCount;
                } else if (vertex_layout.attributes[j].semantic == blast::SEMANTIC_WEIGHTS) {
                    weightOffset = vertex_layout.attributes[j].offset;
                    weightAttributeSize = vertex_layout.attributes[j].size;
                    weightDataSize = vertex_layout.attributes[j].size * vertexCount;
                }
                vertexStride += vertex_layout.attributes[j].size;
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

            gear::BBox bbox;
            if (positionOffset != -1) {
                cgltf_attribute* positionAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_position);
                cgltf_accessor* posAccessor = positionAttribute->data;
                cgltf_buffer_view* posView = posAccessor->buffer_view;
                positionData = (uint8_t*)(posView->buffer->data) + posAccessor->offset + posView->offset;
                vertexCount = posAccessor->count;

                const float* minp = &posAccessor->min[0];
                const float* maxp = &posAccessor->max[0];
                gear::BBox::Grow(bbox, glm::vec3(minp[0], minp[1], minp[2]));
                gear::BBox::Grow(bbox, glm::vec3(maxp[0], maxp[1], maxp[2]));
            }

            if (colorOffset != -1) {
                cgltf_attribute* colorAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_color);
                cgltf_accessor* colorAccessor = colorAttribute->data;
                cgltf_buffer_view* colorView = colorAccessor->buffer_view;
                colorData = (uint8_t*)(colorView->buffer->data) + colorAccessor->offset + colorView->offset;
            }

            if (uvOffset != -1) {
                cgltf_attribute* texcoordAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_texcoord);
                cgltf_accessor* texcoordAccessor = texcoordAttribute->data;
                cgltf_buffer_view* texcoordView = texcoordAccessor->buffer_view;
                uvData = (uint8_t *) (texcoordView->buffer->data) + texcoordAccessor->offset + texcoordView->offset;
            }

            if (normalOffset != -1) {
                cgltf_attribute* normalAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_normal);
                cgltf_accessor* normalAccessor = normalAttribute->data;
                cgltf_buffer_view* normalView = normalAccessor->buffer_view;
                normalData = (uint8_t*)(normalView->buffer->data) + normalAccessor->offset + normalView->offset;
            }

            if (tangentOffset != -1) {
                cgltf_attribute* tangentAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_tangent);
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
                cgltf_attribute* jointsAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_joints);
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
                CombindVertexData(vertexData, positionData, vertexCount, positionAttributeSize, positionOffset, vertexStride, positionDataSize);
            }

            if (colorOffset != -1) {
                CombindVertexData(vertexData, colorData, vertexCount, colorAttributeSize, colorOffset, vertexStride, colorDataSize);
            }

            if (uvOffset != -1) {
                CombindVertexData(vertexData, uvData, vertexCount, uvAttributeSize, uvOffset, vertexStride, uvDataSize);
            }

            if (normalOffset != -1) {
                CombindVertexData(vertexData, normalData, vertexCount, normalAttributeSize, normalOffset, vertexStride, normalDataSize);
            }

            if (tangentOffset != -1) {
                CombindVertexData(vertexData, tangentData, vertexCount, tangentAttributeSize, tangentOffset, vertexStride, tangentDataSize);
            }

            if (bitangentOffset != -1) {
                CombindVertexData(vertexData, bitangentData, vertexCount, bitangentAttributeSize, bitangentOffset, vertexStride, bitangentDataSize);
            }

            if (jointOffset != -1) {
                CombindVertexData(vertexData, jointData, vertexCount, jointAttributeSize, jointOffset, vertexStride, jointDataSize);
            }

            if (weightOffset != -1) {
                CombindVertexData(vertexData, weightData, vertexCount, weightAttributeSize, weightOffset, vertexStride, weightDataSize);
            }

            vertex_buffer->Update(vertexData, 0, vertex_buffer->GetSize());
            SAFE_DELETE(bitangentData);
            SAFE_DELETE(vertexData);

            // Indices
            cgltf_accessor* cIndexAccessor = cprimitive->indices;
            cgltf_buffer_view* cIndexBufferView = cIndexAccessor->buffer_view;
            cgltf_buffer* cIndexBuffer = cIndexBufferView->buffer;

            gear::IndexBuffer::Builder indexBufferBuild;
            uint32_t indexCount = cIndexAccessor->count;
            indexBufferBuild.SetIndexCount(indexCount);
            if (cIndexAccessor->component_type == cgltf_component_type_r_16u) {
                indexBufferBuild.SetIndexType(blast::INDEX_TYPE_UINT16);
            } else if (cIndexAccessor->component_type == cgltf_component_type_r_32u) {
                indexBufferBuild.SetIndexType(blast::INDEX_TYPE_UINT32);
            } else {
                LOGE("Index component type not supported!\n");
            }
            gear::IndexBuffer* indexBuffer = indexBufferBuild.Build();
            uint8_t* indexData = (uint8_t*)cIndexBuffer->data + cIndexAccessor->offset + cIndexBufferView->offset;
            indexBuffer->Update(indexData, 0, indexBuffer->GetSize());

            vertex_buffers.push_back(vertex_buffer);
            index_buffers.push_back(indexBuffer);

            gear::RenderPrimitive primitive;
            primitive.count = indexCount;
            primitive.offset = 0;
            primitive.bbox = bbox;
            primitive.topo = blast::PRIMITIVE_TOPO_TRI_STRIP;
            primitive.mi = material_helper[cmaterial];
            primitive.vb = vertex_buffer;
            primitive.ib = indexBuffer;
            crenderable->AddPrimitive(primitive);
        }
    }

    cgltf_free(data);
    GltfAsset* asset = new GltfAsset();
    asset->textures = std::move(textures);
    asset->vertex_buffers = std::move(vertex_buffers);
    asset->index_buffers = std::move(index_buffers);
    asset->materials = std::move(materials);
    asset->material_instances = std::move(material_instances);
    asset->entities = std::move(entities);
    return asset;
}

void DestroyGltfAsset(GltfAsset* asset) {
    for (int i = 0; i < asset->textures.size(); ++i) {
        SAFE_DELETE(asset->textures[i]);
    }

    for (int i = 0; i < asset->material_instances.size(); ++i) {
        SAFE_DELETE(asset->material_instances[i]);
    }

    for (int i = 0; i < asset->materials.size(); ++i) {
        SAFE_DELETE(asset->materials[i]);
    }

    for (int i = 0; i < asset->vertex_buffers.size(); ++i) {
        SAFE_DELETE(asset->vertex_buffers[i]);
    }

    for (int i = 0; i < asset->index_buffers.size(); ++i) {
        SAFE_DELETE(asset->index_buffers[i]);
    }

    for (int i = 0; i < asset->entities.size(); ++i) {
        gear::gEngine.GetEntityManager()->DestroyEntity(asset->entities[i]);
    }

    asset->textures.clear();
    asset->material_instances.clear();
    asset->materials.clear();
    asset->vertex_buffers.clear();
    asset->index_buffers.clear();
    asset->entities.clear();
    SAFE_DELETE(asset);
}

uint32_t GetCNodeInxFromCData(const cgltf_node* node, const cgltf_data* data)
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

bool HasGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type) {
    for (int i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute* att = &primitive->attributes[i];
        if(att->type == type) {
            return true;
        }
    }
    return false;
}

cgltf_attribute* GetGltfAttribute(cgltf_primitive* primitive, cgltf_attribute_type type) {
    for (int i = 0; i < primitive->attributes_count; i++) {
        cgltf_attribute* att = &primitive->attributes[i];
        if(att->type == type) {
            return att;
        }
    }
    return nullptr;
}

glm::mat4 GetLocalMatrix(cgltf_node* node)
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

glm::mat4 GetWorldMatrix(cgltf_node* node)
{
    cgltf_node* curNode = node;
    glm::mat4 out = GetLocalMatrix(curNode);

    while (curNode->parent != nullptr)
    {
        curNode = node->parent;
        out = GetLocalMatrix(curNode) * out;
    }
    return out;
}

void CombindVertexData(void* dst, void* src, uint32_t vertexCount, uint32_t attributeSize, uint32_t offset, uint32_t stride, uint32_t size) {
    uint8_t* dstData = (uint8_t*)dst + offset;
    uint8_t* srcData = (uint8_t*)src;
    for (uint32_t i = 0; i < vertexCount; i++) {
        memcpy(dstData, srcData, attributeSize);
        dstData += stride;
        srcData += attributeSize;
    }
}