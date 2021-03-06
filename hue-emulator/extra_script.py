import gzip
import os
import shutil

Import("env")


def cleanup_dir(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)
    for root, dirs, files in os.walk(directory):
        for dir_to_proccess in dirs:
            if not dir_to_proccess.startswith("."):
                cleanup_dir(root + "/" + dir_to_proccess)
                os.rmdir(root + "/" + dir_to_proccess)
        for name in files:
            os.remove(root + "/" + name)


def valid_ffs_name(ffs_name, ffs_max_len=30):
    if len(ffs_name) >= ffs_max_len:
        raise AttributeError(
            "File name is to long for SPIFFS max 30 characters. ", ffs_name)


def process_dir(directory):
    for root, dirs, files in os.walk(directory):
        dest_root = "./data/" + "/".join(root.split("/")[2:])
        for name in files:
            if directory.count("/") == 1:
                ffs_name = directory[len(directory):]
            else:
                ffs_name = directory[directory.find("/", 3):]
            ffs_name += "/" + name
            if name.endswith((".html", ".css", ".js")):
                valid_ffs_name(ffs_name, 27)
                with open(root + "/" + name, 'rb') as f_in, gzip.open(dest_root + "/" + name + ".gz", 'wb') as f_out:
                    shutil.copyfileobj(f_in, f_out)
            else:
                valid_ffs_name(ffs_name)
                shutil.copyfile(root + "/" + name, dest_root + "/" + name)
        for dir_to_process in dirs:
            if not dir_to_process.startswith("."):
                if not os.path.exists(dest_root + "/" + dir_to_process):
                    os.mkdir(dest_root + "/" + dir_to_process)
                process_dir(root + "/" + dir_to_process)


def compress_html_resources():
    print("Cleanup of resources for FS")
    cleanup_dir("./data")
    print("Building resources for FS")
    process_dir("./resources")


if "uploadfs" in BUILD_TARGETS or "buildfs" in BUILD_TARGETS:
    compress_html_resources()
