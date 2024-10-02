#!/usr/bin/env python3
import os
import shutil
import subprocess
from config import *
from build_tools import *

__all__ = ["setup_toolchain"]


def find_compilers():
    find_CC()
    find_CXX()
    check_clang_selected_gcc()
    find_nvcc()


def find_nvcc():
    config["NVCC"] = find_executable("nvcc")

    if config["NVCC"] == "":
        # Try to find nvcc in the default CUDA installation path
        default_nvcc_path = "/usr/local/cuda/bin/nvcc"
        if os.path.exists(default_nvcc_path):
            config["NVCC"] = default_nvcc_path

    nvidia_smi = find_executable("nvidia-smi")

    if nvidia_smi != "":
        compute_cap = (
            subprocess.run(
                [
                    "sudo",
                    nvidia_smi,
                    "--query-gpu=compute_cap",
                    "--format=csv,noheader",
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.DEVNULL,
            )
            .stdout.decode("utf-8")
            .strip()
        )

        gpu_ccs = []
        for c in compute_cap.split("\n"):
            c_ = c.strip()
            if len(c_) > 0:
                gpu_ccs.append(float(c_))

        if len(gpu_ccs) > 0:
            # Use the lowest compute capability among multiple GPUs
            config["gpu_compute_capability"] = str(int(min(gpu_ccs) * 10))
        else:
            config["gpu_compute_capability"] = "0"

    if config["NVCC"] == "":
        config["use_cuda"] = False
        if (not config["DISABLE_CUDA"]) and (not config["quiet"]):
            print("Info: nvcc cannot be found. CUDA support is disabled.")
    else:
        if config["DISABLE_CUDA"]:
            config["use_cuda"] = False
            if not config["quiet"]:
                print(
                    "Info: nvcc found, but CUDA support is disabled in the configuration."
                )
        else:
            config["use_cuda"] = True
            if not config["quiet"]:
                print("Info: nvcc found, CUDA support is enabled.")
                print(f"NVCC: '{config['NVCC']}'")

    return config["NVCC"]


def find_CC():
    if config["COMPILER"] == "clang":
        config["CC"] = find_executable("clang")

    if config["COMPILER"] == "gcc":
        config["CC"] = find_executable("gcc")

    if config["COMPILER"] == "icx":
        config["CC"] = find_executable("icx")

    print(f"CC: '{config['CC']}'")

    if config["CC"] == "":
        print("Critical: C compiler cannot be found")

    return config["CC"]


def find_CXX():
    if config["COMPILER"] == "clang":
        config["CXX"] = find_executable("clang++")

    if config["COMPILER"] == "gcc":
        config["CXX"] = find_executable("g++")

    if config["COMPILER"] == "icx":
        config["CXX"] = find_executable("icpx")

    print(f"CXX: '{config['CXX']}'")

    if config["CXX"] == "":
        print("Critical: C++ compiler cannot be found")

    return config["CXX"]


def cpp_compiler_sanity_check():
    if config["dry_run"]:
        return True

    os.makedirs(config["tmp_dir"], exist_ok=True)
    hello_world_cpp = os.path.join(config["tmp_dir"], "hello_world.cpp")
    with open(hello_world_cpp, "w", newline="", encoding="utf-8") as f:
        f.write("#include <iostream>\n")
        f.write('int main() { std::cout << "Hello World!" << std::endl; return 0; }\n')

    output = subprocess.run(
        [
            config["CXX"],
            "-o",
            os.path.join(config["tmp_dir"], "hello_world"),
            hello_world_cpp,
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return output.returncode == 0


def check_clang_selected_gcc():
    """
    Fix a known issue with clang on Ubuntu after installing cuda.
    """
    if cpp_compiler_sanity_check():
        return

    if config["COMPILER"] != "clang" or linux_distro() != "ubuntu":
        return

    output = subprocess.run(
        ["clang", "-v"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=True
    ).stdout.decode("utf-8")
    output_lines = output.split("\n")
    selected_gcc_version = ""
    for line in output_lines:
        l = line.strip()
        if l.startswith("Selected GCC installation"):
            selected_gcc_version = l.split("/")[-1]
            break
    print(f"Clang selected GCC version: {selected_gcc_version}")

    if config["dry_run"]:
        return

    if not is_package_installed(f"gcc-{selected_gcc_version}"):
        print("Installing Clang selected gcc version...")
        subprocess.run(
            ["sudo", "apt", "install", "-qq", "-y", f"gcc-{selected_gcc_version}"],
            stdout=subprocess.DEVNULL if config["quiet"] else None,
            stderr=subprocess.DEVNULL if config["quiet"] else None,
            check=True,
        )

    if not is_package_installed(f"g++-{selected_gcc_version}"):
        print("Installing Clang selected g++ version...")
        subprocess.run(
            ["sudo", "apt", "install", "-qq", "-y", f"g++-{selected_gcc_version}"],
            stdout=subprocess.DEVNULL if config["quiet"] else None,
            stderr=subprocess.DEVNULL if config["quiet"] else None,
            check=True,
        )


def create_version_file(data_dir, version):
    if config["dry_run"]:
        return

    with open(os.path.join(data_dir, version), "w") as f:
        f.write(f"{version}\n")


def check_readline():
    if config["verbose"]:
        print("Checking readline")

    if config["dry_run"]:
        return

    readline_header_path = "/usr/include/readline/readline.h"

    pkgmgr2package = {
        "apt": "libreadline-dev",
        "dnf": "readline-devel",
        "zypper": "readline-devel",
        "pacman": "readline",
    }

    if not os.path.exists(readline_header_path):
        find_package_manager()
        if config["package_manager"] == "unknown":
            print(
                "Cannot find a supported package manager, please install readline manually"
            )
        else:
            install_packages([pkgmgr2package[config["package_manager"]]])
    else:
        if config["verbose"]:
            print("readline found")


def check_third_parties():
    if config["verbose"]:
        print("Checking libraries...")
    check_readline()


def setup_toolchain():
    install_build_tools()
    find_compilers()
    check_third_parties()


if __name__ == "__main__":
    setup_toolchain()
