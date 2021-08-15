import os
import sys
import json

material_file = os.path.join(os.getcwd(), sys.argv[1] + '.mat')
vs_file = os.path.join(os.getcwd(), sys.argv[1] + '.vs')
fs_file = os.path.join(os.getcwd(), sys.argv[1] + '.fs')
print(material_file)
print(vs_file)
print(fs_file)

vs_code = ''
if os.path.exists(vs_file):
    with open(vs_file, 'r') as f:
        vs_code = f.read()

fs_code = ''
if os.path.exists(fs_file):
    with open(fs_file, 'r') as f:
        fs_code = f.read()

if os.path.exists(material_file):
    material_dict = {}
    with open(material_file, 'r') as f:
        material_dict = json.loads(f.read())

    if 'vertex_code' in material_dict:
        material_dict['vertex_code'] = vs_code

    if 'fragment_code' in material_dict:
        material_dict['fragment_code'] = fs_code

    with open(material_file, 'w') as f:
        f.write(json.dumps(material_dict, indent=4))