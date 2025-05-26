#!/bin/bash
script_dir="$(dirname "$(readlink -f "$0")")"

# Import common paths
source ${script_dir}/common-paths.sh

IN_VENV=$(python -c 'import sys; print(sys.prefix != getattr(sys, "base_prefix", sys.prefix))')

if [ "${IN_VENV}" = "True" ]; then
    # In a virtual environment, don't use --user
    PIP_FLAGS="--upgrade"
else
    # Not in a virtual environment, use --user
    PIP_FLAGS="--upgrade --user"
fi

# Install Python packages
pip install ${PIP_FLAGS} -r ${script_dir}/../../requirements.txt

if [[ -n "${build_type}" ]]; then
    # Install Conan profile and packages
    ${script_dir}/setup-conan.sh
else
    # Install Conan profile and debug packages
    export build_type=Debug
    ${script_dir}/setup-conan.sh

    # Install Conan profile and release packages
    export build_type=Release
    ${script_dir}/setup-conan.sh

    # Unset build_type
    unset build_type
fi

# Run CMake Configure
${script_dir}/run-cmake-configure.sh
