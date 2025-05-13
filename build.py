import subprocess
import os
import json

directory = os.path.dirname(os.path.abspath(__file__))

with open(directory+"\\data\\manifest.json", "r") as f:
    manifest = json.load(f)
    name = manifest['name']
    is_preview = manifest['preview']
    version = manifest['version']
    modded = manifest['modded']
    f.close()

name = f'{name} {'(Preview, Modded)' if modded else '(Preview)'}' if is_preview else f'{name} {'(Modded)' if modded else ''}'
if os.path.exists(directory+"\\data\\path.txt"):
    with open(directory+"\\data\\path.txt", "r") as f:
        save_location = f.read()
        save_location = save_location.replace('"','')
else:
    save_location = input("Save file to: ").replace('"','') or directory.replace('"','')

game_version = '.'.join(map(str, version))+(' Preview' if is_preview else '')+(' Modded' if modded else '')
display_version = ','.join(map(str, version))
icon_path =directory + ("\\Icons\\preview" if is_preview else "\\Icons")   

code = {
    '@version': display_version,
    '@name': name,
    '@game_version': game_version,
    '@save_path': save_location,
    '@path': directory,
    '@icon_path': icon_path,
}

def read_list(text:list = []): return ','.join(map(str, text))

print("Building files...")
with open(directory+"\\data\\resources.txt", "r") as f:
    resources = f.read()
    for x,y in code.items():
        resources = resources.replace(x, y)
    f.close()

with open(directory+'\\data\\compile.txt', 'r') as f:
    compile_script = f.read()
    for x,y in code.items():
        compile_script = compile_script.replace(x, y)
    f.close()

with open(directory+"\\resource.rc", "w") as f:
    f.write(resources)
    f.close()

with open(directory+"\\compile.bat", "w") as f:
    f.write(compile_script)
    f.close()

print("Compiling...")
subprocess.run([directory+"\\compile.bat"], shell=True)

print("Done! You can find the file in "+save_location)