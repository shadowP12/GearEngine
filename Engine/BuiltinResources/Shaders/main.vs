void main() {
    MaterialVertexParams material_params;

    InitMaterialVertexParams(material_params);

    ProcessMaterialVertexParams(material_params);

    vec4 world_position = material_params.world_position;
#if defined(HAS_SKINNING_OR_MORPHING) && defined(HAS_ATTRIBUTE_BONE_INDICES)
    mat4 skin_matrix = 
		mesh_bone_weights.x * bone_uniforms.joint_matrixs[int(mesh_bone_indices.x)] +
		mesh_bone_weights.y * bone_uniforms.joint_matrixs[int(mesh_bone_indices.y)] +
		mesh_bone_weights.z * bone_uniforms.joint_matrixs[int(mesh_bone_indices.z)] +
		mesh_bone_weights.w * bone_uniforms.joint_matrixs[int(mesh_bone_indices.w)];
    
    world_position = object_uniforms.model_matrix * skin_matrix * mesh_position;
#endif

    vertex_position = frame_uniforms.view_matrix * world_position;

    vertex_world_position = world_position.xyz;

#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material_params.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material_params.uv1;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
    vertex_color = material_params.color;
#endif

#if defined(HAS_ATTRIBUTE_NORMAL)
    vertex_normal = material_params.world_normal;
#endif

    gl_Position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * world_position;
}
