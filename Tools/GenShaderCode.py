import os

script_dir = os.path.abspath(os.path.dirname(__file__))
shaders_dir = os.path.abspath(os.path.join(script_dir, '../BuiltinResources/Shaders'))
gen_dir = os.path.abspath(os.path.join(script_dir, '../Engine/MaterialCompiler'))

shader_datas = {}
shader_names = os.listdir(shaders_dir)
for shader_name in shader_names:
    prefix = shader_name.split(".")[0]
    suffix = shader_name.split(".")[1]
    data_name = prefix.upper() + '_' + suffix.upper() + '_DATA'
    shader_path = os.path.join(shaders_dir, shader_name)
    with open(shader_path, 'r') as f:
        file_data = f.read()
        shader_datas[data_name] = file_data.replace("\n", "")

content = ''
content = content + '#pragma once\n'
content = content + 'namespace gear {\n'
for key in shader_datas:
    content = content + 'const char ' + key + '[] = \"' + shader_datas[key] + '\";\n'
content = content + '}\n'

gen_file = os.path.join(gen_dir, 'ShaderCode.h')
with open(gen_file, 'w') as f:
    f.write(content)