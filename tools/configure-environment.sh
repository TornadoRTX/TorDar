#!/bin/bash
script_dir="$(dirname "$(readlink -f "$0")")"

IN_VENV=$(python -c 'import sys; print(sys.prefix != getattr(sys, "base_prefix", sys.prefix))')

if [ "${IN_VENV}" = "True" ]; then
    # In a virtual environment, don't use --user
    PIP_FLAGS="--upgrade"
else
    # Not in a virtual environment, use --user
    PIP_FLAGS="--upgrade --user"
fi

# Install Python packages
pip install ${PIP_FLAGS} -r "${script_dir}/../requirements.txt"

# Configure default Conan profile
conan profile detect -e

# Conan profiles
conan_profiles=(
    "scwx-linux_clang-17"
    "scwx-linux_clang-17_armv8"
    "scwx-linux_clang-18"
    "scwx-linux_clang-18_armv8"
    "scwx-linux_gcc-11"
    "scwx-linux_gcc-11_armv8"
    "scwx-linux_gcc-12"
    "scwx-linux_gcc-12_armv8"
    "scwx-linux_gcc-13"
    "scwx-linux_gcc-13_armv8"
    "scwx-linux_gcc-14"
    "scwx-linux_gcc-14_armv8"
    )

# Install Conan profiles
for profile_name in "${conan_profiles[@]}"; do
    conan config install "${script_dir}/conan/profiles/${profile_name}" -tf profiles
done
