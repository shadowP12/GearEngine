void MaterialFragment(inout MaterialFragmentInputs material) {
    material.base_color = vertex_color * texture(albedo_texture, vertex_uv01.xy);
    material.base_color.rgb *= material.base_color.a;
}