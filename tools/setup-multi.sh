#!/bin/bash
export build_dir=${1:-build-release}
export conan_profile=${2:-scwx-linux_gcc-11}
export generator=Ninja
export qt_base=/opt/Qt
export qt_arch=gcc_64

# Perform common setup
./lib/setup-common.sh
