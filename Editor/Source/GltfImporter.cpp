#include "GltfImporter.h"
#include "TextureImporter.h"

#include <Utility/Log.h>
#include <Utility/Hash.h>
#include <Resource/Texture.h>
#include <Resource/Mesh.h>
#include <Resource/Material.h>
#include <Animation/Skeleton.h>
#include <Animation/AnimationClip.h>
#include <Entity/Entity.h>
#include <Entity/Components/CCamera.h>
#include <Entity/Components/CTransform.h>
#include <Entity/Components/CMesh.h>
#include <GearEngine.h>
#include <MaterialCompiler/MaterialCompiler.h>
#include <Utility/FileSystem.h>
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
        "   \"vertex_layout\": [\n"
        "       \"static_mesh\",\n"
        "       \"skin_mesh\"\n"
        "   ],\n"
        "   ${UNIFORMS}\n"
        "   ${TEXTURES}\n"
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

std::shared_ptr<gear::Material> GenMaterial(GltfMaterialConfig& config) {
    std::string code = g_gltf_material_code;

    // 设置混合模式
    if (config.blending_mode == gear::BlendStateType::BST_OPAQUE) {
        Replace(code, "${BLENDING_MODE}", "opaque");
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

    // 设置textures/samplers变量
    std::string textures_code = "";
    std::string samplers_code = "";
    if (config.has_base_color_tex || config.has_normal_tex || config.has_metallic_roughness_tex) {
        textures_code += "\"textures\": [\n";
        samplers_code += "\"samplers\": [\n";
    }
    if (config.has_base_color_tex) {
        textures_code += "{\n";
        textures_code += "\"name\": \"base_color_texture\",\n";
        textures_code += "\"type\": \"texture_2d\"\n";
        textures_code += "},\n";

        samplers_code += "base_color_sampler,\n";
    }

    if (config.has_normal_tex) {
        // TODO
        /*
        textures_code += "{\n";
        textures_code += "\"name\": \"normal_texture\",\n";
        textures_code += "\"type\": \"texture_2d\"\n";
        textures_code += "},\n";

        samplers_code += "\"normal_sampler\",\n";
        */
    }

    if (config.has_metallic_roughness_tex) {
        textures_code += "{\n";
        textures_code += "\"name\": \"metallic_roughness_texture\",\n";
        textures_code += "\"type\": \"texture_2d\"\n";
        textures_code += "},\n";

        samplers_code += "\"metallic_roughness_sampler\",\n";
    }
    if (config.has_base_color_tex || config.has_normal_tex || config.has_metallic_roughness_tex) {
        textures_code += "],";
        samplers_code += "],";
    }
    Replace(code, "${TEXTURES}", textures_code);
    Replace(code, "${SAMPLERS}", samplers_code);

    // 设置shader代码
    std::string frag_shader_code = "";
    frag_shader_code += "void ProcessMaterialFragmentParams(inout MaterialFragmentParams params) {\\n";
    if (config.has_base_color_tex) {
        frag_shader_code += "params.base_color = texture(sampler2D(base_color_texture, base_color_sampler), vertex_uv01.xy);\\n";
        frag_shader_code += "params.base_color.rgb *= params.base_color.a;\\n";
    } else {
        frag_shader_code += "params.base_color = material_uniforms.base_color;\\n";
        frag_shader_code += "params.base_color.rgb *= params.base_color.a;\\n";
    }

    if (config.has_normal_tex) {
        // TODO
    }

    if (config.has_metallic_roughness_tex) {
        frag_shader_code += "params.metallic = texture(sampler2D(metallic_roughness_texture, metallic_roughness_sampler), vertex_uv01.xy).r;\\n";
        frag_shader_code += "params.roughness = texture(sampler2D(metallic_roughness_texture, metallic_roughness_sampler), vertex_uv01.xy).g;\\n";
    } else {
        frag_shader_code += "params.metallic = material_uniforms.metallic_roughness.r;\\n";
        frag_shader_code += "params.roughness = material_uniforms.metallic_roughness.g;\\n";
    }

    frag_shader_code += "}\\n";
    Replace(code, "${FRAG_SHADER}", frag_shader_code);

    return gear::gEngine.GetMaterialCompiler()->Compile(code, false);
}

GltfAsset* ImportGltfAsset(const std::string& path) {
    GltfAsset* asset = new GltfAsset();
    std::unordered_map<GltfMaterialConfig, std::shared_ptr<gear::Material>, MurmurHash<GltfMaterialConfig>, GltfMaterialConfig::Eq> material_map;

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

    // Load gltf textures
    std::map<cgltf_image*, std::shared_ptr<blast::GfxTexture>> image_helper;
    for (int i = 0; i < data->images_count; ++i) {
        cgltf_image* cimage = &data->images[i];
        std::string dir = filesystem::path(path).parent_path().str();
        std::string image_path = dir + "/" + cimage->uri;
        std::shared_ptr<blast::GfxTexture> texture = ImportTexture2D(image_path);
        image_helper[cimage] = texture;
        if (cimage->name) {
            asset->textures[std::string(cimage->name)] = texture;
        } else {
            asset->textures[filesystem::path(path).filename()] = texture;
        }
    }

    // Load gltf materials
    uint32_t material_id = 0;
    uint32_t material_instance_id = 0;
    std::map<cgltf_material*, std::shared_ptr<gear::MaterialInstance>> material_helper;
    for (int i = 0; i < data->materials_count; ++i) {
        cgltf_material* cmaterial = &data->materials[i];
        GltfMaterialConfig config;
        if (cmaterial->alpha_mode == cgltf_alpha_mode_opaque) {
            config.blending_mode = gear::BlendStateType::BST_OPAQUE;
        } else {
            config.blending_mode = gear::BlendStateType::BST_TRANSPARENT;
        }

        if(cmaterial->normal_texture.texture) {
            config.has_normal_tex = true;
        }

        std::shared_ptr<gear::Material> material = nullptr;
        auto iter = material_map.find(config);
        if (iter != material_map.end()) {
            material = iter->second;
        } else {
            material_id++;
            material = GenMaterial(config);
            material_map[config] = material;
            asset->materials["material_" + std::to_string(material_id)] = material;
        }
        material_instance_id++;
        std::shared_ptr<gear::MaterialInstance> material_instance = material->CreateInstance();
        material_helper[cmaterial] = material_instance;
        asset->material_instances["material_instance_" + std::to_string(material_instance_id)] = material_instance;

        glm::vec4 base_color_value = glm::make_vec4(cmaterial->pbr_metallic_roughness.base_color_factor);
        material_instance->SetFloat4("base_color", base_color_value);
        
        glm::vec4 metallic_roughness_value = glm::vec4(0.0f);
        metallic_roughness_value.x = cmaterial->pbr_metallic_roughness.metallic_factor;
        metallic_roughness_value.y = cmaterial->pbr_metallic_roughness.roughness_factor;
        material_instance->SetFloat4("metallic_roughness", metallic_roughness_value);

        if(cmaterial->normal_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->normal_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->normal_texture.texture->sampler;
            blast::GfxSamplerDesc sampler_desc;
            material_instance->SetTexture("normal_texture", image_helper[cimage]);
            material_instance->SetSampler("normal_sampler", sampler_desc);
        }

        if(cmaterial->pbr_metallic_roughness.base_color_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->pbr_metallic_roughness.base_color_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->pbr_metallic_roughness.base_color_texture.texture->sampler;
            blast::GfxSamplerDesc sampler_desc;
            material_instance->SetTexture("base_color_texture", image_helper[cimage]);
            material_instance->SetSampler("base_color_sampler", sampler_desc);
        }

        if(cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture) {
            // TODO: sampler
            cgltf_image* cimage = cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture->image;
            cgltf_sampler* csampler = cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture->sampler;
            blast::GfxSamplerDesc sampler_desc;
            material_instance->SetTexture("metallic_roughness_texture", image_helper[cimage]);
            material_instance->SetSampler("metallic_roughness_sampler", sampler_desc);
        }
    }

    // Load gltf nodes
    uint32_t node_id = 0;
    std::map<cgltf_node*, std::shared_ptr<gear::Entity>> node_helper;
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
        node_id++;
        std::string node_name = "node_" + std::to_string(node_id);
        std::shared_ptr<gear::Entity> entity = gear::Entity::Create(node_name);
        asset->entities[node_name] = entity;

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
            std::shared_ptr<gear::Entity> child = node_helper[cnode];
            std::shared_ptr<gear::Entity> parent = node_helper[cnode->parent];
            child->GetComponent<gear::CTransform>()->SetParent(parent);
        }
    }

    // Load gltf skeletons
    uint32_t skeleton_id = 0;
    std::map<cgltf_skin*, std::shared_ptr<gear::Skeleton>> skin_helper;
    for (uint32_t i = 0; i < data->skins_count; ++i) {
        cgltf_skin* cskin = &data->skins[i];

        std::vector<glm::mat4> bind_poses(cskin->joints_count);
        cgltf_accessor* accessor = cskin->inverse_bind_matrices;
        cgltf_buffer_view* buffer_view = accessor->buffer_view;
        memcpy(bind_poses.data(), ((uint8_t*)(buffer_view->buffer->data) + accessor->offset + buffer_view->offset), cskin->joints_count * sizeof(glm::mat4));

        std::vector<gear::Joint> joints(cskin->joints_count);
        std::map<cgltf_node*, uint32_t> joint_helper;
        for (uint32_t j = 0; j < cskin->joints_count; ++j) {
            cgltf_node* cjoint = cskin->joints[j];
            joint_helper[cjoint] = j;
            joints[j].name = cjoint->name;
            joints[j].bind_pose = bind_poses[j];
            joints[j].pos = node_helper[cjoint]->GetComponent<gear::CTransform>()->GetPosition();
            joints[j].rot = node_helper[cjoint]->GetComponent<gear::CTransform>()->GetRotation();
            joints[j].scale = node_helper[cjoint]->GetComponent<gear::CTransform>()->GetScale();
        }

        for (uint32_t j = 0; j < cskin->joints_count; ++j) {
            cgltf_node* cjoint = cskin->joints[j];

            if(cjoint->parent != nullptr) {
                auto iter = joint_helper.find(cjoint->parent);
                if (iter != joint_helper.end()) {
                    joints[j].parent = joint_helper[cjoint->parent];
                }
            }
        }
        skeleton_id++;
        std::shared_ptr<gear::Skeleton> skeleton = std::make_shared<gear::Skeleton>(joints);
        skin_helper[cskin] = skeleton;
        asset->skeletons["skeleton_" + std::to_string(skeleton_id)] = skeleton;
    }

    // Load animation clip
    for (uint32_t i = 0; i < data->animations_count; ++i) {
        cgltf_animation* canimation = &data->animations[i];

        std::vector<gear::AnimationTrack> tracks(canimation->channels_count);
        for (uint32_t j = 0; j < canimation->channels_count; ++j) {
            cgltf_animation_channel* cchannel = &canimation->channels[j];
            cgltf_animation_sampler* csampler = cchannel->sampler;

            tracks[j].joint_name = cchannel->target_node->name;
            if (cchannel->target_path == cgltf_animation_path_type_translation) {
                tracks[j].target = gear::AnimationTarget::POSITION;
            } else if (cchannel->target_path == cgltf_animation_path_type_rotation) {
                tracks[j].target = gear::AnimationTarget::ROTATION;
            } else if (cchannel->target_path == cgltf_animation_path_type_scale) {
                tracks[j].target = gear::AnimationTarget::SCALE;
            }

            cgltf_accessor* in_accessor = csampler->input;
            cgltf_buffer_view* in_buffer_view = in_accessor->buffer_view;
            uint8_t* in_data = (uint8_t*)(in_buffer_view->buffer->data) + in_accessor->offset + in_buffer_view->offset;

            cgltf_accessor* out_accessor = csampler->output;
            cgltf_buffer_view* out_buffer_view = out_accessor->buffer_view;
            uint8_t* out_data = (uint8_t*)(out_buffer_view->buffer->data) + out_accessor->offset + out_buffer_view->offset;

            tracks[j].keys.resize(csampler->input->count);
            for (uint32_t k = 0; k < csampler->input->count; ++k) {
                tracks[j].keys[k].time = *(float*)(in_data + k * sizeof(float));

                if (cchannel->target_path == cgltf_animation_path_type_translation || cchannel->target_path == cgltf_animation_path_type_scale) {
                    tracks[j].keys[k].vec_value = *(glm::vec3*)(out_data + k * sizeof(glm::vec3));
                } else if (cchannel->target_path == cgltf_animation_path_type_rotation) {
                    tracks[j].keys[k].quat_value = *(glm::quat*)(out_data + k * sizeof(glm::quat));
                }
            }
        }

        asset->animation_clips[canimation->name] = std::make_shared<gear::AnimationClip>(tracks);
    }

    // Load gltf meshs
    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node* cnode = &data->nodes[i];
        cgltf_mesh* cmesh = cnode->mesh;
        cgltf_skin* cskin = cnode->skin;

        if(!cmesh) {
            continue;
        }

        gear::CMesh* mesh_component = node_helper[cnode]->AddComponent<gear::CMesh>();
        gear::BBox mesh_bbox;
        std::vector<std::shared_ptr<gear::SubMeshData>> sub_mesh_datas;
        std::vector<std::shared_ptr<gear::MaterialInstance>> mesh_materials;
        for (int i = 0; i < cmesh->primitives_count; i++) {
            cgltf_primitive* cprimitive = &cmesh->primitives[i];
            cgltf_material* cmaterial = cprimitive->material;
            if (cmaterial) {
                mesh_materials.push_back(material_helper[cmaterial]);
            }

            gear::VertexLayoutType vertex_layout = gear::VLT_STATIC_MESH;
            if(HasGltfAttribute(cprimitive, cgltf_attribute_type_joints)) {
                vertex_layout = gear::VLT_SKIN_MESH;
            }
            uint32_t vertex_count = GetGltfAttribute(cprimitive, cgltf_attribute_type_position)->data->count;
            uint32_t vertex_stride = gear::GetVertexLayoutStride(vertex_layout);
            uint32_t vertex_buffer_size = vertex_count * vertex_stride;

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

            bool uv_data_empty = false;
            bool normal_data_empty = false;

            uint32_t vertex_buffer_offset = 0;
            {
                positionOffset = vertex_buffer_offset;
                positionAttributeSize = sizeof(glm::vec3);
                positionDataSize = positionAttributeSize * vertex_count;
                vertex_buffer_offset += positionAttributeSize;
            }
            {
                uvOffset = vertex_buffer_offset;
                uvAttributeSize = sizeof(glm::vec2);
                uvDataSize = uvAttributeSize * vertex_count;
                vertex_buffer_offset += uvAttributeSize;
            }
            {
                normalOffset = vertex_buffer_offset;
                normalAttributeSize = sizeof(glm::vec3);
                normalDataSize = normalAttributeSize * vertex_count;
                vertex_buffer_offset += normalAttributeSize;
            }
            {
                tangentOffset = vertex_buffer_offset;
                tangentAttributeSize = sizeof(glm::vec3);
                tangentDataSize = tangentAttributeSize * vertex_count;
                vertex_buffer_offset += tangentAttributeSize;
            }
            {
                bitangentOffset = vertex_buffer_offset;
                bitangentAttributeSize = sizeof(glm::vec3);
                bitangentDataSize = bitangentAttributeSize * vertex_count;
                vertex_buffer_offset += bitangentAttributeSize;
            }
            if (vertex_layout == gear::VLT_SKIN_MESH) {
                jointOffset = vertex_buffer_offset;
                jointAttributeSize = sizeof(glm::u16vec4);
                jointDataSize = jointAttributeSize * vertex_count;
                vertex_buffer_offset += jointAttributeSize;

                weightOffset = vertex_buffer_offset;
                weightAttributeSize = sizeof(glm::vec4);
                weightDataSize = weightAttributeSize * vertex_count;
                vertex_buffer_offset += weightAttributeSize;
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

            gear::BBox primitive_bbox;
            if (positionOffset != -1) {
                cgltf_attribute* positionAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_position);
                cgltf_accessor* posAccessor = positionAttribute->data;
                cgltf_buffer_view* posView = posAccessor->buffer_view;
                positionData = (uint8_t*)(posView->buffer->data) + posAccessor->offset + posView->offset;

                const float* minp = &posAccessor->min[0];
                const float* maxp = &posAccessor->max[0];
                primitive_bbox.Grow(glm::vec3(minp[0], minp[1], minp[2]));
                primitive_bbox.Grow(glm::vec3(maxp[0], maxp[1], maxp[2]));
            }

            if (GetGltfAttribute(cprimitive, cgltf_attribute_type_texcoord)) {
                cgltf_attribute* texcoordAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_texcoord);
                cgltf_accessor* texcoordAccessor = texcoordAttribute->data;
                cgltf_buffer_view* texcoordView = texcoordAccessor->buffer_view;
                uvData = (uint8_t *) (texcoordView->buffer->data) + texcoordAccessor->offset + texcoordView->offset;
            }

            if (GetGltfAttribute(cprimitive, cgltf_attribute_type_normal)) {
                cgltf_attribute* normalAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_normal);
                cgltf_accessor* normalAccessor = normalAttribute->data;
                cgltf_buffer_view* normalView = normalAccessor->buffer_view;
                normalData = (uint8_t*)(normalView->buffer->data) + normalAccessor->offset + normalView->offset;

                cgltf_attribute* tangentAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_tangent);
                cgltf_accessor *tangentAccessor = tangentAttribute->data;
                cgltf_buffer_view *tangentView = tangentAccessor->buffer_view;
                tangentData = (uint8_t*)(tangentView->buffer->data) + tangentAccessor->offset + tangentView->offset;

                // 计算副切线
                bitangentData = new uint8_t[vertex_count * sizeof(glm::vec3)];
                float* normals = (float*)normalData;
                float* tangents = (float*)tangentData;
                float* bitangents = (float*)bitangentData;
                for (int j = 0; j < vertex_count; ++j) {
                    glm::vec3 noraml = glm::vec3(normals[j * 3], normals[j * 3 + 1], normals[j * 3 + 2]);
                    glm::vec3 tangent = glm::vec3(tangents[j * 3], tangents[j * 3 + 1], tangents[j * 3 + 2]);
                    glm::vec3 bitangent = glm::cross(noraml, tangent);
                    bitangents[j * 3] = bitangent.x;
                    bitangents[j * 3 + 1] = bitangent.y;
                    bitangents[j * 3 + 2] = bitangent.z;
                }
            }

            if (GetGltfAttribute(cprimitive, cgltf_attribute_type_joints)) {
                cgltf_attribute* jointsAttribute = GetGltfAttribute(cprimitive, cgltf_attribute_type_joints);
                cgltf_accessor* jointsAccessor = jointsAttribute->data;
                cgltf_buffer_view* jointsView = jointsAccessor->buffer_view;
                jointData = (uint8_t*)(jointsView->buffer->data) + jointsAccessor->offset + jointsView->offset;

                cgltf_attribute* weightsAttributes = GetGltfAttribute(cprimitive, cgltf_attribute_type_weights);
                cgltf_accessor* weightsAccessor = weightsAttributes->data;
                cgltf_buffer_view* weightsView = weightsAccessor->buffer_view;
                weightData = (uint8_t*)(weightsView->buffer->data) + weightsAccessor->offset + weightsView->offset;
            }

            // 填充空缺数据
            if (!uvData || normalData) {
                float* uvs = nullptr;
                if (!uvData) {
                    uv_data_empty = true;
                    uvData = new uint8_t[vertex_count * sizeof(glm::vec2)];
                    uvs = (float*)normalData;
                }

                float* normals = nullptr;
                float* tangents = nullptr;
                float* bitangents = nullptr;
                if (!normalData) {
                    normal_data_empty = true;
                    normalData = new uint8_t[vertex_count * sizeof(glm::vec3)];
                    tangentData = new uint8_t[vertex_count * sizeof(glm::vec3)];
                    bitangentData = new uint8_t[vertex_count * sizeof(glm::vec3)];
                    normals = (float*)normalData;
                    tangents = (float*)tangentData;
                    bitangents = (float*)bitangentData;
                }

                for (int j = 0; j < vertex_count; ++j) {
                    if (!uvData) {
                        uvs[j * 2] = 0.0f;
                        uvs[j * 2 + 1] = 0.0f;
                    }

                    if (!normalData) {
                        normals[j * 3] = 1.0f;
                        normals[j * 3 + 1] = 0.0f;
                        normals[j * 3 + 2] = 0.0f;

                        tangents[j * 3] = 0.0f;
                        tangents[j * 3 + 1] = 1.0f;
                        tangents[j * 3 + 2] = 0.0f;

                        bitangents[j * 3] = 0.0f;
                        bitangents[j * 3 + 1] = 0.0f;
                        bitangents[j * 3 + 2] = 1.0f;
                    }
                }
            }

            // 组装数据
            uint8_t* vertexData = new uint8_t[vertex_buffer_size];
            if (vertex_layout == gear::VLT_STATIC_MESH) {
                CombindVertexData(vertexData, positionData, vertex_count, positionAttributeSize, positionOffset, vertex_stride, positionDataSize);
                CombindVertexData(vertexData, uvData, vertex_count, uvAttributeSize, uvOffset, vertex_stride, uvDataSize);
                CombindVertexData(vertexData, normalData, vertex_count, normalAttributeSize, normalOffset, vertex_stride, normalDataSize);
                CombindVertexData(vertexData, tangentData, vertex_count, tangentAttributeSize, tangentOffset, vertex_stride, tangentDataSize);
                CombindVertexData(vertexData, bitangentData, vertex_count, bitangentAttributeSize, bitangentOffset, vertex_stride, bitangentDataSize);
            } else {
                CombindVertexData(vertexData, positionData, vertex_count, positionAttributeSize, positionOffset, vertex_stride, positionDataSize);
                CombindVertexData(vertexData, uvData, vertex_count, uvAttributeSize, uvOffset, vertex_stride, uvDataSize);
                CombindVertexData(vertexData, normalData, vertex_count, normalAttributeSize, normalOffset, vertex_stride, normalDataSize);
                CombindVertexData(vertexData, tangentData, vertex_count, tangentAttributeSize, tangentOffset, vertex_stride, tangentDataSize);
                CombindVertexData(vertexData, bitangentData, vertex_count, bitangentAttributeSize, bitangentOffset, vertex_stride, bitangentDataSize);
                CombindVertexData(vertexData, jointData, vertex_count, jointAttributeSize, jointOffset, vertex_stride, jointDataSize);
                CombindVertexData(vertexData, weightData, vertex_count, weightAttributeSize, weightOffset, vertex_stride, weightDataSize);
            }

            // Indices
            cgltf_accessor* cIndexAccessor = cprimitive->indices;
            cgltf_buffer_view* cIndexBufferView = cIndexAccessor->buffer_view;
            cgltf_buffer* cIndexBuffer = cIndexBufferView->buffer;

            uint8_t* indexData = (uint8_t*)cIndexBuffer->data + cIndexAccessor->offset + cIndexBufferView->offset;
            uint32_t index_count = cIndexAccessor->count;
            uint32_t index_data_size = 0;
            blast::IndexType index_type;
            if (cIndexAccessor->component_type == cgltf_component_type_r_16u) {
                index_type = blast::INDEX_TYPE_UINT16;
                index_data_size = index_count * sizeof(uint16_t);
            } else if (cIndexAccessor->component_type == cgltf_component_type_r_32u) {
                index_type = blast::INDEX_TYPE_UINT32;
                index_data_size = index_count * sizeof(uint32_t);
            } else {
                LOGE("Index component type not supported!\n");
            }

            std::shared_ptr<gear::SubMeshData> sub_mesh_data = gear::SubMeshData::Builder()
                    .SetVertexCount(vertex_count)
                    .SetVertexLayout(vertex_layout)
                    .SetVertexData(vertexData, vertex_buffer_size)
                    .SetIndexCount(index_count)
                    .SetIndexType(index_type)
                    .SetIndexData(indexData, index_data_size)
                    .Build();
            sub_mesh_datas.push_back(sub_mesh_data);
            mesh_bbox.Grow(primitive_bbox);

            // Release temp memory
            if (uv_data_empty) {
                SAFE_DELETE(uvData);
            }
            if (normal_data_empty) {
                SAFE_DELETE(normalData);
                SAFE_DELETE(tangentData);
            }
            SAFE_DELETE(bitangentData);
            SAFE_DELETE(vertexData);
        }

        gear::MeshData::Builder mesh_data_builder;
        for (auto sub_mesh_data : sub_mesh_datas) {
            mesh_data_builder.AddSubMeshData(sub_mesh_data);
        }
        std::shared_ptr<gear::Mesh> mesh = mesh_data_builder.Build()->LoadMesh();
        mesh_component->SetMesh(mesh);
        mesh_component->SetMaterials(mesh_materials);
        mesh_component->SetBoundBox(mesh_bbox);
        if (cskin) {
            mesh_component->SetSkeleton(skin_helper[cskin]);
        }
        asset->meshs[cmesh->name] = mesh;
    }

    cgltf_free(data);
    return asset;
}

void DestroyGltfAsset(GltfAsset* asset) {
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