import os
import sys
import zipfile


def zipdir(path, ziph):
    # ziph is zipfile handle
    for root, dirs, files in os.walk(path):
        for file in files:
            ziph.write(os.path.join(root, file))


if __name__ == "__main__":
    zipf = zipfile.ZipFile('Action-RPG-Demo.zip', 'w', zipfile.ZIP_DEFLATED)
    zipdir("Assets", zipf)

    if sys.platform == "linux":
        zipf.write("bin/Game")
        zipf.write("bin/Editor")
    elif sys.platform == "win32":
        zipdir("bin/Release", zipf)

    zipdir("bin/shaders", zipf)
    zipdir("data", zipf)
    zipdir("scripts", zipf)
    zipf.close()
