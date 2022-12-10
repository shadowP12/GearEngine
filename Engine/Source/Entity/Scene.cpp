#include "Scene.h"
#include "GearEngine.h"
#include "Renderer/Renderer.h"
#include "Entity/Entity.h"
#include "Components/CCamera.h"
#include "Components/CLight.h"
#include "Components/CMesh.h"
#include "Components/CSkybox.h"
#include "Components/CTransform.h"
#include "Components/CAtmosphere.h"
#include "Resource/Mesh.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Animation/Skeleton.h"

namespace gear {
    Scene::Scene(const std::string& name) {
        this->name = name;
    }

    Scene::~Scene() {
        SAFE_DELETE(renderables_ub);
    }

    void Scene::AddEntity(std::shared_ptr<Entity> entity) {
        auto iter = find(entities.begin( ), entities.end( ), entity);
        if (iter != entities.end()) {
            return;
        }

        if (entity->HasComponent<CMesh>()) {
            num_mesh_entitys++;
        }

        entities.push_back(entity);
    }

    void Scene::RemoveEntity(std::shared_ptr<Entity> entity) {
        auto iter = find(entities.begin( ), entities.end( ), entity);
        if (iter != entities.end()) {
            if (entity->HasComponent<CMesh>()) {
                num_mesh_entitys--;
            }
            entities.erase(iter);
        }
    }

    std::shared_ptr<Scene> Scene::Create(const std::string &name) {
        return std::make_shared<Scene>(name);
    }

