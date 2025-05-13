import subprocess
import os

is_preview = False
name = '"Zapolemos Lite 3"' if is_preview else '"Zapolemos Lite 3 (Preview)"'
version = [0, 4, 0, 0]
directory = os.path.dirname(os.path.abspath(__file__))
save_location = input("Save file to: ").replace('"','') or directory

def read_list(text:list = []): return ','.join(map(str, text))

print("Building files...")
resources = f'''1 VERSIONINFO
FILEVERSION {read_list(version)}
PRODUCTVERSION {read_list(version)}
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904E4"
        BEGIN
            VALUE "CompanyName", "Jikoac Games"
            VALUE "FileDescription", "The third iteration of Zapolemos Lite.\\r\\nThe original 2 versions were written in MicroPython.\\r\\nThis is the first version written in C++."
            VALUE "ProductName", {name}
        END
    END
END
GAMEICON ICON "{directory}\\Icons\\icon.ico"
OLDGAMEICON ICON "{directory}\\Icons\\iconOutdated.ico"
VERSIONICON ICON "{directory}\\Icons\\iconVersion.ico"
HEROICON ICON "{directory}\\Icons\\iconHero.ico"
STRINGTABLE
BEGIN

    1, "{'.'.join(map(str, version))} {'Preview' if is_preview else ''}"
    2, {name}

    101, "Version: "
    102, "Name: "

	201, "{save_location}\\Heroes\\\\"
END'''

compile_script=f'''windres "{directory}\\resource.rc" -o "{directory}\\resource.o"
g++ "{directory}\\main.cpp" "{directory}\\resource.o" -o "{save_location}\\Zapolemos Lite 3.exe"
del /F /Q "{directory}\\resource.o"'''

with open(directory+"\\resource.rc", "w") as f:
    f.write(resources)

with open(directory+"\\compile.bat", "w") as f:
    f.write(compile_script)

print("Compiling...")
subprocess.run([directory+"\\compile.bat"], shell=True)

print("Done!")