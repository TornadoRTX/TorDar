#!/bin/bash
script_dir="$(dirname "$(readlink -f "$0")")"

export build_dir=${1:-${script_dir}/../build-release}
export build_type=Release
export conan_profile=${2:-scwx-linux_gcc-11}
export generator=Ninja
export qt_base=/opt/Qt
export qt_arch=gcc_64

# Perform common setup
./lib/setup-common.sh
