#!/bin/bash
script_source="${BASH_SOURCE[0]:-$0}"
script_dir="$(cd "$(dirname "${script_source}")" && pwd)"

# Configure default Conan profile
conan profile detect -e

# Install selected Conan profile
conan config install "${script_dir}/../conan/profiles/${conan_profile}" -tf profiles

# Install Conan packages
conan install "${script_dir}/../.." \
    --remote conancenter \
    --build missing \
    --profile:all ${conan_profile} \
    --settings:all build_type=${build_type} \
    --output-folder "${build_dir}/conan"
