#!/bin/bash
script_dir="$(dirname "$(readlink -f "$0")")"

# Import common paths
source ./common-paths.sh

# Install Python packages
pip install --upgrade --user ${script_dir}/../../requirements.txt

if [[ -n "${build_type}" ]]; then
    # Install Conan profile and packages
    ./setup-conan.sh
else
    # Install Conan profile and debug packages
    export build_type=Debug
    ./setup-conan.sh

    # Install Conan profile and release packages
    export build_type=Release
    ./setup-conan.sh

    # Unset build_type
    unset build_type
fi

# Run CMake Configure
./run-cmake-configure.sh
