import os, os.path
import subprocess
import zipfile
import re

base_dir = os.path.dirname(__file__)
subprocess.call('Devenv "' + os.path.join(base_dir, 'mmo.sln') + '" /build release', shell=True)

version_header = open(os.path.join(base_dir, 'client/version.hpp')).read()
ver_major =    re.search("#define MMO_VERSION_MAJOR\s+(\d+)", version_header).group(1)
ver_minor =    re.search("#define MMO_VERSION_MINOR\s+(\d+)", version_header).group(1)
ver_revision = re.search("#define MMO_VERSION_REVISION\s+(\d+)", version_header).group(1)

ver_text = ver_major + "." + ver_minor + "." + ver_revision

build_version_path = os.path.join(base_dir, 'client/buildversion.hpp')

if os.path.exists(build_version_path):
    build_version_header = open(build_version_path).read()
    ver_build =    re.search("#define MMO_VERSION_BUILD\s+(\d+)", build_version_header).group(1)
    ver_text += "_" + ver_build

zip_path = os.path.join(base_dir, "mmo-" + ver_text + ".zip")
zip = zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED)

bin_path = os.path.join(base_dir, 'client/bin/')


zip.write(os.path.join(base_dir, 'release/client.exe'), 'client.exe')

zip.write(os.path.join(base_dir, 'readme.txt'), 'readme.txt')
zip.write(os.path.join(base_dir, 'license.txt'), 'license.txt')
zip.write(os.path.join(base_dir, 'mmd.txt'), 'mmd.txt')

zip.write(os.path.join(bin_path, 'config.json'), 'config.json')
zip.write(os.path.join(bin_path, 'server/server.exe'), 'server/server.exe')

for root, dirs, files in os.walk(os.path.join(bin_path, 'cards')):
    for file in files:
    	absolute_path = os.path.join(root, file)
        relative_path = os.path.relpath(absolute_path, bin_path)
        zip.write(absolute_path, relative_path)
        
for root, dirs, files in os.walk(os.path.join(bin_path, 'resources')):
    for file in files:
    	absolute_path = os.path.join(root, file)
        relative_path = os.path.relpath(absolute_path, bin_path)
        zip.write(absolute_path, relative_path)