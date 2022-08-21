from operator import index
import os

def is_atmosphere_shader(shader_name):
    if shader_name.find('atmosphere') >= 0:
        return True
    return False

def handle_atmosphere_shader(shader_datas, shader_name, shader_path):
    if shader_name == 'atmosphere_comon.fs':
        return

    common_path = os.path.join(os.path.dirname(shader_path), 'atmosphere_comon.fs')
    common_data = ''
    with open(common_path, 'r', encoding='utf8') as f:
        file_data = f.read()
        common_data = file_data.replace("\n", "\\n")

    prefix = shader_name.split(".")[0]
    suffix = shader_name.split(".")[1]
    data_name = prefix.upper() + '_' + suffix.upper() + '_DATA'
    with open(shader_path, 'r', encoding='utf8') as f:
        file_data = f.read()
    shader_datas[data_name] = common_data + '\\n' + file_data.replace("\n", "\\n")

def handle_common_shader(shader_datas, shader_name, shader_path):
    prefix = shader_name.split(".")[0]
    suffix = shader_name.split(".")[1]
    data_name = prefix.upper() + '_' + suffix.upper() + '_DATA'
    with open(shader_path, 'r', encoding='utf8') as f:
        file_data = f.read()
    shader_datas[data_name] = file_data.replace("\n", "\\n")

def main():
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
        shader_path = shader_paths[idx]

        if is_atmosphere_shader(shader_name):
            handle_atmosphere_shader(shader_datas, shader_name, shader_path)
            continue

        handle_common_shader(shader_datas, shader_name, shader_path)

    content = ''
    content = content + '#pragma once\n'
    content = content + 'namespace gear {\n'
    for key in shader_datas:
        content = content + 'const char ' + key + '[] = \"' + shader_datas[key] + '\";\n'
    content = content + '}\n'

    gen_file = os.path.join(gen_dir, 'ShaderCode.h')
    with open(gen_file, 'w') as f:
        f.write(content)

if __name__ == '__main__':
    main()