#!/bin/bash
script_source="${BASH_SOURCE[0]:-$0}"
script_dir="$(cd "$(dirname "${script_source}")" && pwd)"

# Import common paths
source "${script_dir}/common-paths.sh"

# Load custom build settings
if [ -f "${script_dir}/user-setup.sh" ]; then
    source "${script_dir}/user-setup.sh"
fi

# Activate python3 Virtual Environment
if [ -n "${venv_path:-}" ]; then
    python3 -m venv "${venv_path}"
    source "${venv_path}/bin/activate"
fi

# Detect if a python3 Virtual Environment was specified above, or elsewhere
IN_VENV=$(python3 -c 'import sys; print(sys.prefix != getattr(sys, "base_prefix", sys.prefix))')

if [ "${IN_VENV}" = "True" ]; then
    # In a virtual environment, don't use --user
    PIP_FLAGS="--upgrade"
else
    # Not in a virtual environment, use --user
    PIP_FLAGS="--upgrade --user"
fi

# Install python3 packages
python3 -m pip install ${PIP_FLAGS} pip
python3 -m pip install ${PIP_FLAGS} -r "${script_dir}/../../requirements.txt"

if [[ -n "${build_type}" ]]; then
    # Install Conan profile and packages
    "${script_dir}/setup-conan.sh"
else
    # Install Conan profile and debug packages
    export build_type=Debug
    "${script_dir}/setup-conan.sh"

    # Install Conan profile and release packages
    export build_type=Release
    "${script_dir}/setup-conan.sh"

    # Unset build_type
    unset build_type
fi

# Run CMake Configure
"${script_dir}/run-cmake-configure.sh"

# Deactivate python3 Virtual Environment
if [ -n "${venv_path:-}" ]; then
    deactivate
fi
