#include "Scene.h"
#include "GearEngine.h"
#include "EntityManager.h"
#include "Entity/Entity.h"
#include "Components/CCamera.h"
#include "Components/CLight.h"
#include "Components/CMesh.h"
#include "Components/CSkybox.h"
#include "Components/CTransform.h"
#include "Resource/GpuBuffer.h"
#include "Resource/Texture.h"
#include "Resource/Material.h"
#include "Animation/Skeleton.h"

#include <Blast/Gfx/GfxDevice.h>

namespace gear {
    Scene::Scene() {
    }

    Scene::~Scene() {
        if (renderables_ub) {
            gEngine.GetDevice()->DestroyBuffer(renderables_ub);
        }
    }

    void Scene::AddEntity(Entity* entity) {
        auto iter = find(entities.begin( ), entities.end( ), entity);
        if (iter != entities.end()) {
            return;
        }

        if (entity->HasComponent<CMesh>()) {
            num_mesh_entitys++;
        }

        entities.push_back(entity);
    }

    void Scene::RemoveEntity(Entity* entity) {
        auto iter = find(entities.begin( ), entities.end( ), entity);
        if (iter != entities.end()) {
            if (entity->HasComponent<CMesh>()) {
                num_mesh_entitys--;
            }
            entities.erase(iter);
        }
    }

    bool Scene::Prepare(blast::GfxCommandBuffer* cmd) {
        // 生成渲染器需要的数据
        uint32_t num_cameras = 0;
        uint32_t num_renderables = 0;
        num_mesh_renderables = 0;
        num_ui_renderables = 0;
        light_info.has_direction_light = false;
        light_info.has_ibl = false;
        skybox_map = nullptr;

        if (renderables.size() < num_mesh_entitys) {
            renderables.resize(num_mesh_entitys);
            ui_renderables.resize(num_mesh_entitys);
            mesh_renderables.resize(num_mesh_entitys);
            renderables_storage.resize(num_mesh_entitys);

            blast::GfxBuffer* old_renderables_ub = renderables_ub;
            if (old_renderables_ub) {
                gEngine.GetDevice()->DestroyBuffer(old_renderables_ub);
            }

            blast::GfxBufferDesc buffer_desc;
            buffer_desc.size = sizeof(RenderableUniforms) * num_mesh_entitys;
            buffer_desc.mem_usage = blast::MEMORY_USAGE_GPU_ONLY;
            buffer_desc.res_usage = blast::RESOURCE_USAGE_UNIFORM_BUFFER;
            renderables_ub = gEngine.GetDevice()->CreateBuffer(buffer_desc);
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
                        light_info.irradiance_map = entity->GetComponent<CLight>()->GetIrradianceMap()->GetTexture();
                        light_info.prefiltered_map = entity->GetComponent<CLight>()->GetPrefilteredMap()->GetTexture();
                        light_info.lut = entity->GetComponent<CLight>()->GetBRDFLut()->GetTexture();
                    }
                }
            }

            if (entity->HasComponent<CSkybox>()) {
                skybox_map = entity->GetComponent<CSkybox>()->GetCubeMap()->GetTexture();
            }

            if (entity->HasComponent<CMesh>()) {
                glm::mat4 local_matrix = entity->GetComponent<CTransform>()->GetTransform();
                glm::mat4 model_matrix = entity->GetComponent<CTransform>()->GetWorldTransform();
                glm::mat4 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model_matrix)));
                CMesh* cmesh = entity->GetComponent<CMesh>();

                renderables[num_renderables].num_primitives = cmesh->_sub_meshs.size();
                for (uint32_t i = 0; i < cmesh->_sub_meshs.size(); ++i) {
                    // 更新材质信息
                    renderables[num_renderables].primitives[i].mi = cmesh->_sub_meshs[i].mi;
                    renderables[num_renderables].primitives[i].cast_shadow = cmesh->_sub_meshs[i].cast_shadow;
                    renderables[num_renderables].primitives[i].receive_shadow = cmesh->_sub_meshs[i].receive_shadow;
                    renderables[num_renderables].primitives[i].material_ub = cmesh->_sub_meshs[i].mi->GetUniformBuffer();
                    if (cmesh->_sub_meshs[i].mi->GetUniformBuffer() && cmesh->_sub_meshs[i].mi->IsStorageDirty()) {
                        uint8_t* storage_data = cmesh->_sub_meshs[i].mi->GetStorage();
                        uint32_t storage_size = cmesh->_sub_meshs[i].mi->GetStorageSize();
                        gEngine.GetDevice()->UpdateBuffer(cmd, renderables[num_renderables].primitives[i].material_ub->GetHandle(), storage_data, storage_size);
                    }

                    // 更新绘制信息
                    renderables[num_renderables].primitives[i].ib = cmesh->_sub_meshs[i].ib;
                    renderables[num_renderables].primitives[i].vb = cmesh->_sub_meshs[i].vb;
                    renderables[num_renderables].primitives[i].topo = cmesh->_sub_meshs[i].topo;
                    renderables[num_renderables].primitives[i].count = cmesh->_sub_meshs[i].count;
                    renderables[num_renderables].primitives[i].offset = cmesh->_sub_meshs[i].offset;

                    // 更新包围盒
                    renderables[num_renderables].primitives[i].bbox = cmesh->_sub_meshs[i].bbox;
                    renderables[num_renderables].primitives[i].bbox.bb_min = TransformPoint(renderables[num_renderables].primitives[i].bbox.bb_min, model_matrix);
                    renderables[num_renderables].primitives[i].bbox.bb_max = TransformPoint(renderables[num_renderables].primitives[i].bbox.bb_max, model_matrix);
                }

                renderables[num_renderables].bone_ub = nullptr;
                if (cmesh->skeleton && cmesh->skeleton->Prepare(cmd)) {
                    renderables[num_renderables].bone_ub = cmesh->skeleton->GetUniformBuffer();
                }

                // 更新renderable
                renderables[num_renderables].renderable_ub = renderables_ub;
                renderables[num_renderables].renderable_ub_size = sizeof(RenderableUniforms);
                renderables[num_renderables].renderable_ub_offset = num_renderables * sizeof(RenderableUniforms);
                renderables_storage[num_renderables].local_matrix = local_matrix;
                renderables_storage[num_renderables].model_matrix = model_matrix;
                renderables_storage[num_renderables].normal_matrix = normal_matrix;

                // 归类
                RenderableType type = cmesh->GetRenderableType();
                if (type == RENDERABLE_COMMON) {
                    mesh_renderables[num_mesh_renderables] = num_renderables;
                    num_mesh_renderables++;
                } else if (type == RENDERABLE_UI) {
                    ui_renderables[num_ui_renderables] = num_renderables;
                    num_ui_renderables++;
                }

                num_renderables++;
            }
        }

        // 更新整个scene的uniform buffer
        gEngine.GetDevice()->UpdateBuffer(cmd, renderables_ub, renderables_storage.data(), renderables_storage.size() * sizeof(RenderableUniforms));

        // 若当前场景不存在相机或没有renderable，则不进行渲染
        if (num_cameras == 0 || num_renderables == 0) {
            return false;
        }

        return true;
    }
}