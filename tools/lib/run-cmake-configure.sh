#!/bin/bash
script_dir="$(dirname "$(readlink -f "$0")")"

cmake_args=(
    -B "${build_dir}"
    -S "${script_dir}/../.."
    -G "${generator}"
    -DCMAKE_PREFIX_PATH="${qt_base}/${qt_version}/${qt_arch}"
    -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="${script_dir}/../../external/cmake-conan/conan_provider.cmake"
    -DCONAN_HOST_PROFILE="${conan_profile}"
    -DCONAN_BUILD_PROFILE="${conan_profile}"
)

if [[ -n "${build_type}" ]]; then
    cmake_args+=(
        -DCMAKE_BUILD_TYPE="${build_type}"
        -DCMAKE_CONFIGURATION_TYPES="${build_type}"
        -DCMAKE_INSTALL_PREFIX="${build_dir}/${build_type}/supercell-wx"
    )
else
    # CMAKE_BUILD_TYPE isn't used to build, but is required by the Conan CMakeDeps generator
    cmake_args+=(
        -DCMAKE_BUILD_TYPE="Release"
        -DCMAKE_CONFIGURATION_TYPES="Debug;Release"
    )
fi

mkdir -p "${build_dir}"
cmake "${cmake_args[@]}"
