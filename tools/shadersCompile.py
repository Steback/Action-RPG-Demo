#!/usr/bin/python
import sys
import os

VULKAN_SDK = os.environ.get('VULKAN_SDK')
GLSLC = VULKAN_SDK + '/bin/glslc'
SHADERS_DIR = '../assets/shaders/'
BUILD_DIR = sys.argv[1] + '/shaders/'

if __name__ == "__main__":
    print("Compile Shaders!")

    if not os.path.exists(BUILD_DIR):
        os.mkdir(BUILD_DIR)

    for shader in os.listdir(SHADERS_DIR):
        cmd = GLSLC + ' -o ' + BUILD_DIR + shader + '.spv ' + SHADERS_DIR + shader
        print(cmd)
        os.system(cmd)