    bool Scene::Prepare(blast::GfxCommandBuffer* cmd) {
        blast::GfxDevice* device = gEngine.GetRenderer()->GetDevice();

        // Prepare scene render data
        uint32_t num_cameras = 0;
        uint32_t num_renderables = 0;
        num_mesh_renderables = 0;
        light_info.has_direction_light = false;
        light_info.has_ibl = false;
        skybox_map = nullptr;
        should_render_atmosphere = false;

        if (renderables.size() < num_mesh_entitys) {
            renderables.resize(num_mesh_entitys);
            mesh_renderables.resize(num_mesh_entitys);
            renderables_storage.resize(num_mesh_entitys);

            blast::GfxBuffer* old_renderables_ub = renderables_ub;
            if (old_renderables_ub) {
                SAFE_DELETE(old_renderables_ub);
            }

            blast::GfxBufferDesc buffer_desc{};
            buffer_desc.size = sizeof(RenderableUniforms) * num_mesh_entitys;
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
            renderables_ub = device->CreateBuffer(buffer_desc);
        }

        for (auto entity : entities) {
            if (entity->HasComponent<CCamera>()) {
                CCamera* ccamera = entity->GetComponent<CCamera>();

                if (ccamera->GetDisplay()) {
                    display_camera_info.zn = ccamera->GetNear();
                    display_camera_info.zf = ccamera->GetFar();
                    display_camera_info.model = ccamera->GetModelMatrix();
                    display_camera_info.view = ccamera->GetViewMatrix();
                    display_camera_info.projection = ccamera->GetProjMatrix();
                    display_camera_info.position = GetTranslate(display_camera_info.model);
                    display_camera_info.view_direction = entity->GetComponent<CTransform>()->GetFrontVector();
                    display_camera_info.ev100 = std::log2((ccamera->GetAperture() * ccamera->GetAperture()) / ccamera->GetShutterSpeed() * 100.0f / ccamera->GetSensitivity());
                    display_camera_info.exposure = 1.0f / (1.2f * std::pow(2.0, display_camera_info.ev100));
                }

                if (ccamera->GetMain()) {
                    main_camera_info.zn = ccamera->GetNear();
                    main_camera_info.zf = ccamera->GetFar();
                    main_camera_info.model = ccamera->GetModelMatrix();
                    main_camera_info.view = ccamera->GetViewMatrix();
                    main_camera_info.projection = ccamera->GetProjMatrix();
                    main_camera_info.position = GetTranslate(main_camera_info.model);
                    main_camera_info.view_direction = entity->GetComponent<CTransform>()->GetFrontVector();
                    main_camera_info.ev100 = std::log2((ccamera->GetAperture() * ccamera->GetAperture()) / ccamera->GetShutterSpeed() * 100.0f / ccamera->GetSensitivity());
                    main_camera_info.exposure = 1.0f / (1.2f * std::pow(2.0, main_camera_info.ev100));
                }

                num_cameras++;
            }

            if (entity->HasComponent<CLight>()) {
                if (entity->GetComponent<CLight>()->GetLightType() == CLight::LightType::DIRECTION) {
                    light_info.has_direction_light = true;
                    light_info.sun_direction = entity->GetComponent<CTransform>()->GetFrontVector();
                    glm::vec3 light_color = entity->GetComponent<CLight>()->GetColor();
                    float light_intensity = entity->GetComponent<CLight>()->GetIntensity();
                    light_info.sun_color_intensity = glm::vec4(light_color, light_intensity);
                    glm::normalize(light_info.sun_direction);
                } else if (entity->GetComponent<CLight>()->GetLightType() == CLight::LightType::IBL) {
                    if (entity->GetComponent<CLight>()->GetIrradianceMap() &&
                        entity->GetComponent<CLight>()->GetPrefilteredMap() &&
                        entity->GetComponent<CLight>()->GetBRDFLut()) {
                        light_info.has_ibl = true;
                        light_info.irradiance_map = entity->GetComponent<CLight>()->GetIrradianceMap().get();
                        light_info.prefiltered_map = entity->GetComponent<CLight>()->GetPrefilteredMap().get();
                        light_info.lut = entity->GetComponent<CLight>()->GetBRDFLut().get();
                    }
                }
            }

            if (entity->HasComponent<CSkybox>()) {
                skybox_map = entity->GetComponent<CSkybox>()->GetCubeMap().get();
            }

            if (entity->HasComponent<CAtmosphere>()) {
                should_render_atmosphere = true;

                atmosphere_parameters.bottom_radius = 6360.0f;
                atmosphere_parameters.top_radius = 6460.0f;

                atmosphere_parameters.rayleigh_density_exp_scale = -1.0f / 8.0f;
                atmosphere_parameters.rayleigh_scattering = glm::vec4(0.005802f, 0.013558f, 0.033100f, 0.0f);

                atmosphere_parameters.mie_density_exp_scale = -1.0f / 1.2f;
                atmosphere_parameters.mie_scattering = glm::vec4(0.003996f, 0.003996f, 0.003996f, 0.0f);
                atmosphere_parameters.mie_extinction = glm::vec4(0.004440f, 0.004440f, 0.004440f, 0.0f);
                atmosphere_parameters.mie_absorption = atmosphere_parameters.mie_extinction - atmosphere_parameters.mie_scattering;
                atmosphere_parameters.mie_absorption.x = glm::max(0.0f, atmosphere_parameters.mie_absorption.x);
                atmosphere_parameters.mie_absorption.y = glm::max(0.0f, atmosphere_parameters.mie_absorption.y);
                atmosphere_parameters.mie_absorption.z = glm::max(0.0f, atmosphere_parameters.mie_absorption.z);
                atmosphere_parameters.mie_phase_g = 0.8f;

                atmosphere_parameters.absorption_density0_layer_width = 25.0f;
                atmosphere_parameters.absorption_density0_constant_term = -2.0f / 3.0f;
                atmosphere_parameters.absorption_density0_linear_term = 1.0f / 15.0f;
                atmosphere_parameters.absorption_density1_constant_term = 8.0f / 3.0f;
                atmosphere_parameters.absorption_density1_linear_term = -1.0f / 15.0f;
                atmosphere_parameters.absorption_extinction = glm::vec4(0.000650f, 0.001881f, 0.000085f, 0.0f);

                atmosphere_parameters.ground_albedo = glm::vec4(0.0f);
            }

            if (entity->HasComponent<CMesh>()) {
                glm::mat4 local_matrix = entity->GetComponent<CTransform>()->GetTransform();
                glm::mat4 model_matrix = entity->GetComponent<CTransform>()->GetWorldTransform();
                glm::mat4 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
                CMesh* cmesh = entity->GetComponent<CMesh>();
                std::shared_ptr<Mesh> mesh = cmesh->mesh;
                renderables[num_renderables].num_primitives = mesh->GetSubMeshs().size();
                for (uint32_t i = 0; i < mesh->GetSubMeshs().size(); ++i) {
                    // Update material data
                    std::shared_ptr<MaterialInstance> mi = cmesh->material_list[i];
                    if (!mi) {
                        continue;
                    }
                    renderables[num_renderables].primitives[i].mi = mi.get();
                    renderables[num_renderables].primitives[i].topo = mi->GetMaterial()->GetTopology();
                    renderables[num_renderables].primitives[i].material_ub = mi->GetUniformBuffer();
                    if (mi->GetUniformBuffer() && mi->IsStorageDirty()) {
                        uint8_t* storage_data = mi->GetStorage();
                        uint32_t storage_size = mi->GetStorageSize();
                        gEngine.GetRenderer()->UpdateUniformBuffer(cmd, renderables[num_renderables].primitives[i].material_ub, storage_data, storage_size);
                    }

                    // Update relevance
                    renderables[num_renderables].primitives[i].cast_shadow = cmesh->cast_shadow;
                    renderables[num_renderables].primitives[i].receive_shadow = cmesh->receive_shadow;

                    // Update mesh info
                    std::shared_ptr<Mesh::SubMesh> sub_mesh = mesh->GetSubMeshs()[i];
                    renderables[num_renderables].primitives[i].ib = sub_mesh->index_buf.get();
                    renderables[num_renderables].primitives[i].vb = sub_mesh->vertex_buf.get();
                    renderables[num_renderables].primitives[i].index_count = sub_mesh->vertex_count;
                    renderables[num_renderables].primitives[i].vertex_layout = sub_mesh->vertex_layout;
                    renderables[num_renderables].primitives[i].index_count = sub_mesh->index_count;
                    renderables[num_renderables].primitives[i].index_type = sub_mesh->index_type;

                    // Update bbox
                    renderables[num_renderables].primitives[i].bbox = cmesh->bbox;
                    renderables[num_renderables].primitives[i].bbox.bb_min = TransformPoint(renderables[num_renderables].primitives[i].bbox.bb_min, model_matrix);
                    renderables[num_renderables].primitives[i].bbox.bb_max = TransformPoint(renderables[num_renderables].primitives[i].bbox.bb_max, model_matrix);
                }

                renderables[num_renderables].bone_ub = nullptr;
                if (cmesh->skeleton && cmesh->skeleton->Prepare(cmd)) {
                    renderables[num_renderables].bone_ub = cmesh->skeleton->GetBoneMatrixBuffer();
                }

                // Update renderable
                renderables[num_renderables].renderable_ub = renderables_ub;
                renderables[num_renderables].renderable_ub_size = sizeof(RenderableUniforms);
                renderables[num_renderables].renderable_ub_offset = num_renderables * sizeof(RenderableUniforms);
                renderables_storage[num_renderables].local_matrix = local_matrix;
                renderables_storage[num_renderables].model_matrix = model_matrix;
                renderables_storage[num_renderables].normal_matrix = normal_matrix;

                // Sort out
                mesh_renderables[num_mesh_renderables] = num_renderables;
                num_mesh_renderables++;
                num_renderables++;
            }
        }

        if (num_cameras == 0 || num_renderables == 0) {
            return false;
        }

        // Update all renderable uniform buffer
        if (renderables_storage.size()) {
            gEngine.GetRenderer()->UpdateUniformBuffer(cmd, renderables_ub, renderables_storage.data(), renderables_storage.size() * sizeof(RenderableUniforms));
        }

        // Atmosphere
        if (should_render_atmosphere) {
            if (light_info.has_direction_light) {
                atmosphere_parameters.sun_direction = glm::vec4(light_info.sun_direction, 0.0f);
            } else {
                atmosphere_parameters.sun_direction = glm::vec4(glm::normalize(glm::vec3(0.0f, -1.0f, 1.0f)), 0.0f);
            }

            atmosphere_parameters.view_direction = glm::vec4(display_camera_info.view_direction, 0.0f);
            atmosphere_parameters.view_position = glm::vec4(display_camera_info.position, 0.0f);

            atmosphere_parameters.sky_inv_view_proj_mat = glm::inverse(display_camera_info.projection * display_camera_info.view);
        }

        return true;
    }
}