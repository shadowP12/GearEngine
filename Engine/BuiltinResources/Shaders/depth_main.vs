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

    gl_Position = frame_uniforms.proj_matrix * frame_uniforms.view_matrix * world_position;
}
