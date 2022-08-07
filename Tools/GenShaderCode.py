from operator import index
import os

script_dir = os.path.abspath(os.path.dirname(__file__))
shaders_dir = os.path.abspath(os.path.join(script_dir, '../Engine/BuiltinResources/Shaders'))
gen_dir = os.path.abspath(os.path.join(script_dir, '../Engine/Source/MaterialCompiler'))

shader_names = []
shader_paths = []
for root, dirs, files in os.walk(shaders_dir):
    for shader_file in files:
        shader_names.append(shader_file)
        shader_paths.append(os.path.join(root, shader_file))

shader_datas = {}
for shader_name in shader_names:
    idx = shader_names.index(shader_name)
    prefix = shader_name.split(".")[0]
    suffix = shader_name.split(".")[1]
    data_name = prefix.upper() + '_' + suffix.upper() + '_DATA'
    shader_path = shader_paths[idx]
    with open(shader_path, 'r') as f:
        file_data = f.read()
        shader_datas[data_name] = file_data.replace("\n", "\\n")

content = ''
content = content + '#pragma once\n'
content = content + 'namespace gear {\n'
for key in shader_datas:
    content = content + 'const char ' + key + '[] = \"' + shader_datas[key] + '\";\n'
content = content + '}\n'

gen_file = os.path.join(gen_dir, 'ShaderCode.h')
with open(gen_file, 'w') as f:
    f.write(content)