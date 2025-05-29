#!/bin/bash
script_dir="$(dirname "$(readlink -f "$0")")"

export build_dir="${1:-${script_dir}/../build}"
export conan_profile=${2:-scwx-linux_gcc-11}
export generator="Ninja Multi-Config"
export qt_base=/opt/Qt
export qt_arch=gcc_64

# FIXME: aws-sdk-cpp fails to configure using Ninja Multi-Config
echo "Ninja Multi-Config is not supported in Linux"
read -p "Press Enter to continue..."

# Perform common setup
# ${script_dir}/lib/setup-common.sh
